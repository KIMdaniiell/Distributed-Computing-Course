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


#define get_physical_time get_lamport_time


int get_optc(int argc, char **argv, balance_t **optv_ptr);

balance_t *get_optv(int optind, int optc, char **argv);

void parent_run(pid_t children_PIDs[]);

void child_run(local_id id, int bank_account);


struct communicator *communicator;
struct logger *logger;


int N;      ///< Общее число процессов
int X;      ///< Число дочерних процессов


timestamp_t timestamp = 0;


int main(int argc, char **argv) {

    /**====---- Command-line argument parsing ----====**/
    balance_t *initial_bank_accounts = NULL;
    X = get_optc(argc, argv, &initial_bank_accounts);
    N = X + 1;
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

            child_run(child_local_id, initial_bank_accounts[child_count]);

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

    free(initial_bank_accounts);

    printf("[PARENT] Shutting down... PID=[%d]\n", getpid());
    return 0;
}

void parent_run(pid_t children_PIDs[]) {
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);
    AllHistory allHistory = {
            .s_history_len = 0
    };

    int c;
    int count = 0;

    /**====---- START-messages RECEIVE ----====**/
    while (-1 != (c = receive_any(communicator, message))) {
        printf("[PARENT-got--START-message %d/%d ]\n", ++count, X);

        if (count == X)
            break;
    }

    /**====---- TRANSFERRING BALANCES  ----====**/
    printf("[PARENT] Robbing the bank!\n");
    bank_robbery(message, X);
    printf("[PARENT] Bank was robbed!\n");


    /**====---- STOP-messages SEND ----====**/
    printf("[PARENT] Sending STOP multicast!\n");
//    timestamp = get_physical_time();
    build_STOP_msg(message, timestamp);
    send_multicast(communicator, message);
    printf("[PARENT] Sent all STOP multicast!\n");


    /**====---- DONE-messages RECEIVE ----====**/
    /*count = 0;
    while (-1 != (c = receive_any(communicator, message))) {
        printf("[PARENT-got-DONE-message %d/%d ]\n", ++count, X);

        if (count == X)
            break;
    }*/

    /**====---- BALANCE_HISTORY-message RECEIVE ----====**/
    /*count = 0;
    while (-1 != (c = receive_any(communicator, message))) {
        BalanceHistory *balanceHistory = (BalanceHistory *) message->s_payload;

        printf("[PARENT-got--BALANCE_HISTORY-message %d/%d (%d)]\n", ++count, X, c);
        print_BalanceHistory(balanceHistory);

        append_AllHistory(&allHistory, balanceHistory);

        if (count == X)
            break;
    }*/

    local_id DONE_msg_count = 0;
    local_id BALANCE_HISTORY_msg_count = 0;
    /**====---- DONE-messages RECEIVE ----====**/
    /**====---- BALANCE_HISTORY-message RECEIVE ----====**/
    while (BALANCE_HISTORY_msg_count != X && -1 != (c = receive_any(communicator, message))) {
        switch (message->s_header.s_type) {
            case DONE:
                DONE_msg_count++;
                printf("[PARENT-got-DONE-message %d/%d ]\n", DONE_msg_count, X);
                break;
            case BALANCE_HISTORY:
                BALANCE_HISTORY_msg_count++;
                printf("[PARENT-got--BALANCE_HISTORY-message %d/%d (%d)]\n", BALANCE_HISTORY_msg_count, X, c);

                BalanceHistory *balanceHistory = (BalanceHistory *) message->s_payload;
                print_BalanceHistory(balanceHistory);
                append_AllHistory(&allHistory, balanceHistory);
                break;
        }
    }

    complete_AllHistory(&allHistory);

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

    /**====---- Initial balance SAVE ----====**/
//    timestamp = get_physical_time();
    append_BalanceHistory(&balanceHistory, bank_account, timestamp, 0);

    /**====---- START-messages SEND ----====**/
//    timestamp = get_physical_time();
    do_log_started_fmt(logger, timestamp, id, bank_account);
    build_log_started_msg(message, timestamp, id, bank_account);
    send_multicast(communicator, message);

    /**====---- START-messages RECEIVE ----====**/
    for (local_id child_local_id = 1; child_local_id <= X; child_local_id++) {
        if (child_local_id == id)
            continue;
        while (0 != receive(communicator, child_local_id, message)) {
            sleep(1);
        }
    }
//    timestamp = get_physical_time();
    do_log_received_all_started_fmt(logger, timestamp, id);

    /**====---- TRANSFER & STOP-messages processing ----====**/
    TransferOrder *transferOrder;
    local_id stopped_children_count = 0;
    while (stopped_children_count != X && -1 != receive_any(communicator, message)) {
//        timestamp = get_physical_time();

        switch (message->s_header.s_type) {
            case TRANSFER:
                if (message->s_header.s_local_time > timestamp) {
                    timestamp = message->s_header.s_local_time;
                }
                timestamp = get_physical_time();

                transferOrder = (TransferOrder *) message->s_payload;
                if (transferOrder->s_dst == id) {
                    printf("[CHILD #%d] got-TRANSFER-message %d -> %d IS_DESTINATION\n",
                           id, transferOrder->s_src, transferOrder->s_dst);

                    bank_account += transferOrder->s_amount;

                    /**====---- ACK-message SEND ----====**/
                    build_ACK_msg(message_out, timestamp);
                    send(communicator, PARENT_ID, message_out);
                    do_log_transfer_in_fmt(logger, timestamp, id, transferOrder->s_src, transferOrder->s_amount);
                    append_BalanceHistory(&balanceHistory, bank_account, timestamp, 0);
                    /**====---- FIN-message SEND ----====**/
                    build_FIN_msg(message_out, timestamp, transferOrder->s_amount);
                    send(communicator, transferOrder->s_src, message_out);
                } else {
                    printf("[CHILD #%d] got-TRANSFER-message %d -> %d IS_SENDER\n",
                           id, transferOrder->s_src, transferOrder->s_dst);

                    bank_account -= transferOrder->s_amount;

                    /**====---- TRANSFER-message SEND ----====**/
                    build_TRANSFER_msg(message_out, timestamp, transferOrder);
                    send(communicator, transferOrder->s_dst, message_out);
                    do_log_transfer_out_fmt(logger, timestamp, id, transferOrder->s_dst, transferOrder->s_amount);
                    append_BalanceHistory(&balanceHistory, bank_account, timestamp, transferOrder->s_amount);
                }
                break;
            case STOP:
                printf("[CHILD #%d] got-STOP-message\n", id);
                stopped_children_count++;

                /**====---- DONE-messages SEND ----====**/
                build_log_done_msg(message, timestamp, id, bank_account);
                send_multicast(communicator, message);
                do_log_done_fmt(logger, timestamp, id, bank_account);
                break;
            case DONE:
                /**====---- DONE-messages RECEIVE ----====**/
                printf("[CHILD #%d] got-DONE-message [stopped_children_count %d->%d]\n",
                       id,
                       stopped_children_count, stopped_children_count + 1);
                stopped_children_count++;
                break;
            case ACK:
                /**====---- FIN-messages RECEIVE ----====**/
                append_BalanceHistory(&balanceHistory, bank_account,
                                      message->s_header.s_local_time,
                                      balanceHistory.s_history[balanceHistory.s_history_len - 1].s_balance_pending_in -
                                      *(balance_t *) message->s_payload);
                break;
            default:
                printf("[CHILD #%d] got-message\n", id);
                break;
        }
    }
    if (stopped_children_count == X) {
        printf("[CHILD #%d] RECEIVED ALL DONE\n", id);
        do_log_received_all_done_fmt(logger, timestamp, id);
        goto SEND_BALANCE_HISTORY;
    } else {
        exit(-27);
    }

    /**====---- DONE-messages SEND ----====**/
    /*timestamp = get_physical_time();
    do_log_done_fmt(logger, timestamp, id, bank_account);
    build_log_done_msg(message, timestamp, id, bank_account);
    send_multicast(communicator, message);*/

    /**====---- DONE-messages RECEIVE ----====**/
    /*timestamp = get_physical_time();
    for (local_id child_count = 0; child_count < X; child_count++) {
        local_id child_local_id = child_count + 1;
        if (child_local_id == id)
            continue;
        while (0 != receive(communicator, child_local_id, message)) {
            sleep(1);
        }
    }
    do_log_received_all_done_fmt(logger, timestamp, id);*/

    SEND_BALANCE_HISTORY:
    /**====---- BALANCE_HISTORY-message SEND ----====**/
//    timestamp = get_physical_time();
    build_BALANCE_HISTORY_msg(message_out, timestamp, &balanceHistory);
    send(communicator, PARENT_ID, message_out);
    printf("[CHILD #%d] sent BALANCE_HISTORY\n", id);
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


void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    printf("[PARENT] Transferring %d -> %d\n", src, dst);
    Message *message;

    /**====---- Init variables ----====**/
    message = parent_data;
    TransferOrder transferOrder = {
            .s_src = src,
            .s_dst = dst,
            .s_amount = amount
    };
//    timestamp = get_physical_time();

    /**====---- Build Message ----====**/
    build_TRANSFER_msg(message, timestamp, &transferOrder);

    int res;
    /**====---- TRANSFER Message SEND ----====**/
    res = send(communicator, src, message);

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
}


timestamp_t get_lamport_time() {
    timestamp++;
    return timestamp;
}
