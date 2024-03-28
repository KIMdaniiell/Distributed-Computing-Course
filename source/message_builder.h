#ifndef PA1_MESSAGE_BUILDER_H
#define PA1_MESSAGE_BUILDER_H

#include <string.h>

#include "pa2345.h"
#include "ipc.h"
#include "banking.h"


void build_log_started_msg(Message *new_message, timestamp_t timestamp, local_id id, balance_t balance);

void build_log_done_msg(Message *new_message, timestamp_t timestamp, local_id id, balance_t balance);

/*void build_log_received_all_started_msg(Message *new_message, timestamp_t timestamp, local_id id);

void build_log_transfer_out_msg(Message *new_message, timestamp_t timestamp, local_id id, local_id target_id, balance_t balance);

void build_log_transfer_in_msg(Message *new_message, timestamp_t timestamp, local_id id, local_id target_id, balance_t balance;

void build_log_received_all_done_msg(Message *new_message, timestamp_t timestamp, local_id id);*/

void build_TRANSFER_msg(Message *new_message, timestamp_t timestamp, TransferOrder *transferOrder);

void build_ACK_msg(Message *new_message, timestamp_t timestamp);

void build_STOP_msg(Message *new_message, timestamp_t timestamp);

void build_BALANCE_HISTORY_msg(Message *new_message, timestamp_t timestamp, BalanceHistory *balanceHistory);

#endif //PA1_MESSAGE_BUILDER_H
