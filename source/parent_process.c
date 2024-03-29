#include "parent_process.h"

void pre_parent_run() {
    pre_run();
    printf("%3d: [PARENT] Starting ... PID=[%d]\n", timestamp, getpid());
}

void post_parent_run() {
    post_run();
    printf("%3d: [PARENT] Shutting down... PID=[%d]\n", timestamp, getpid());
}

void parent_run(pid_t children_PIDs[]) {
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);

    /**====---- START-messages RECEIVE ----====**/
    receive_multicast_wrapper(STARTED, message);

    /**====---- DONE-messages RECEIVE ----====**/
//    receive_multicast_wrapper(DONE, message);

    /**====---- Children WAITING ----====**/
    for (int child_count = 0; child_count < X; child_count++) {
        int child_status = 0;

        waitpid(children_PIDs[child_count], &child_status, 0);
        printf("%3d: [PARENT-wait-complete pid=%d %d/%d]\n", timestamp, children_PIDs[child_count], child_count + 1, X);
    }
}

void parent_run_full(pid_t children_PIDs[]) {
    pre_parent_run();
    parent_run(children_PIDs);
    post_parent_run();
}
