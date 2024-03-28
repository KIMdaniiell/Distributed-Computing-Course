#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <getopt.h>

#define N_MIN_VALUE 2                   ///< Минимальное число процессов
#define N_MAX_VALUE 10                  ///< Максимальное число процессов
#define X_MIN_VALUE N_MIN_VALUE - 1     ///< Минимальное число процессов
#define X_MAX_VALUE N_MAX_VALUE - 1     ///< Максимальное число процессов
#define S_MIN_VALUE 1                   ///< Минимальное значение начального баланса
#define S_MAX_VALUE 99                  ///< Максимальное значение начального баланса


typedef int16_t balance_t;

int get_optc(int argc, char **argv, balance_t **optv_ptr);

balance_t *get_optv(int optind, int optc, char **argv);


int N;      ///< Число процессов
int X;      ///< Число дочерних процессов


int main(int argc, char **argv) {

    /**====---- Command-line argument parsing ----====**/
    balance_t *bank_accounts = NULL;
    X = get_optc(argc, argv, &bank_accounts);
    N = X + 1;

    printf("[main] Параметр N равен %d\n", N);
    printf("[main] Параметр X равен %d\n", X);

    for (int i = 0; i < X; i++) {
        printf("[main] \t%d) %d\n", i, bank_accounts[i]);
    }

    return 0;
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

    if (optc < N_MIN_VALUE) {
        printf("[main:get_opt_C] Параметр optc = %d невалидный! (< N_MIN_VALUE = %d)\n",
               optc, N_MIN_VALUE);
        exit(-1);
    }

    if (optc > N_MAX_VALUE) {
        printf("[main:get_opt_C] Параметр optc = %d невалидный! (> N_MAX_VALUE = %d)\n",
               optc, N_MAX_VALUE);
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
