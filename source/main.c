#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>

#include "pa1.h"
#include "common.h"
#include "ipc.h"
#include "communicator.h"


int events_log_fd;
int X;
struct communicator *communicator;

int get_opt_p(int argc, char **argv);

void parent_run(int N, pid_t children[]);

void child_run(local_id id);


int main(int argc, char **argv) {
    printf("[PARENT] Starting ... PID=[%d]\n", getpid());

    X = get_opt_p(argc, argv);    ///< число детей                                            BOTH_USE
    int N = X + 1;
    pid_t children[X];              ///< список pid детей, для wait в родительском процессе     PARENT_USE

    events_log_fd = open(events_log, O_CREAT | O_RDWR | O_TRUNC, 0777);

    communicator = init_communicator(X + 1);
    communicator->header.N = N;

    pid_t pid = 0;
    for (local_id child_count = 0; child_count < X; child_count++) {
        local_id child_id = child_count + 1;

        pid = fork();
        if (pid == 0) {
            /* Потомок */
            communicator->header.owner_id = child_id;

            child_run(child_id);

            close_communicator(communicator);

            exit(0);
        }
        printf("[PARENT] Creating child №%d (pid = %d)\n", child_id, pid);
        children[child_count] = pid;
    }


    communicator->header.owner_id = 0;

    parent_run(X, children);

    close_communicator(communicator);

    close(events_log_fd);

    return 0;
}

void parent_run(int X, pid_t children[]) {
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);

    int c;
    int count = 0;
    while (-1 != (c = receive_any(communicator, message))) {
        printf("[PARENT-got--START-message %d/%d ]\n", ++count, X);

        if (count == X)
            break;
    }

    count = 0;
    while (-1 != (c = receive_any(communicator, message))) {
        printf("[PARENT-got-DONE-message %d/%d ]\n", ++count, X);

        if (count == X)
            break;
    }

    for (int child_count = 0; child_count < X; child_count++) {
        int child_status = 0;

        waitpid(children[child_count], &child_status, 0);
        printf("Wait [%d] complete\n", children[child_count]);
    }

    printf("Shutting down... PID=[%d]\n", getpid());
}

void child_run(local_id id) {
    char event_message_buffer[64];
    //    printf("(pid=%d) comm head trans is %d\n", getpid(), communicator->header.owner_id);

    /**====---- WRITE log messages ----====**/
    Message *start_message = (Message *) calloc(1, MAX_MESSAGE_LEN);
    start_message->s_header.s_magic = MESSAGE_MAGIC;
    start_message->s_header.s_payload_len = MAX_PAYLOAD_LEN;
    start_message->s_header.s_type = STARTED;
    start_message->s_header.s_local_time = 0;

    int msg_l = sprintf(event_message_buffer, log_started_fmt, id, getpid(), getppid());
    write(events_log_fd, event_message_buffer, msg_l);
    sprintf(start_message->s_payload, log_started_fmt, id, getpid(), getppid());
    send_multicast(communicator, start_message);


    for (local_id child_count = 0; child_count < X; child_count++) {
        Message *got_message = (Message *) calloc(1, MAX_MESSAGE_LEN);
        local_id child_id = child_count + 1;

        if (child_id == id)
            continue;

        int c = -1;
        while (c != 0) {
            c = receive(communicator, child_id, got_message);
        }

    }
    msg_l = sprintf(event_message_buffer, log_received_all_started_fmt, id);
    write(events_log_fd, event_message_buffer, msg_l);


    Message *done_message = (Message *) calloc(1, MAX_MESSAGE_LEN);
    done_message->s_header.s_magic = MESSAGE_MAGIC;
    done_message->s_header.s_payload_len = MAX_PAYLOAD_LEN;
    done_message->s_header.s_type = DONE;
    done_message->s_header.s_local_time = 0;

    msg_l = sprintf(event_message_buffer, log_done_fmt, id);
    write(events_log_fd, event_message_buffer, msg_l);
    sprintf(done_message->s_payload, log_done_fmt, id);
    send_multicast(communicator, done_message);


    for (local_id child_count = 0; child_count < X; child_count++) {
        Message *got_message = (Message *) calloc(1, MAX_MESSAGE_LEN);
        local_id child_id = child_count + 1;

        if (child_id == id)
            continue;

        int c = -1;
        while (c != 0) {
            c = receive(communicator, child_id, got_message);
        }
    }
    msg_l = sprintf(event_message_buffer, log_received_all_done_fmt, id);
    write(events_log_fd, event_message_buffer, msg_l);


    printf("[CHILD] Shutting down... PID=[%d]\n", getpid());
}

int get_opt_p(int argc, char **argv) {
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
}


