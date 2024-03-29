#include "process.h"


void build_message(int16_t type, Message *message);

int is_less(timestamp_t Li, local_id i, timestamp_t Lj, local_id j);

int is_least(timestamp_t Li, local_id i, timestamp_t *queue);

void print_queue(timestamp_t *queue);

void print_queue_msg(timestamp_t *queue, char *msg, local_id scr);

int get_size(timestamp_t *queue);


struct communicator *communicator;
struct logger *logger;

local_id process_id;

timestamp_t timestamp;


timestamp_t *queue;


void set_local_id(local_id local_id) {
    process_id = local_id;
}

void set_process_communicator() {
    printf("\n");
    communicator = init_communicator(N);
    printf("\n");

    communicator->header.N = N;
    communicator->header.owner_id = -1;
}

void set_process_logger() {
    logger = init_logger();
}

void pre_run() {
    communicator->header.owner_id = process_id;
    optimise_communicator(communicator);
    queue = (timestamp_t *) calloc(X + 1, sizeof(timestamp_t));
}

void post_run() {
    close_communicator(communicator);
    close_logger(logger);
    free(queue);
}

timestamp_t get_lamport_time() {
    return timestamp;
}


void send_wrapper(int16_t type, Message *message, local_id dst) {
    // Build message
    build_message(type, message);
    // Send message
    decorated_send(communicator, dst, message, &timestamp);
}

void send_multicast_wrapper(int16_t type, Message *message) {
    // Build message
    build_message(type, message);

    // Send message
    if (process_id == 0) {
        printf("%3d: [PARENT] SENDM %s-message\n", timestamp, get_msg_type_name(logger, type));
    } else {
        printf("%3d: [CHILD #%d] SENDM %s-message\n", timestamp, process_id, get_msg_type_name(logger, type));
    }
    decorated_send_multicast(communicator, message, &timestamp);
}


void receive_multicast_wrapper(int16_t type, Message *message) {
    for (local_id child_local_id = 1; child_local_id <= X; child_local_id++) {
        if (child_local_id == process_id)
            continue;

        if (message->s_header.s_type == DONE && queue[child_local_id - 1] < 0) {
            printf("%3d: [CHILD #%d] RECEV DONE-message [%d/%d] while being in CS\n",
                   timestamp,
                   process_id,
                   child_local_id < process_id ? child_local_id : child_local_id - 1,
                   X - 1);
            continue;
        }

        decorated_receive(communicator, child_local_id, message, &timestamp);
        while (message->s_header.s_type != type) {
            if (process_id == 0) {
                printf("%3d: [PARENT] SKPPD message from %d! Got-message-type (%s) <> expected (%s)\n",
                       timestamp, child_local_id, get_msg_type_name(logger, message->s_header.s_type),
                       get_msg_type_name(logger, type));
            } else {
                printf("%3d: [CHILD #%d] SKPPD message from %d! Got-message-type (%s) <> ",
                       timestamp, process_id, child_local_id, get_msg_type_name(logger, message->s_header.s_type));
                printf("expected (%s)\n", get_msg_type_name(logger, type));
            }
            decorated_receive(communicator, child_local_id, message, &timestamp);
        }

        if (process_id == 0) {
            printf("%3d: [PARENT] RECEV %s-message [%d/%d]\n",
                   timestamp, get_msg_type_name(logger, type), child_local_id, X);
        } else {
            printf("%3d: [CHILD #%d] RECEV %s-message [%d/%d]\n", timestamp, process_id,
                   get_msg_type_name(logger, type),
                   child_local_id < process_id ? child_local_id : child_local_id - 1, X - 1);
        }
    }
}


int request_cs(const void *self) {
    Message *message = (Message *) self;

    /* 1. Положить свой запрос в свою очередь */
    queue[process_id - 1] = timestamp + 1;
    /* 2. Разослать свой запрос */
    send_multicast_wrapper(CS_REQUEST, message);

    int replies_count = 0;
    for (;;) {
        if (replies_count >= get_size(queue) - 1 && is_least(queue[process_id - 1], process_id, queue) == 1) {
            print_queue_msg(queue, "BECOME LEAST!", -1);
            return 0;
        }

//        print_queue(queue);
//        print_queue_msg(queue, "REPLIES GOT", get_size(queue) - 1);
//        printf("%d: [CHILD #%d] ---------------------------------------------------------------------------------- replies_count[%d] out of size[%d] && %s\n", timestamp, process_id, replies_count, get_size(queue) - 1,
//               is_least(queue[process_id - 1], process_id, queue)==1 ? "LEAST" : "NOT LEAST");
        int src_process_id = decorated_receive_any(communicator, message, &timestamp);

        switch (message->s_header.s_type) {
            case CS_REQUEST:
                /* 4. Положить пришедший в свою очередь */
                queue[src_process_id - 1] = message->s_header.s_local_time;
                print_queue_msg(queue, "got-REQUEST-message", src_process_id);
                /* 5. Отправить в ответ REPLY (L-лог.время, j-id процесса) */
                send_wrapper(CS_REPLY, message, src_process_id);
                break;
            case CS_REPLY:
                /* 4. Дождаться получения ответа от всех */
                replies_count++;
                if (replies_count < X - 1) {
                    print_queue_msg(queue, "got-REPLY-message", src_process_id);
                    break;
                }
                /* 5. Проверить свою "наименьшесть": */
                if (1 == is_least(queue[process_id - 1], process_id, queue)) {
                    print_queue_msg(queue, "got-ALL-REPLY-messages, IS LEAST!", src_process_id);
                    /* Если нет меньше:
                      6. Возвращение к точке вызова и выполнение критической секции */
                    return 0;
                    /*7. Удаление себя из своей очереди
                      8. Разослать RELEASE*/
                }
                /*Если есть меньше:
                  ждем RELEASE*/
                print_queue_msg(queue, "got-ALL-REPLY-messages, but is not least", src_process_id);
                break;
            case CS_RELEASE:
                /* 4. Удаление пришедшего из своей очереди */
                queue[src_process_id - 1] = 0;
                print_queue_msg(queue, "got-RELEASE-message", src_process_id);
                break;
            default:
                if (message->s_header.s_type == DONE) {
                    queue[src_process_id - 1] = -1;
                    print_queue_msg(queue, "got-DONE-message", src_process_id);
                } else {
                    print_queue_msg(queue, "got-___-message PANIC!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",
                                    src_process_id);
                }
                break;
        }
    }
}

int release_cs(const void *self) {
    Message *message = (Message *) self;

    queue[process_id - 1] = 0;
    send_multicast_wrapper(CS_RELEASE, message);
    return 0;
}


void build_message(int16_t type, Message *message) {
    switch (type) {
        case STARTED:
            build_log_STARTED_msg(message, timestamp, process_id, 0);
            break;
        case DONE:
            build_log_DONE_msg(message, timestamp, process_id, 0);
            break;
        case TRANSFER:
            printf("[process:build_message] ERROR! NOT-YET-SUPPORTED MESSAGE TYPE (%d)!\n", type);
            exit(-51);
            break;
        case ACK:
            printf("[process:build_message] ERROR! NOT-YET-SUPPORTED MESSAGE TYPE (%d)!\n", type);
            exit(-51);
            break;
        case STOP:
            printf("[process:build_message] ERROR! NOT-YET-SUPPORTED MESSAGE TYPE (%d)!\n", type);
            exit(-51);
            break;
        case BALANCE_HISTORY:
            printf("[process:build_message] ERROR! NOT-YET-SUPPORTED MESSAGE TYPE (%d)!\n", type);
            exit(-51);
            break;
        case CS_REQUEST:
            build_CS_REQUEST_msg(message, timestamp);
            break;
        case CS_REPLY:
            build_CS_REPLY_msg(message, timestamp);
            break;
        case CS_RELEASE:
            build_CS_RELEASE_msg(message, timestamp);
            break;
        default:
            printf("[process:build_message] ERROR! NO SUCH MESSAGE TYPE (%d)!\n", type);
            exit(-50);
    }
}


int is_less(timestamp_t Li, local_id i, timestamp_t Lj, local_id j) {
    if (Lj < 1)
        return 1;

    if (Li < Lj || ((Li == Lj) && (i < j)))
        return 1;
    else
        return 0;
}

int is_least(timestamp_t Li, local_id i, timestamp_t *queue) {
    int is_least = 1;

    for (int j = 0; j < X; j++) {
        if (j == i - 1)
            continue;
        is_least *= is_less(Li, i, queue[j], j + 1);
    }

    return is_least;
}


void print_queue(timestamp_t *queue) {
    printf("%3d: [CHILD #%d] QUEUE [ ", timestamp, process_id);
    for (int i = 0; i < X; i++)
        if (queue[i] == 0) {
            printf("__ ");
        } else {
            printf("%2d ", queue[i]);
        }
    printf("]\n");
}


void print_queue_msg(timestamp_t *queue, char *msg, local_id src) {
    printf("%3d: [CHILD #%d] QUEUE [ ", timestamp, process_id);
    for (int i = 0; i < X; i++)
        if (queue[i] == 0) {
            printf("__ ");
        } else {
            printf("%2d ", queue[i]);
        }

    if (src == -1) {
        printf("] %s\n", msg);
    } else {
        printf("] %s FROM %d\n", msg, src);
    }
}


int get_size(timestamp_t *queue) {
    int counter = 0;
    for (int i = 0; i < X; i++)
        if (queue[i] >= 0) {
            counter++;
        }
    return counter;
}
