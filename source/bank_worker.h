#ifndef PA1_BANK_WORKER_H
#define PA1_BANK_WORKER_H

#include <string.h>
#include <stdio.h>

#include "banking.h"

/**====---- AllHistory working ----====**/

void append_AllHistory(AllHistory *allHistory, BalanceHistory *balanceHistory);

void complete_AllHistory(AllHistory *allHistory);

/**====---- BalanceHistory working ----====**/

void append_BalanceHistory_wrapper(BalanceHistory *balanceHistory, BalanceState balanceState);

void append_BalanceHistory(BalanceHistory *balanceHistory, balance_t balance, timestamp_t timestamp);

/*void optimise_BalanceHistory(BalanceHistory *balanceHistory);*/

void complete_BalanceHistory(BalanceHistory *balanceHistory, timestamp_t end_timestamp);

void print_BalanceHistory(BalanceHistory *balanceHistory);

#endif //PA1_BANK_WORKER_H
