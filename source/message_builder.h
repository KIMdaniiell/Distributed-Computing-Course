#ifndef PA1_MESSENGER_H
#define PA1_MESSENGER_H

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "pa1.h"
#include "ipc.h"
#include "logger.h"


void build_log_STARTED_msg(Message *new_message, local_id id);

void build_log_DONE_msg(Message *new_message, local_id id);

#endif //PA1_MESSENGER_H
