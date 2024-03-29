#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <unistd.h>
#include <stdbool.h>

#include "ipc.h"
#include "banking.h"


#include "parent_process.h"
#include "child_process.h"


#define N_MIN_VALUE 2                   ///< Минимальное число процессов
#define N_MAX_VALUE 10                  ///< Максимальное число процессов
#define X_MIN_VALUE N_MIN_VALUE - 1     ///< Минимальное число процессов
#define X_MAX_VALUE N_MAX_VALUE - 1     ///< Максимальное число процессов
#define S_MIN_VALUE 1                   ///< Минимальное значение начального баланса
#define S_MAX_VALUE 99                  ///< Максимальное значение начального баланса


int handle_opt(int argc, char **argv, bool *mutex_l_is_on);


int N;      ///< Общее число процессов
int X;      ///< Число дочерних процессов


int main(int argc, char **argv) {
    printf("=========================THE BEGINNING=========================\n");

    /**====---- Command-line argument parsing ----====**/
    bool mutex_l_is_on;
    X =  handle_opt(argc, argv, &mutex_l_is_on);
    N = X + 1;
    printf("[main] Параметр N (общее число процессов) равен %d\n", N);
    printf("[main] Параметр X (число дочерних процессов) равен %d\n", X);
    printf("[main] Параметр mutexl %s\n", mutex_l_is_on ? "АКТИВЕН" : "НЕ_АКТИВЕН");

    /**====---- Init process utils ----====**/
    set_process_communicator();
    set_process_logger();

    /**====---- BREEDING ----====**/
    pid_t pid;
    pid_t children_PIDs[X];
    for (local_id child_count = 0; child_count < X; child_count++) {
        local_id child_local_id = child_count + 1;

        pid = fork();
        if (pid == 0) {
            /* Потомок */
            set_local_id(child_local_id);
            child_run_full(mutex_l_is_on);
        }
        printf("[main] Creating child №%d (pid = %d)\n", child_local_id, pid);
        children_PIDs[child_count] = pid;
    }

    /* Родитель */
    set_local_id(PARENT_ID);
    parent_run_full(children_PIDs);

    printf("============================THE END============================\n");
}

/**====---- Process command line options ----====**/
int handle_opt(int argc, char **argv, bool *mutex_l_is_on) {
    *mutex_l_is_on = false;
    int ps_count = -1;
    struct option long_options[] = {
            {"mutexl", 0, 0, 0},
    };

    int opt = -1;
    int opt_id = -1;

    while (-1 != (opt = getopt_long(argc, argv, "p:", long_options, &opt_id))) {
        switch (opt) {
            case 0:
//                printf ("параметр %s\n", long_options[opt_id].name);
                *mutex_l_is_on = true;
                break;
            case 'p':
//                printf("«option 'p' selected, value %d»\n", atoi(optarg));
                ps_count = atoi(optarg);
                break;
            default:
                printf("«something selected\n»");
                break;
        }
    }
    return ps_count;
}
