#ifndef PA1_COMMUNICATOR_H
#define PA1_COMMUNICATOR_H

#include <fcntl.h>

#include "pa1_starter_code/ipc.h"
#include "pa1_starter_code/common.h"


struct entry {
    int pipe_fd[2];
} __attribute__((packed));


struct communicator {
    struct entry *entries;
};

struct communicator * init_communicator(size_t N);


#endif //PA1_COMMUNICATOR_H
