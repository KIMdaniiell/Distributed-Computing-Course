#include <stdio.h>
//#include <unistd.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>


//#include <sys/types.h>
//#include <sys/wait.h>

#include "pa1_starter_code/pa1.h"


void parent_run (int N, pid_t children[]);
void child_run (int id);


int main() {
    printf("Starting ... PID=[%d]\n", getpid());
    int N = 10;             ///< число детей                                            BOTH_USE
    pid_t children[N];      ///< список pid детей, для wait в родительском процессе     PARENT_USE
//    int pipe_fd [N2];       ///< массив с парами дескрипторов каналов процессов         BOTH_USE

    pid_t pid = 0;

    for (int child_count = 0; child_count < N; child_count ++) {
        printf("Creating child №%d\n", child_count);
        pid = fork();
        if (pid == 0) {
            /* Потомок */
            child_run(child_count);
        }
        children[child_count] = pid;
    }

    parent_run(N, children);
    return 0;
}

void parent_run (int N, pid_t children[]) {

    printf("Children list:\n");
    for (int child_count = 0; child_count < N; child_count ++) {
        printf("    %d) [%d]\n",child_count ,children[child_count]);
    }

    for (int child_count = 0; child_count < N; child_count ++) {
        waitpid(children[child_count]);
        printf("Wait [%d] complete\n", children[child_count]);
    }

    printf("Shutting down... PID=[%d]\n", getpid());
}

void child_run (int id) {
    char log_file_name[128];
    sprintf (log_file_name, "build/logs/ps-%d-log.txt", id);
    int fd = open(log_file_name, O_CREAT | O_RDWR , 0777);

    char start_message[64];
    char dome_message[64];
    int s_msg_l = sprintf(start_message, log_started_fmt, id, getpid(), getppid());
    int d_msg_l = sprintf(dome_message, log_done_fmt, id);

    write(fd, start_message, s_msg_l);
    write(fd, dome_message, d_msg_l);


    printf("----------------- [%s] %d  ---  %s",log_file_name, fd, start_message);

    close(fd);
//    printf("Hi, Im %d\n", getpid());

    sleep(4);
//            printf("Shutting down... PID=[%d]\n", getpid());
    exit(0);
}

