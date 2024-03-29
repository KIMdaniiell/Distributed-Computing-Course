#ifndef PA1_MESSAGES_H
#define PA1_MESSAGES_H

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "pa2345.h"
//#include "banking.h"
#include "common.h"

#define MESSAGE_LENGTH 128


struct logger {
    int file_descriptor;
    char log_buffer[MESSAGE_LENGTH];
    int log_length;
};

struct logger *init_logger();

int do_log_started_fmt(struct logger *logger, timestamp_t timestamp, local_id id);

int do_log_received_all_started_fmt(struct logger *logger, timestamp_t timestamp, local_id id);

int do_log_done_fmt(struct logger *logger, timestamp_t timestamp, local_id id);

int do_log_transfer_out_fmt(struct logger *logger, timestamp_t timestamp, local_id id, local_id target_id);

int do_log_transfer_in_fmt(struct logger *logger, timestamp_t timestamp, local_id id, local_id target_id);

int do_log_received_all_done_fmt(struct logger *logger, timestamp_t timestamp, local_id id);

void close_logger(struct logger *logger);


char* get_msg_type_name(struct logger* logger, MessageType type);


#endif //PA1_MESSAGES_H
