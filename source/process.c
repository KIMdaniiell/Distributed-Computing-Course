#include "process.h"

struct communicator *communicator;
struct logger *logger;

local_id process_id;

timestamp_t timestamp;


void set_local_id(local_id local_id) {
    process_id = local_id;
}

void set_process_communicator() {
    printf("\n");
    communicator = init_communicator(N);
    printf("\n");

    communicator->header.N = N;
    communicator->header.owner_id = -1;
}

void set_process_logger() {
    logger = init_logger();
}

void pre_run() {
    communicator->header.owner_id = process_id;
    optimise_communicator(communicator);
}

void post_run() {
    close_communicator(communicator);
    close_logger(logger);
}

timestamp_t get_lamport_time() {
    return timestamp;
}
