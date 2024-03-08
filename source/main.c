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

#include "pa1_starter_code/pa1.h"
#include "pa1_starter_code/common.h"
#include "communicator.h"


static const char *const PIPES_LOG_FILE_NAME = pipes_log;
static const char *const EVENTS_LOG_FILE_NAME = events_log;


int get_opt_p(int argc, char **argv);

void parent_run(int N, pid_t children[]);

void child_run(int id);


int main(int argc, char **argv) {
    printf("[PARENT] Starting ... PID=[%d]\n", getpid());
    int X = get_opt_p(argc, argv);    ///< число детей                                            BOTH_USE
    int N = X + 1;
    pid_t children[X];              ///< список pid детей, для wait в родительском процессе     PARENT_USE

    struct communicator * communicator = init_communicator(X + 1);

    pid_t pid = 0;
    for (int child_count = 0; child_count < X; child_count++) {
        pid = fork();
        if (pid == 0) {
            /* Потомок */
            child_run(child_count);
            exit(0);
        }
        printf("[PARENT] Creating child №%d (pid = %d)\n", child_count, pid);
        children[child_count] = pid;
    }
    parent_run(X, children);
    return 0;
}

void parent_run(int N, pid_t children[]) {

    printf("[PARENT] Children list [%d]:\n", N);
    for (int child_count = 0; child_count < N; child_count++) {
        printf("    %d) [%d]\n", child_count, children[child_count]);
    }

    for (int child_count = 0; child_count < N; child_count++) {
        int child_status = 0;

        waitpid(children[child_count], &child_status, 0);
        printf("Wait [%d] complete\n", children[child_count]);
    }

    printf("Shutting down... PID=[%d]\n", getpid());
}

void child_run(int id) {
    char log_file_name[128];
    sprintf(log_file_name, "build/ps-%d-log.txt", id);
    int fd = open(log_file_name, O_CREAT | O_RDWR, 0777);
    sleep(2);

    char start_message[64];
    char dome_message[64];
    int s_msg_l = sprintf(start_message, log_started_fmt, id, getpid(), getppid());
    int d_msg_l = sprintf(dome_message, log_done_fmt, id);

    write(fd, start_message, s_msg_l);
    write(fd, dome_message, d_msg_l);


    printf("----------------- [%s] %d  ---  %s", log_file_name, fd, start_message);

    close(fd);
//    printf("Hi, Im %d\n", getpid());

    sleep(2);
//            printf("Shutting down... PID=[%d]\n", getpid());
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
