#ifndef PA1_LAMPORT_IPC_H
#define PA1_LAMPORT_IPC_H


#include <unistd.h>
#include <time.h>

#include "ipc.h"
//#include "banking.h"

int decorated_send(void *self, local_id dst, Message *msg, timestamp_t *timestamp_ptr);

int decorated_send_multicast(void *self, Message *msg, timestamp_t *timestamp_ptr);

int decorated_receive(void *self, local_id from, Message *msg, timestamp_t *timestamp_ptr);

int decorated_receive_any(void *self, Message *msg, timestamp_t *timestamp_ptr);

#endif //PA1_LAMPORT_IPC_H
