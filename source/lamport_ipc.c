#include "lamport_ipc.h"


void set_timestamp(timestamp_t *timestamp_ptr, timestamp_t timestamp);

void increment_timestamp(timestamp_t *timestamp_ptr);


int decorated_send(void *self, local_id dst, Message *msg, timestamp_t *timestamp_ptr) {
    increment_timestamp(timestamp_ptr);
    msg->s_header.s_local_time = *timestamp_ptr;
    return send(self, dst, msg);
}

int decorated_send_multicast(void *self, Message *msg, timestamp_t *timestamp_ptr) {
    increment_timestamp(timestamp_ptr);
    msg->s_header.s_local_time = *timestamp_ptr;
    return send_multicast(self, msg);
}

// blocking
int decorated_receive(void *self, local_id from, Message *msg, timestamp_t *timestamp_ptr) {
    int res;
    timestamp_t received_timestamp;

    for (;;) {
        res = receive(self, from, msg);

        if (res == 0) {
            received_timestamp = msg->s_header.s_local_time;
            if (received_timestamp > *timestamp_ptr) {
                set_timestamp(timestamp_ptr, received_timestamp);
            }
            increment_timestamp(timestamp_ptr);
            return res;
        } else {
            sleep(1);
        }
    }
}

int decorated_receive_any(void *self, Message *msg, timestamp_t *timestamp_ptr) {
    int res = receive_any(self, msg);

    timestamp_t received_timestamp = msg->s_header.s_local_time;
    if (received_timestamp > *timestamp_ptr) {
        set_timestamp(timestamp_ptr, received_timestamp);
    }
    increment_timestamp(timestamp_ptr);

    return res;
}


void set_timestamp(timestamp_t *timestamp_ptr, timestamp_t timestamp) {
    *timestamp_ptr = timestamp;
}

void increment_timestamp(timestamp_t *timestamp_ptr) {
    *timestamp_ptr += 1;
}
