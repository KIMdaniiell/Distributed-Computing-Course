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
#include "message_builder.h"


#define N_MIN_VALUE 2                   ///< Минимальное число процессов
#define N_MAX_VALUE 10                  ///< Максимальное число процессов
#define X_MIN_VALUE N_MIN_VALUE - 1     ///< Минимальное число процессов
#define X_MAX_VALUE N_MAX_VALUE - 1     ///< Максимальное число процессов
#define S_MIN_VALUE 1                   ///< Минимальное значение начального баланса
#define S_MAX_VALUE 99                  ///< Максимальное значение начального баланса


int get_opt_p(int argc, char **argv);

void parent_run(pid_t children_PIDs[]);

void child_run(local_id id);


struct communicator *communicator;
struct logger *logger;


int N;      ///< Общее число процессов
int X;      ///< Число дочерних процессов


int main(int argc, char **argv) {

    /**====---- Command-line argument parsing ----====**/
    X = get_opt_p(argc, argv);
    int N = X + 1;
    printf("[main] Параметр N (общее число процессов) равен %d\n", N);
    printf("[main] Параметр X (число дочерних процессов) равен %d\n", X);

    /**====---- Init communicator ----====**/
    printf("\n");
    communicator = init_communicator(X + 1);
    printf("\n");
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

            child_run(child_local_id);

            /**====---- Close resources ----====**/
            close_communicator(communicator);
            close_logger(logger);

            printf("[CHILD #%d] Shutting down... PID=[%d]\n", child_local_id, getpid());
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

    printf("[PARENT] Shutting down... PID=[%d]\n", getpid());
    return 0;
}

void parent_run(pid_t children_PIDs[]) {
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);

    int c;
    int count = 0;

    /**====---- START-messages RECEIVE ----====**/
    while (-1 != (c = receive_any(communicator, message))) {
        printf("[PARENT-got--START-message %d/%d ]\n", ++count, X);

        if (count == X)
            break;
    }

    /**====---- DONE-messages RECEIVE ----====**/
    count = 0;
    while (-1 != (c = receive_any(communicator, message))) {
        printf("[PARENT-got-DONE-message %d/%d ]\n", ++count, X);

        if (count == X)
            break;
    }

    /**====---- Children WAITING ----====**/
    for (int child_count = 0; child_count < X; child_count++) {
        int child_status = 0;

        waitpid(children_PIDs[child_count], &child_status, 0);
        printf("[PARENT-wait-complete pid=%d %d/%d]\n", children_PIDs[child_count], child_count + 1, X);
    }
}

void child_run(local_id id) {
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);

    /**====---- START messages SEND ----====**/
    do_log_started_fmt(logger, id);
    build_log_STARTED_msg(message, id);
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
    build_log_DONE_msg(message, id);
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
        exit(-1);
    }

    printf("[main:get_opt_p] Параметр X равен %d\n", X);

    return X;
}


