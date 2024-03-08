#include <stdlib.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include "communicator.h"


struct communicator *init_communicator(size_t N) {
    struct entry *entries = calloc(N * N, sizeof(struct entry));

    int fd_pipes_log = open(pipes_log, O_CREAT | O_RDWR, 0777);


    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j)
                continue;

            struct entry *entry = &entries[i * N + j];

            pipe(entry->pipe_fd);

            char message[128];
            int msg_len = sprintf(message, "%d/%d\n", entry->pipe_fd[0], entry->pipe_fd[1]);
            write(fd_pipes_log, message, msg_len);
        }
    }

    close(fd_pipes_log);

    struct communicator *communicator = malloc(sizeof(struct communicator));
    communicator->entries = entries;
    return communicator;
}

/*void ipc_self_dtr(ipc_self **ipc_p) {
    if (ipc_p != NULL) {
        free((*ipc_p)->pipes);
        free(*ipc_p);
        *ipc_p = NULL;
    }
}

void ipc_self_set_cur(ipc_self *ipc, local_id cur) { ipc->cur = cur; }

int *get_pipe(ipc_self *ipc, local_id i, local_id j) {
    return ipc->pipes + 2 * (ipc->n * i + j);
}

void ipc_self_pipe_link(ipc_self *ipc) {
    for (int i = 0; i < ipc->n; ++i) {
        for (int j = 0; j < ipc->n; ++j) {
            int *pipe = get_pipe(ipc, i, j);
            if (i == j) {
                continue;
            } else if (i == ipc->cur) {
                close(pipe[0]);
            } else if (j == ipc->cur) {
                int rflags = fcntl(pipe[0], F_GETFL);
                rflags |= O_NONBLOCK;
                fcntl(pipe[0], F_SETFL, rflags);
                close(pipe[1]);
            } else {
                close(pipe[0]);
                close(pipe[1]);
            }
        }
    }
}

int ipc_self_wfd(ipc_self *ipc, local_id to) {
    return get_pipe(ipc, ipc->cur, to)[1];
}

int ipc_self_rfd(ipc_self *ipc, local_id from) {
    return get_pipe(ipc, from, ipc->cur)[0];
}*/
