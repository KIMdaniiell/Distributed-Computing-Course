#ifndef PA1_PROCESS_H
#define PA1_PROCESS_H

#include <sys/types.h>
#include <unistd.h>

#include "ipc.h"

#include "communicator.h"
#include "logger.h"
#include "message_builder.h"
//#include "bank_worker.h"
#include "lamport_ipc.h"


extern struct communicator *communicator;
extern struct logger *logger;

extern int N;      ///< Общее число процессов
extern int X;      ///< Число дочерних процессов


extern local_id process_id;

extern timestamp_t timestamp;


extern timestamp_t *queue;


void set_local_id(local_id local_id);

void set_process_communicator();

void set_process_logger();

void pre_run();

void post_run();


void send_multicast_wrapper(int16_t type, Message *message);

void receive_multicast_wrapper(int16_t type, Message *message);

#endif //PA1_PROCESS_H
