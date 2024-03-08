#include <stdlib.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include "communicator.h"


struct communicator *init_communicator(size_t N) {
    struct entry *entries = calloc(N * N, sizeof(struct entry));

    int fd_pipes_log = open(pipes_log, O_CREAT | O_RDWR | O_TRUNC, 0777);


    for (int i = 0; i < N; i++) {
        printf(" %d)", i);
        for (int j = 0; j < N; j++) {
            if (i == j) {
                printf("     [READ=_;WRITE=_]");
                continue;
            }

            if (i > j) {
                struct entry *entry = &entries[i * N + j];
                printf("     [READ=%d;WRITE=%d]", entry->read_fd, entry->write_fd);
                continue;
            }

            // i - transmitter (writes)     [1]
            // j - receiver (reads)         [0]
            int pipe_fds_i_to_j[2];
            int pipe_fds_j_to_i[2];
            pipe(pipe_fds_i_to_j);
            pipe(pipe_fds_j_to_i);

            struct entry *entry = &entries[i * N + j];
            struct entry *entry_reverse = &entries[j*N + i];

            entry->write_fd = pipe_fds_i_to_j[1];
            entry->read_fd = pipe_fds_j_to_i[0];

            const int flags_i_to_j = fcntl(entry->read_fd, F_GETFL, 0);
            fcntl(entry->read_fd, F_SETFL, flags_i_to_j | O_NONBLOCK);

            entry_reverse->write_fd = pipe_fds_j_to_i[1];
            entry_reverse->read_fd = pipe_fds_i_to_j[0];

            const int flags_j_to_i = fcntl(entry_reverse->read_fd, F_GETFL, 0);
            fcntl(entry_reverse->read_fd, F_SETFL, flags_j_to_i | O_NONBLOCK);

            printf("     [READ=%d;WRITE=%d]", entry->read_fd, entry->write_fd);

            char message[128];
            int msg_len = sprintf(message, "%d/%d\n", pipe_fds_i_to_j[0], pipe_fds_i_to_j[1]);
            write(fd_pipes_log, message, msg_len);

            msg_len = sprintf(message, "%d/%d\n", pipe_fds_j_to_i[0], pipe_fds_j_to_i[1]);
            write(fd_pipes_log, message, msg_len);
        }
        printf("\n");
    }

    close(fd_pipes_log);

    struct communicator *communicator = malloc(sizeof(struct communicator));
    communicator->entries = entries;
    return communicator;
}

void close_communicator(struct communicator *communicator) {
    if (NULL == communicator)
        return;

    free(communicator->entries);
    free(communicator);
}

struct entry *get_entry_to_write(struct communicator *communicator, local_id receiver_id) {
    int N = communicator->header.N;
    local_id owner_id = communicator->header.owner_id;
    return &communicator->entries[N * owner_id + receiver_id];
}

struct entry *get_entry_to_read(struct communicator *communicator, local_id transmitter_id) {
    int N = communicator->header.N;
    local_id owner_id = communicator->header.owner_id;

    return &communicator->entries[N * owner_id + transmitter_id];
}
