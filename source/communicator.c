#include "communicator.h"


void close_commutation(struct communicator *communicator, local_id id);


struct communicator *init_communicator(size_t N) {
    struct entry *entries = calloc(N * N, sizeof(struct entry));

    int fd_pipes_log = open(pipes_log, O_CREAT | O_RDWR | O_TRUNC, 0777);


    for (int i = 0; i < N; i++) {
        printf(" %d)", i);
        for (int j = 0; j < N; j++) {
            if (i == j) {
                printf("     [READ=__;WRITE=__]");
                continue;
            }

            if (i > j) {
                struct entry *entry = &entries[i * N + j];
                printf("     [READ=%2d;WRITE=%2d]", entry->read_fd, entry->write_fd);
                continue;
            }

            int pipe_fds_i_to_j[2];
            int pipe_fds_j_to_i[2];
            pipe(pipe_fds_i_to_j);
            pipe(pipe_fds_j_to_i);

            struct entry *entry = &entries[i * N + j];
            struct entry *entry_reverse = &entries[j * N + i];

            entry->write_fd = pipe_fds_i_to_j[1];
            entry->read_fd = pipe_fds_j_to_i[0];

//            const int flags_i_to_j = fcntl(entry->read_fd, F_GETFL, 0);
//            fcntl(entry->read_fd, F_SETFL, flags_i_to_j | O_NONBLOCK);
            const int flags_i_to_j_read = fcntl(entry->read_fd, F_GETFL, 0);
            fcntl(entry->read_fd, F_SETFL, flags_i_to_j_read | O_NONBLOCK);
            const int flags_i_to_j_write = fcntl(entry->write_fd, F_GETFL, 0);
            fcntl(entry->write_fd, F_SETFL, flags_i_to_j_write | O_NONBLOCK);

            entry_reverse->write_fd = pipe_fds_j_to_i[1];
            entry_reverse->read_fd = pipe_fds_i_to_j[0];

//            const int flags_j_to_i = fcntl(entry_reverse->read_fd, F_GETFL, 0);
//            fcntl(entry_reverse->read_fd, F_SETFL, flags_j_to_i | O_NONBLOCK);
            const int flags_j_to_i_read = fcntl(entry_reverse->read_fd, F_GETFL, 0);
            fcntl(entry_reverse->read_fd, F_SETFL, flags_j_to_i_read | O_NONBLOCK);
            const int flags_j_to_i_write = fcntl(entry_reverse->write_fd, F_GETFL, 0);
            fcntl(entry_reverse->write_fd, F_SETFL, flags_j_to_i_write | O_NONBLOCK);

            printf("     [READ=%2d;WRITE=%2d]", entry->read_fd, entry->write_fd);

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

    int owner_id = communicator->header.owner_id;

    close_commutation(communicator, owner_id);

    free(communicator->entries);
    free(communicator);
}

void optimise_communicator(struct communicator *communicator) {
    int N = communicator->header.N;
    int owner_id = communicator->header.owner_id;

//    printf("%d) ", owner_id);
    for (local_id i = 0; i < N; i++) {
        if (i == owner_id)
            continue;

        close_commutation(communicator, i);
    }
//    printf("\n");
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


void close_commutation(struct communicator *communicator, local_id id) {
    if (NULL == communicator)
        return;

    int N = communicator->header.N;

    for (int i = 0; i < N; i++) {
        if (i == id)
            continue;

        struct entry *entry = &communicator->entries[id * N + i];
//        printf(" %d", entry->read_fd);
//        printf(" %d", entry->write_fd);
        close(entry->read_fd);
        close(entry->write_fd);
    }
}
