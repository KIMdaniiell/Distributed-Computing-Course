#ifndef PA1_MESSENGER_H
#define PA1_MESSENGER_H

#include <string.h>

#include "pa1.h"
#include "ipc.h"
#include "logger.h"


void init_log_started_fmt_message(Message * new_message, local_id id);

void init_log_received_all_started_fmt(Message * new_message, local_id id);

void init_log_done_fmt(Message * new_message, local_id id);

void init_log_received_all_done_fmt(Message * new_message, local_id id);

#endif //PA1_MESSENGER_H
