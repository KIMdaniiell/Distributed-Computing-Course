#ifndef PA1_COMMUNICATOR_H
#define PA1_COMMUNICATOR_H

#include <fcntl.h>

#include "ipc.h"
#include "common.h"


struct entry {
    int read_fd;
    int write_fd;
} __attribute__((packed));

struct communicator_header {
    int N;
    local_id owner_id;
};

struct communicator {
    struct communicator_header header;
    struct entry *entries;
};

struct communicator * init_communicator(size_t N);

void close_communicator(struct communicator *communicator);

struct entry *get_entry_to_write(struct communicator *communicator, local_id receiver_id);

struct entry *get_entry_to_read(struct communicator *communicator, local_id transmitter_id);


#endif //PA1_COMMUNICATOR_H
