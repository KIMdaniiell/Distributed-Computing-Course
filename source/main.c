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


int get_optc(int argc, char **argv, balance_t **optv_ptr);

balance_t *get_optv(int optind, int optc, char **argv);


int N;      ///< Общее число процессов
int X;      ///< Число дочерних процессов


int main(int argc, char **argv) {
    printf("=========================THE BEGINNING=========================\n");

    /**====---- Command-line argument parsing ----====**/
    balance_t *initial_bank_accounts = NULL;
    X = get_optc(argc, argv, &initial_bank_accounts);
    N = X + 1;
    printf("[main] Параметр N (общее число процессов) равен %d\n", N);
    printf("[main] Параметр X (число дочерних процессов) равен %d\n", X);

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
            child_run_full(initial_bank_accounts[child_count]);
        }
        printf("[main] Creating child №%d (pid = %d)\n", child_local_id, pid);
        children_PIDs[child_count] = pid;
    }

    /* Родитель */
    set_local_id(PARENT_ID);
    parent_run_full(children_PIDs);

    free(initial_bank_accounts);

    printf("============================THE END============================\n");
}


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
