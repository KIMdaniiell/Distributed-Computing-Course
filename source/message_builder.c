#include "message_builder.h"

void set_msg_header(Message *new_message, uint16_t magic, int16_t type, uint16_t payload_len, timestamp_t local_time);


void build_log_started_msg(Message *new_message, timestamp_t timestamp, local_id id, balance_t balance) {
    int msg_l = sprintf(new_message->s_payload, log_started_fmt,
                        timestamp,
                        id,
                        getpid(), getppid(),
                        balance);

    set_msg_header(new_message, MESSAGE_MAGIC, STARTED, msg_l, timestamp);
}

/*void build_log_received_all_started_msg(Message *new_message, timestamp_t timestamp, local_id id) {
    int msg_l = sprintf(new_message->s_payload, log_received_all_started_fmt,
                        timestamp,
                        id);

    new_message->s_header.s_magic = MESSAGE_MAGIC;
    new_message->s_header.s_type = STARTED;
    new_message->s_header.s_payload_len = msg_l;
    new_message->s_header.s_local_time = timestamp;
}*/

void build_log_done_msg(Message *new_message, timestamp_t timestamp, local_id id, balance_t balance) {
    int msg_l = sprintf(new_message->s_payload, log_done_fmt,
                        timestamp,
                        id,
                        balance);

    set_msg_header(new_message, MESSAGE_MAGIC, DONE, msg_l, timestamp);
}

/*void build_log_transfer_out_msg(Message *new_message, timestamp_t timestamp, local_id id, local_id target_id,
                                balance_t balance) {
    int msg_l = sprintf(new_message->s_payload, log_transfer_out_fmt,
                        timestamp,
                        id,
                        balance,
                        target_id);

    // TODO message header building
}*/

/*void build_log_transfer_in_msg(Message *new_message, timestamp_t timestamp, local_id id, local_id target_id,
                               balance_t balance {
    int msg_l = sprintf(new_message->s_payload, log_transfer_in_fmt,
                        timestamp,
                        id,
                        balance,
                        target_id);

    // TODO message header building
}*/

/*void build_log_received_all_done_msg(Message *new_message, timestamp_t timestamp, local_id id) {
    int msg_l = sprintf(new_message->s_payload, log_received_all_done_fmt,
                        timestamp,
                        id);

    new_message->s_header.s_magic = MESSAGE_MAGIC;
    new_message->s_header.s_type = DONE;
    new_message->s_header.s_payload_len = msg_l;
    new_message->s_header.s_local_time = timestamp;
}*/

void build_TRANSFER_msg(Message *new_message, timestamp_t timestamp, TransferOrder *transferOrder) {
    // TODO payload building
    int msg_l = sizeof(TransferOrder);
    TransferOrder *payload = (TransferOrder *) new_message->s_payload;
    payload->s_src = transferOrder->s_src;
    payload->s_dst = transferOrder->s_dst;
    payload->s_amount = transferOrder->s_amount;

    // TODO msg-header building
    set_msg_header(new_message, MESSAGE_MAGIC, TRANSFER, msg_l, timestamp);
}

void build_ACK_msg(Message *new_message, timestamp_t timestamp) {
    // TODO payload building
    int msg_l = 0;

    // TODO msg-header building
    set_msg_header(new_message, MESSAGE_MAGIC, ACK, msg_l, timestamp);
}

void build_STOP_msg(Message *new_message, timestamp_t timestamp) {
    // TODO payload building
    int msg_l = 0;

    // TODO msg-header building
    set_msg_header(new_message, MESSAGE_MAGIC, STOP, msg_l, timestamp);
}

void build_BALANCE_HISTORY_msg(Message *new_message, timestamp_t timestamp, BalanceHistory *balanceHistory) {
    // TODO payload building
    // TODO compute real payload len according to BalanceHistory . s_history_len
    int msg_l = sizeof(BalanceHistory);
    memcpy(new_message->s_payload, balanceHistory, msg_l);

    // TODO msg-header building
    set_msg_header(new_message, MESSAGE_MAGIC, BALANCE_HISTORY, msg_l, timestamp);
}


void set_msg_header(Message *new_message, uint16_t magic, int16_t type, uint16_t payload_len, timestamp_t local_time) {
    new_message->s_header.s_magic = magic;
    new_message->s_header.s_type = type;
    new_message->s_header.s_payload_len = payload_len;
    new_message->s_header.s_local_time = local_time;
}
