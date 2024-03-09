#ifndef PA1_MESSAGES_H
#define PA1_MESSAGES_H

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "pa1.h"
#include "common.h"

#define MESSAGE_LENGTH 128


struct logger {
    int file_descriptor;
    char log_buffer[MESSAGE_LENGTH];
    int log_length;
};

struct logger *init_logger();

int do_log_started_fmt(struct logger *logger, int local_id);

int do_log_received_all_started_fmt(struct logger *logger, int local_id);

int do_log_done_fmt(struct logger *logger, int local_id);

int do_log_received_all_done_fmt(struct logger *logger, int local_id);

void close_logger(struct logger *logger);

#endif //PA1_MESSAGES_H
