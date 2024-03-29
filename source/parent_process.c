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
//    Message *message = (Message *) calloc(1, MAX_MESSAGE_LEN);

    /**====---- START-messages RECEIVE ----====**/
//    receive_multicast_wrapper(STARTED, message);

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

void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    printf("[PARENT] Transferring %d -> %d\n", src, dst);
    Message *message;

    /**====---- Init variables ----====**/
    message = parent_data;
    TransferOrder transferOrder = {
            .s_src = src,
            .s_dst = dst,
            .s_amount = amount
    };

    /**====---- Build Message ----====**/
    build_TRANSFER_msg(message, timestamp, &transferOrder);

    /**====---- TRANSFER Message SEND ----====**/
    decorated_send(communicator, src, message, &timestamp);

    /**====---- ACK Message RECEIVE ----====**/
    int res;
    for (;;) {
        res = decorated_receive(communicator, dst, message, &timestamp);

        if (res == 0) {
            /*printf("[id = %d] [from = %d] [code = %d] [c_N = %d]\n",
                   PARENT_ID , dst,
                   res,
                   communicator->header.N);*/
            TransferOrder *receive_transferOrder = (TransferOrder *) message->s_payload;
            printf("[PARENT] got-TRANSFER-message %d -> %d ACKNOWLEDGE\n",
                   receive_transferOrder->s_src,
                   receive_transferOrder->s_dst);
            break;
        } else {
            sleep(1);
        }
    }
}
