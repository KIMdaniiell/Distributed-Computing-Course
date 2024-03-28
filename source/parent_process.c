#include "parent_process.h"

void pre_parent_run() {
    pre_run();
    printf("[PARENT] Starting ... PID=[%d]\n", getpid());
}

void post_parent_run() {
    post_run();
    printf("[PARENT] Shutting down... PID=[%d]\n", getpid());
}

void parent_run(pid_t children_PIDs[]) {
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);
    AllHistory allHistory = {
            .s_history_len = 0
    };

    int c;
    int count = 0;

    /**====---- START-messages RECEIVE ----====**/
    while (-1 != decorated_receive_any(communicator, message, &timestamp)) {
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
    build_STOP_msg(message, timestamp);
    decorated_send_multicast(communicator, message, &timestamp);
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

    /**====---- DONE-messages RECEIVE ----====**/
    /**====---- BALANCE_HISTORY-message RECEIVE ----====**/
    local_id DONE_msg_count = 0;
    local_id BALANCE_HISTORY_msg_count = 0;
    while (BALANCE_HISTORY_msg_count != X && -1 != (c = decorated_receive_any(communicator, message, &timestamp))) {
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

    /**====---- AllHistory process ----====**/
    complete_AllHistory(&allHistory);
    print_history(&allHistory);

    /**====---- Children WAITING ----====**/
    for (int child_count = 0; child_count < X; child_count++) {
        int child_status = 0;

        waitpid(children_PIDs[child_count], &child_status, 0);
        printf("[PARENT-wait-complete pid=%d %d/%d]\n", children_PIDs[child_count], child_count + 1, X);
    }
}

void parent_run_full(pid_t children_PIDs[]) {
    pre_parent_run();
    parent_run(children_PIDs);
    post_parent_run();
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

    /**====---- Build Message ----====**/
    build_TRANSFER_msg(message, timestamp, &transferOrder);

    /**====---- TRANSFER Message SEND ----====**/
    decorated_send(communicator, src, message, &timestamp);

    /**====---- ACK Message RECEIVE ----====**/
    int res;
    for (;;) {
        res = decorated_receive(communicator, dst, message, &timestamp);

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
