#ifndef PA1_BANK_WORKER_H
#define PA1_BANK_WORKER_H

#include <string.h>
#include <stdio.h>

#include "banking.h"


//void init_AllHistory(AllHistory *allHistory, int X);

void append_AllHistory(AllHistory *allHistory, BalanceHistory *balanceHistory);

//timestamp_t get_end_timestamp(AllHistory *allHistory);

void complete_AllHistory(AllHistory *allHistory);



//void init_BalanceHistory(BalanceHistory *balanceHistory, local_id id);

void append_BalanceHistory_wrapper(BalanceHistory *balanceHistory, BalanceState balanceState);

void append_BalanceHistory(BalanceHistory *balanceHistory, balance_t balance, timestamp_t timestamp);

void optimise_BalanceHistory(BalanceHistory *balanceHistory);

void complete_BalanceHistory(BalanceHistory *balanceHistory, timestamp_t end_timestamp);

void print_BalanceHistory(BalanceHistory *balanceHistory);



#endif //PA1_BANK_WORKER_H
