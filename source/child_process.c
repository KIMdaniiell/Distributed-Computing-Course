#include "child_process.h"

void pre_child_run() {
    pre_run();
}

void post_child_run() {
    post_run();
    printf("[CHILD #%d] Shutting down... PID=[%d]\n", process_id, getpid());
    exit(0);
}

void child_run(int bank_account) {
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);
    Message *message_out = (Message *) calloc(1, MAX_MESSAGE_LEN);
    BalanceHistory balanceHistory = {
            .s_id = process_id
    };

    /**====---- Initial balance SAVE ----====**/
    append_BalanceHistory(&balanceHistory, bank_account, timestamp, 0);

    /**====---- START-messages SEND ----====**/
    do_log_started_fmt(logger, timestamp, process_id, bank_account);
    build_log_STARTED_msg(message, timestamp, process_id, bank_account);
    decorated_send_multicast(communicator, message, &timestamp);

    /**====---- START-messages RECEIVE ----====**/
    for (local_id child_local_id = 1; child_local_id <= X; child_local_id++) {
        if (child_local_id == process_id)
            continue;
        decorated_receive(communicator, child_local_id, message, &timestamp);
    }
    do_log_received_all_started_fmt(logger, timestamp, process_id);

    /**====---- TRANSFER & STOP-messages processing ----====**/
    TransferOrder *transferOrder;
    local_id stopped_children_count = 0;
    while (stopped_children_count != X && -1 != decorated_receive_any(communicator, message, &timestamp)) {
        switch (message->s_header.s_type) {
            case TRANSFER:
                transferOrder = (TransferOrder *) message->s_payload;
                if (transferOrder->s_dst == process_id) {
                    printf("[CHILD #%d] got-TRANSFER-message %d -> %d IS_DESTINATION\n",
                           process_id, transferOrder->s_src, transferOrder->s_dst);

                    bank_account += transferOrder->s_amount;

                    /**====---- ACK-message SEND ----====**/
                    build_ACK_msg(message_out, timestamp);
                    decorated_send(communicator, PARENT_ID, message_out, &timestamp);
                    do_log_transfer_in_fmt(logger, timestamp, process_id, transferOrder->s_src, transferOrder->s_amount);
                    append_BalanceHistory(&balanceHistory, bank_account, timestamp, 0);

                    /**====---- FIN-message SEND ----====**/
                    build_FIN_msg(message_out, timestamp, transferOrder->s_amount);
                    send(communicator, transferOrder->s_src, message_out);
                } else {
                    printf("[CHILD #%d] got-TRANSFER-message %d -> %d IS_SENDER\n",
                           process_id, transferOrder->s_src, transferOrder->s_dst);

                    bank_account -= transferOrder->s_amount;

                    /**====---- TRANSFER-message SEND ----====**/
                    build_TRANSFER_msg(message_out, timestamp, transferOrder);
                    decorated_send(communicator, transferOrder->s_dst, message_out, &timestamp);
                    do_log_transfer_out_fmt(logger, timestamp, process_id, transferOrder->s_dst, transferOrder->s_amount);
                    append_BalanceHistory(&balanceHistory, bank_account, timestamp, transferOrder->s_amount);
                }
                break;
            case STOP:
                printf("[CHILD #%d] got-STOP-message\n", process_id);
                stopped_children_count++;

                /**====---- DONE-messages SEND ----====**/
                build_log_DONE_msg(message, timestamp, process_id, bank_account);
                decorated_send_multicast(communicator, message, &timestamp);
                do_log_done_fmt(logger, timestamp, process_id, bank_account);
                break;
            case DONE:
                /**====---- DONE-messages RECEIVE ----====**/
                printf("[CHILD #%d] got-DONE-message [stopped_children_count %d->%d]\n",
                       process_id,
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
                printf("[CHILD #%d] got-message\n", process_id);
                break;
        }
    }
    if (stopped_children_count == X) {
        printf("[CHILD #%d] RECEIVED ALL DONE\n", process_id);
        do_log_received_all_done_fmt(logger, timestamp, process_id);
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
    build_BALANCE_HISTORY_msg(message_out, timestamp, &balanceHistory);
    decorated_send(communicator, PARENT_ID, message_out, &timestamp);
    printf("[CHILD #%d] sent BALANCE_HISTORY\n", process_id);
}

void child_run_full(int bank_account) {
    pre_child_run();
    child_run(bank_account);
    post_child_run();
}
