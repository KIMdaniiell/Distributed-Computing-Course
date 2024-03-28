#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <unistd.h> // todo убрать (sleep)
#include <stdbool.h>

#include "ipc.h"
#include "banking.h"

#include "logger.h"
#include "communicator.h"
#include "message_builder.h"
#include "bank_worker.h"


#define N_MIN_VALUE 2                   ///< Минимальное число процессов
#define N_MAX_VALUE 10                  ///< Максимальное число процессов
#define X_MIN_VALUE N_MIN_VALUE - 1     ///< Минимальное число процессов
#define X_MAX_VALUE N_MAX_VALUE - 1     ///< Максимальное число процессов
#define S_MIN_VALUE 1                   ///< Минимальное значение начального баланса
#define S_MAX_VALUE 99                  ///< Максимальное значение начального баланса

/*int get_opt_p(int argc, char **argv);*/

int get_optc(int argc, char **argv, balance_t **optv_ptr);

balance_t *get_optv(int optind, int optc, char **argv);

void parent_run(pid_t children_PIDs[]);

void child_run(local_id id, int bank_account);


struct communicator *communicator;
struct logger *logger;


int N;      ///< Общее число процессов
int X;      ///< Число дочерних процессов



int main(int argc, char **argv) {

    /**====---- Command-line argument parsing ----====**/
    balance_t *initial_bank_accounts = NULL;
    X = get_optc(argc, argv, &initial_bank_accounts);
    N = X + 1;
    printf("[main] Параметр N (общее число процессов) равен %d\n", N);
    printf("[main] Параметр X (число дочерних процессов) равен %d\n", X);
    printf("\n");

    /**====---- Init communicator ----====**/
    communicator = init_communicator(X + 1);
    communicator->header.owner_id = PARENT_ID;
    communicator->header.N = N;

    /**====---- Init logger ----====**/
    logger = init_logger();

    /**====---- BREEDING ----====**/
    printf("[PARENT] Starting ... PID=[%d]\n", getpid());
    pid_t children_PIDs[X];
    pid_t pid;
    for (local_id child_count = 0; child_count < X; child_count++) {
        local_id child_local_id = child_count + 1;

        pid = fork();
        if (pid == 0) {
            /* Потомок */
            communicator->header.owner_id = child_local_id;
            optimise_communicator(communicator);

            child_run(child_local_id, initial_bank_accounts[child_count]);
            close_communicator(communicator);
//            close_logger(logger);
            exit(0);
        }
        printf("[PARENT] Creating child №%d (pid = %d)\n", child_local_id, pid);
        children_PIDs[child_count] = pid;
    }

    optimise_communicator(communicator);
    parent_run(children_PIDs);

    /**====---- Close resources ----====**/
    close_communicator(communicator);
    close_logger(logger);

    free(initial_bank_accounts);

    printf("[PARENT] Shutting down... PID=[%d]\n", getpid());
    return 0;
}

void parent_run(pid_t children_PIDs[]) {
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);
    AllHistory allHistory = {
            .s_history_len = 0
    };

    timestamp_t timestamp;

    int c;
    int count = 0;

    /**====---- START messages RECEIVE ----====**/
    while (-1 != (c = receive_any(communicator, message))) {
        printf("[PARENT-got--START-message %d/%d ]\n", ++count, X);

        if (count == X)
            break;
    }

    /**====---- TRANSFERRING BALANCES  ----====**/
    // TODO bank_robbery() invocation
    //      = multiple transfer() invocations
    printf("[PARENT] Robbing the bank!\n");
    bank_robbery(message, X);
    printf("[PARENT] Bank was robbed!\n");


    /**====---- STOP messages SEND ----====**/
    // TODO send STOP messages
    printf("[PARENT] Sending STOP multicast!\n");
    timestamp = get_physical_time();
    build_STOP_msg(message, timestamp);
    send_multicast(communicator, message);
    printf("[PARENT] Sent all STOP multicast!\n");


//    /**====---- DONE messages RECEIVE ----====**/
//    count = 0;
//    while (-1 != (c = receive_any(communicator, message))) {
//        printf("[PARENT-got-DONE-message %d/%d ]\n", ++count, X);
//
//        if (count == X)
//            break;
//    }
//
//    /**====---- BALANCE_HISTORY message RECEIVE ----====**/
//    // TODO receive BALANCE_HISTORY messages
//    // TODO aggregate BALANCE_HISTORY messages
//    count = 0;
//    while (-1 != (c = receive_any(communicator, message))) {
//        BalanceHistory *balanceHistory = (BalanceHistory *) message->s_payload;
//
//        printf("[PARENT-got--BALANCE_HISTORY-message %d/%d (%d)]\n", ++count, X, c);
//        print_BalanceHistory(balanceHistory);
//
//        append_AllHistory(&allHistory, balanceHistory);
//
//        if (count == X)
//            break;
//    }
    local_id done_children_count = 0;
    local_id balance_history_count = 0;
    /**====---- DONE messages RECEIVE ----====**/
    /**====---- BALANCE_HISTORY message RECEIVE ----====**/
    while (balance_history_count != X && -1 != (c = receive_any(communicator, message))) {
        switch (message->s_header.s_type) {
            case DONE:
                // todo
                done_children_count++;
                printf("[PARENT-got-DONE-message %d/%d ]\n", done_children_count, X);
                break;
            case BALANCE_HISTORY:
                // todo
                balance_history_count++;
                BalanceHistory *balanceHistory = (BalanceHistory *) message->s_payload;
                printf("[PARENT-got--BALANCE_HISTORY-message %d/%d (%d)]\n", balance_history_count, X, c);
                print_BalanceHistory(balanceHistory);
                append_AllHistory(&allHistory, balanceHistory);
                break;
            default:
                // todo
                break;
        }
    }

    complete_AllHistory(&allHistory);


    // TODO print history
    print_history(&allHistory);

    /**====---- Children WAITING ----====**/
    for (int child_count = 0; child_count < X; child_count++) {
        int child_status = 0;

        waitpid(children_PIDs[child_count], &child_status, 0);
        printf("[PARENT-wait-complete pid=%d %d/%d]\n", children_PIDs[child_count], child_count + 1, X);
    }
}

void child_run(local_id id, int bank_account) {
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);
    Message *message_out = (Message *) calloc(1, MAX_MESSAGE_LEN);
    BalanceHistory balanceHistory = {
            .s_id = id
    };

    timestamp_t timestamp;

    /**====---- Initial balance SAVE ----====**/
    timestamp = get_physical_time();
    append_BalanceHistory(&balanceHistory, bank_account, timestamp);


    /**====---- START messages SEND ----====**/
    timestamp = get_physical_time();
    do_log_started_fmt(logger, timestamp, id, bank_account);
    build_log_started_msg(message, timestamp, id, bank_account);
    send_multicast(communicator, message);

    /**====---- START messages RECEIVE ----====**/
    for (local_id child_count = 0; child_count < X; child_count++) {
        local_id child_local_id = child_count + 1;
        if (child_local_id == id)
            continue;
        while (0 != receive(communicator, child_local_id, message)) {
            sleep(1);
        }
    }
    timestamp = get_physical_time();
    do_log_received_all_started_fmt(logger, timestamp, id);


    /**====---- TRANSFER & STOP messages processing ----====**/
    TransferOrder *transferOrder;
    bool is_stopped = false;
    local_id stopped_children_count = 0;
    //TODO process TRANSFER & STOP messages
    for (;;) {
        printf("[CHILD #%d] DONE-children-counter %d/%d]\n", id, stopped_children_count, X);

        while (stopped_children_count != X && -1 != receive_any(communicator, message)) {
            timestamp = get_physical_time();

            switch (message->s_header.s_type) {
                case TRANSFER:
                    transferOrder = (TransferOrder *) message->s_payload;
                    printf("[CHILD #%d] got-TRANSFER-message %d -> %d ", id, transferOrder->s_src,
                           transferOrder->s_dst);

                    if (transferOrder->s_dst == id) {
                        printf("IS_DESTINATION\n");
                        bank_account += transferOrder->s_amount;

                        build_ACK_msg(message_out, timestamp);
                        /*int res =*/ send(communicator, PARENT_ID, message_out);
                        /*printf("[CHILD #%d] \t %d -> %d [%d]\n", id, id, PARENT_ID, res);*/
                        do_log_transfer_in_fmt(logger, timestamp,
                                               id,
                                               transferOrder->s_src,
                                               transferOrder->s_amount);
                        append_BalanceHistory(&balanceHistory, bank_account, timestamp);
                    } else {
                        if (is_stopped) {
                            printf("[CHILD #%d] Error! Got TRANSFER from Parent after STOP!", id);
                            exit(-30);
                        }
                        printf("IS_SENDER\n");
                        bank_account -= transferOrder->s_amount;

                        build_TRANSFER_msg(message_out, timestamp, transferOrder);
                        /*int res =*/ send(communicator, transferOrder->s_dst, message_out);
                        /*printf("[CHILD #%d] \t %d -> %d [%d]\n", id, id, transferOrder->s_dst, res);*/
                        do_log_transfer_out_fmt(logger, timestamp,
                                                id,
                                                transferOrder->s_dst,
                                                transferOrder->s_amount);
                        append_BalanceHistory(&balanceHistory, bank_account, timestamp);
                    }
                    break;
                case STOP:
                    // todo
                    printf("[CHILD #%d] got-STOP-message\n", id);
                    is_stopped = true;
                    stopped_children_count++;
//                    goto SEND_DONE_MULTICAST;

                    /**====---- DONE messages SEND ----====**/
                    timestamp = get_physical_time();
                    do_log_done_fmt(logger, timestamp, id, bank_account);
                    build_log_done_msg(message, timestamp, id, bank_account);
                    send_multicast(communicator, message);
                    break;
                case DONE:
                    /**====---- DONE messages RECEIVE ----====**/
                    printf("[CHILD #%d] got-DONE-message [stopped_children_count %d->%d]\n",
                           id,
                           stopped_children_count, stopped_children_count + 1);
                    stopped_children_count++;
                    break;
                default:
                    // todo
                    printf("[CHILD #%d] got-message\n", id);
                    break;
            }
        }
        if (stopped_children_count == X) {
            printf("[CHILD #%d] RECEIVED ALL DONE\n", id);
            do_log_received_all_done_fmt(logger, timestamp, id);
            goto SEND_BALANCE_HISTORY;
        }
    }

//    SEND_DONE_MULTICAST:
    /**====---- DONE messages SEND ----====**/
    timestamp = get_physical_time();
    do_log_done_fmt(logger, timestamp, id, bank_account);
    build_log_done_msg(message, timestamp, id, bank_account);
    send_multicast(communicator, message);

//    RECEIVE_ALL_DONE:
    /**====---- DONE messages RECEIVE ----====**/
    timestamp = get_physical_time();
    for (local_id child_count = 0; child_count < X; child_count++) {
        local_id child_local_id = child_count + 1;
        if (child_local_id == id)
            continue;
        while (0 != receive(communicator, child_local_id, message)) {
            sleep(1);
        }
    }
    do_log_received_all_done_fmt(logger, timestamp, id);

    SEND_BALANCE_HISTORY:
    /**====---- BALANCE_HISTORY message SEND ----====**/
    // TODO send BALANCE_HISTORY message
    timestamp = get_physical_time();
    build_BALANCE_HISTORY_msg(message_out, timestamp, &balanceHistory);
    send(communicator, PARENT_ID, message_out);
    printf("[CHILD #%d] sent BALANCE_HISTORY\n", id);

    printf("[CHILD] Shutting down... PID=[%d]\n", getpid());
}

/*int get_opt_p(int argc, char **argv) {
    char *opts = "p:";                  ///< список доступных опций
    int opt;                            ///< каждая следующая опция попадает сюда
    int X = -1;

    while (-1 != (opt = getopt(argc, argv, opts))) {
        switch (opt) {
            case 'p':                   ///< если опция -p, преобразуем строку с аргументом в число
                X = atoi(optarg);
                break;
            default:
                break;
        }
    }

    if (X < 1) {
        printf("[main:get_opt_p] Параметр X некорректен\n");
        printf("[main:get_opt_p] Необходимо указать аргумент -p \n");
        // abort();
        exit(-1);
    }

    printf("[main:get_opt_p] Параметр X равен %d\n", X);

    return X;
}*/

/**====---- Getting bank accounts count ----====**/
int get_optc(int argc, char **argv, balance_t **optv_ptr) {
    char *opts = "p:";                  ///< список доступных опций
    int opt;                            ///< каждая следующая опция попадает сюда
    int optc = -1;

    while (-1 != (opt = getopt(argc, argv, opts))) {
        switch (opt) {
            case 'p':                   ///< если опция -p, преобразуем строку с аргументом в число
                optc = atoi(optarg);
                break;
            default:
                break;
        }
    }

    if (optc < 1) {
        printf("[main:get_opt_C] Необходимо указать аргумент -p \n");
        exit(-1);
    }

    if (optc > argc - 3) {
        printf("[main:get_opt_C] Параметр optc = %d некорректный! (optc > argc - 3)\n",
               optc);
        exit(-1);
    }

    if (optc < X_MIN_VALUE) {
        printf("[main:get_opt_C] Параметр optc = %d невалидный! (< X_MIN_VALUE = %d)\n",
               optc, X_MIN_VALUE);
        exit(-1);
    }

    if (optc > X_MAX_VALUE) {
        printf("[main:get_opt_C] Параметр optc = %d невалидный! (> X_MAX_VALUE = %d)\n",
               optc, X_MAX_VALUE);
        exit(-1);
    }

    printf("[main:get_opt_C] Параметр optc равен %d\n", optc);

    *optv_ptr = get_optv(optind, optc, argv);

    return optc;
}

/**====---- Getting bank accounts values ----====**/
balance_t *get_optv(int optind, int optc, char **argv) {
    balance_t *optv = calloc(optc, sizeof(balance_t));

    for (int i = 0; i < optc; i++) {
        int raw_opt_value = atoi(argv[optind + i]);

        if (raw_opt_value < S_MIN_VALUE) {
            printf("[main:get_opt_V] Параметр S[%d] = %d невалидный! (< S_MIN_VALUE = %d)\n",
                   i, raw_opt_value, S_MIN_VALUE);
            exit(-1);
        }

        if (raw_opt_value > S_MAX_VALUE) {
            printf("[main:get_opt_V] Параметр S[%d] = %d невалидный! (> S_MAX_VALUE = %d)\n",
                   i, raw_opt_value, S_MAX_VALUE);
            exit(-1);
        }

        optv[i] = (balance_t) raw_opt_value;

        printf("[main:get_opt_V] \t%d) %d\n", i, optv[i]);
    }

    return optv;
}

// TODO implement in BANKER module
void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    printf("[PARENT] Transferring %d -> %d\n", src, dst);
    Message *message;
    timestamp_t timestamp;

    /**====---- Init variables ----====**/
    message = parent_data;
    TransferOrder transferOrder = {
            .s_src = src,
            .s_dst = dst,
            .s_amount = amount
    };
    timestamp = get_physical_time(); // TODO проверить, где это нужно инициализировать

    /**====---- Build Message ----====**/
    build_TRANSFER_msg(message, timestamp, &transferOrder);

    int res;
    /**====---- TRANSFER Message SEND ----====**/
    res = send(communicator, src, message);
//    printf("[PARENT] \t parent -> %d [%d]\n", src, res);

    /**====---- ACK Message RECEIVE ----====**/
    for (;;) {
        res = receive(communicator, dst, message);

        if (res == 0) {
            /*printf("[id = %d] [from = %d] [code = %d] [c_N = %d]\n",
                   PARENT_ID , dst,
                   res,
                   communicator->header.N);*/
            TransferOrder *receive_transferOrder = (TransferOrder *) message->s_payload;
            printf("[PARENT] got-TRANSFER-message %d -> %d ACKNOWLEDGE\n",
                   receive_transferOrder->s_src,
                   receive_transferOrder->s_dst);
            break;
        } else {
            sleep(1);
        }
    }
//    printf("[PARENT] \t %d -> parent [%d]\n", dst, res);
}
