#include "child_process.h"

#define CALL_C 3


void pre_child_run() {
    pre_run();
}

void post_child_run() {
    post_run();
    printf("%3d: [CHILD #%d] B-BYE PID=[%d]\n", timestamp, process_id, getpid());
    exit(0);
}

void child_run(bool mutex_l_is_on) {
    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);
    char *loop_operation_msg_buffer = calloc(1, MAX_PAYLOAD_LEN);


    /**====---- START-messages SEND ----====**/
    do_log_started_fmt(logger, timestamp, process_id, 0);
    send_multicast_wrapper(STARTED, message);

    /**====---- START-messages RECEIVE ----====**/
    receive_multicast_wrapper(STARTED, message);
    do_log_received_all_started_fmt(logger, timestamp, process_id);



    /**====---- CS ----====**/
    for (int call_count = 1; call_count <= process_id * CALL_C; call_count++) {
        sprintf(loop_operation_msg_buffer, log_loop_operation_fmt,
                process_id, call_count, process_id * CALL_C);

        if (mutex_l_is_on) {
            printf("%3d: [CHILD #%d] ----- REQUESTING CRITICAL-SECTION >>>\n", timestamp, process_id);
            request_cs(message);
            printf("%3d: [CHILD #%d] ----- EXECUTING CRITICAL-SECTION !!!\n", timestamp, process_id);

            print(loop_operation_msg_buffer);

            printf("%3d: [CHILD #%d] ----- RELEASING CRITICAL-SECTION <<<\n", timestamp, process_id);
            release_cs(message);
        } else {
            print(loop_operation_msg_buffer);
        }
    }


    /**====---- DONE-messages SEND ----====**/
    printf("%3d: [CHILD #%d] BYE BYE Im DONE \n", timestamp, process_id);
    do_log_done_fmt(logger, timestamp, process_id, 0);
    send_multicast_wrapper(DONE, message);

    /**====---- DONE-messages RECEIVE ----====**/
    receive_multicast_wrapper(DONE, message);
    do_log_received_all_done_fmt(logger, timestamp, process_id);
    free(loop_operation_msg_buffer);
}

void child_run_full(bool mutex_l_is_on) {
    pre_child_run();
    child_run(mutex_l_is_on);
    post_child_run();
}
