/**
 * @file     parent.c
 * @Author   Kim Daniel (kimdaniiell@gmail.com)
 * @date     October, 2023
 *
 *//*


#include <stdlib.h>     // exit()

//#include "parent.h"


enum states {
    INITIAL_STATE = 0,
    BREEDING_STATE,
    MONITORING_STATE,
    WAITING_STATE,
    FINAL_STATE
};

typedef void (*transition_callback)();

struct transition {
    enum states new_state;
    transition_callback worker;
};


void initial_callback(){};
void breeding_callback(){};
void monitoring_callback(){};
void waiting_callback(){};
void final_callback(){};


struct transition FSM_table[5]= {
        [INITIAL_STATE]                 = {BREEDING_STATE,      breeding_callback},
        [BREEDING_STATE]                = {MONITORING_STATE,    monitoring_callback},
        [MONITORING_STATE]              = {WAITING_STATE,       waiting_callback},
        [WAITING_STATE]                 = {FINAL_STATE,         final_callback},
//        [FINAL_STATE]                   = {INITIAL_STATE,       initial_callback}
};


int run() {
    enum states state = INITIAL_STATE;
    enum states next_state;
    transition_callback worker;


    while(1) {
        next_state = FSM_table[state].new_state;
        worker = FSM_table[state].worker;

        if (NULL != worker) {
            worker();
        }

        // TODO добавить условие перехода
        state = next_state;
        break;
    }

    exit(1); // Корректный выход предполагается внутри цикла конечного автомата
}
*/
