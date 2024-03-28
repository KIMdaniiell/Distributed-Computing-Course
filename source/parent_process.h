#ifndef PA1_PARENT_PROCESS_H
#define PA1_PARENT_PROCESS_H


#include "process.h"

#include <sys/wait.h>


void parent_run_full(pid_t children_PIDs[]);

#endif //PA1_PARENT_PROCESS_H
