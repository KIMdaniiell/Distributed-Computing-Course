#include "message_builder.h"


void set_msg_header(Message *new_message, uint16_t magic, int16_t type, uint16_t payload_len, timestamp_t local_time);


void build_log_STARTED_msg(Message *new_message, local_id id) {
    /**====---- Payload building ----====**/
    int msg_l = sprintf(new_message->s_payload, log_started_fmt,
                        id,
                        getpid(), getppid());

    /**====---- Header building ----====**/
    set_msg_header(new_message, MESSAGE_MAGIC, STARTED, msg_l, 0);
}

void build_log_DONE_msg(Message *new_message, local_id id) {
    /**====---- Payload building ----====**/
    int msg_l = sprintf(new_message->s_payload, log_done_fmt, id);

    /**====---- Header building ----====**/
    set_msg_header(new_message, MESSAGE_MAGIC, DONE, msg_l, 0);
}


void set_msg_header(Message *new_message, uint16_t magic, int16_t type, uint16_t payload_len, timestamp_t local_time) {
    new_message->s_header.s_magic = magic;
    new_message->s_header.s_type = type;
    new_message->s_header.s_payload_len = payload_len;
    new_message->s_header.s_local_time = local_time;
}
