#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>


#include "ipc.h"
#include "communicator.h"

/** Send a message to the process specified by id.
 *
 * @param self    Any data structure implemented by students to perform I/O
 * @param dst     ID of recipient
 * @param msg     Message to send
 *
 * @return 0 on success, any non-zero value on error
 */
int send(void *self, local_id dst, const Message *msg) {
    struct communicator *communicator = (struct communicator *) self;

    if (dst >= communicator->header.N || dst < 0)
        return -3;

    struct entry *entry = get_entry_to_write(communicator, dst);

    int bytes_cnt = write(entry->write_fd, msg, msg->s_header.s_payload_len + sizeof(MessageHeader));

    /*printf("\t[%02d] sends to [%02d] with pipe № %d \t%d/%lu [H=%lu;P=%d]\n",
           communicator->header.owner_id,
           dst,
           entry->write_fd,
           bytes_cnt,
           msg->s_header.s_payload_len + sizeof(MessageHeader),
           sizeof(MessageHeader),
           msg->s_header.s_payload_len);*/

    if (bytes_cnt == -1) {
        return -1;
    }
    if (bytes_cnt != msg->s_header.s_payload_len + sizeof(MessageHeader)) {
        printf("Has to write %lu, but written %d\n", msg->s_header.s_payload_len + sizeof(MessageHeader), bytes_cnt);
        return -2;
    } else {
        return 0;
    }
}

//------------------------------------------------------------------------------

/** Send multicast message.
 *
 * Send msg to all other processes including parent.
 * Should stop on the first error.
 *
 * @param self    Any data structure implemented by students to perform I/O
 * @param msg     Message to multicast.
 *
 * @return 0 on success, any non-zero value on error
 */
int send_multicast(void *self, const Message *msg) {
    struct communicator *communicator = (struct communicator *) self;
    local_id owner_id = communicator->header.owner_id;
    int N = communicator->header.N;

    for (int i = 0; i < N; i++) {
        if (i == owner_id)
            continue;
        send(communicator, i, msg);
    }

    return 0;
}

//------------------------------------------------------------------------------

/** Receive a message from the process specified by id.
 *
 * Might block depending on IPC settings.
 *
 * @param self    Any data structure implemented by students to perform I/O
 * @param from    ID of the process to receive message from
 * @param msg     Message structure allocated by the caller
 *
 * @return 0 on success, any non-zero value on error
 */
int receive(void *self, local_id from, Message *msg) {
    struct communicator *communicator = (struct communicator *) self;

    if (from >= communicator->header.N || from < 0)
        return -3;

    struct entry *entry = get_entry_to_read(communicator, from);


    ssize_t bytes_cnt;
    if ((bytes_cnt = read(entry->read_fd, msg, sizeof(MessageHeader)) > 0)) {
        int payload_l = msg->s_header.s_payload_len;

        if (bytes_cnt == 0) {
            printf("[receive] EOF!\n");
            return -404;
        } else if (bytes_cnt == -1) {
            printf("[receive] Read error!\n");
            exit(-1);
        }

        /**====---- Вариант 1 - ждем пока не получим что хотим ----====**/
        /*for (;;) {
            if (read(entry->read_fd, msg->s_payload, payload_l) != -1) {
                return 0;
            }
            sleep(1);
        }*/

        /**====---- Вариант 2 - надеемся что все получим всё что хотим ----====**/
        /*read(entry->read_fd, msg->s_payload, payload_l);        // есть шанс не прочитать
        return 0;*/

        /**====---- Вариант 3 - надеемся что все получим всё что хотим, возвращаем ошибку ----====**/
        if (read(entry->read_fd, msg->s_payload, payload_l) != -1 )        // есть шанс не прочитать
            return 0;
        else
            return -4;
    }
    return -1;
}

//------------------------------------------------------------------------------

/** Receive a message from any process.
 *
 * Receive a message from any process, in case of blocking I/O should be used
 * with extra care to avoid deadlocks.
 *
 * @param self    Any data structure implemented by students to perform I/O
 * @param msg     Message structure allocated by the caller
 *
 * @return 0 on success, any non-zero value on error
 */
int receive_any(void *self, Message *msg) {
    struct communicator *communicator = (struct communicator *) self;

    int N = communicator->header.N;
    int receiver_id = communicator->header.owner_id;
    int pipes_to_read = N - 1;

    int ps_id = 0;
    for (;;) {

        if (ps_id + 1 < N) {
            ps_id++;
        } else {
            ps_id = 0;
            sleep(1);
        }

        if (ps_id == receiver_id)
            continue;

        int res_code = receive(communicator, ps_id, msg);

//        printf("[id = %d] [ptr = %d] [code = %d]\n", ps_id , pipes_to_read, res_code);
        /*printf("[id = %d] [from = %d] [pp_to_r = %d] [code = %d] [c_N = %d]\n",
               communicator->header.owner_id , ps_id,
               pipes_to_read,
               res_code,
               communicator->header.N);*/

        if (res_code == -404)
            pipes_to_read--;

        if (res_code == 0)
            return 0;
    }
    return -1;
}


