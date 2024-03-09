#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>


#include "ipc.h"

#include "logger.h"
#include "communicator.h"
#include "messenger.h"


int get_opt_p(int argc, char **argv);

void parent_run(pid_t children[]);

void child_run(local_id id);


struct communicator *communicator;
struct logger *logger;

int N;      ///< Число детей        BOTH_USE
int X;      ///< Число процессов


int main(int argc, char **argv) {
    printf("[PARENT] Starting ... PID=[%d]\n", getpid());

    /**====---- Init variables ----====**/
    X = get_opt_p(argc, argv);
    int N = X + 1;
    pid_t children[X];                              ///< список pid детей, для wait в родительском процессе   PARENT_USE
    pid_t pid;
    local_id id = 0;

    /**====---- Init communicator ----====**/
    communicator = init_communicator(X + 1);
    communicator->header.owner_id = id;
    communicator->header.N = N;

    /**====---- Init logger ----====**/
    logger = init_logger();


    /**====---- BREEDING ----====**/
    for (local_id child_count = 0; child_count < X; child_count++) {
        local_id child_local_id = child_count + 1;

        pid = fork();
        if (pid == 0) {
            /* Потомок */
            communicator->header.owner_id = child_local_id;
            optimise_communicator(communicator);

            child_run(child_local_id);
            close_communicator(communicator);
            exit(0);
        }
        printf("[PARENT] Creating child №%d (pid = %d)\n", child_local_id, pid);
        children[child_count] = pid;
    }

    optimise_communicator(communicator);
    parent_run(children);

    /**====---- Close resources ----====**/
    close_communicator(communicator);
    close_logger(logger);

    return 0;
}

void parent_run(pid_t children[]) {
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
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);

    /**====---- START messages SEND ----====**/
    do_log_started_fmt(logger, id);
    init_log_started_fmt_message(message, id);
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
    do_log_received_all_started_fmt(logger, id);

    /**====---- DONE messages SEND ----====**/
    do_log_done_fmt(logger, id);
    init_log_done_fmt(message, id);
    send_multicast(communicator, message);

    /**====---- DONE messages RECEIVE ----====**/
    for (local_id child_count = 0; child_count < X; child_count++) {
        local_id child_local_id = child_count + 1;
        if (child_local_id == id)
            continue;
        while (0 != receive(communicator, child_local_id, message)) {
            sleep(1);
        }
    }
    do_log_received_all_done_fmt(logger, id);

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


