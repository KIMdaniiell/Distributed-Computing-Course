
#include "messenger.h"

void init_log_started_fmt_message(Message *new_message, local_id id) {
//    int msg_l = strlen(log_started_fmt);
    int msg_l = sprintf(new_message->s_payload, log_started_fmt, id, getpid(), getppid());

    new_message->s_header.s_magic = MESSAGE_MAGIC;
    new_message->s_header.s_type = STARTED;
    new_message->s_header.s_payload_len = msg_l;
    new_message->s_header.s_local_time = 0;
}

/*void init_log_received_all_started_fmt(Message * new_message, local_id id) {
//    int msg_l = strlen(log_received_all_started_fmt);
    int msg_l = sprintf(new_message->s_payload, log_received_all_started_fmt, id);

    new_message->s_header.s_magic = MESSAGE_MAGIC;
    new_message->s_header.s_type = STARTED;
    new_message->s_header.s_payload_len = msg_l;
    new_message->s_header.s_local_time = 0;
}*/

void init_log_done_fmt(Message *new_message, local_id id) {
//    int msg_l = strlen(log_done_fmt);
    int msg_l = sprintf(new_message->s_payload, log_done_fmt, id);

    new_message->s_header.s_magic = MESSAGE_MAGIC;
    new_message->s_header.s_type = DONE;
    new_message->s_header.s_payload_len = msg_l;
    new_message->s_header.s_local_time = 0;
}

/*void init_log_received_all_done_fmt(Message * new_message, local_id id) {
//    int msg_l = strlen(log_received_all_done_fmt);
    int msg_l = sprintf(new_message->s_payload, log_received_all_done_fmt, id);

    new_message->s_header.s_magic = MESSAGE_MAGIC;
    new_message->s_header.s_type = DONE;
    new_message->s_header.s_payload_len = msg_l;
    new_message->s_header.s_local_time = 0;
}*/
