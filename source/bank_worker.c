#include "bank_worker.h"

/*void init_AllHistory(AllHistory *allHistory, int X) {

}*/

void append_AllHistory(AllHistory *allHistory, BalanceHistory *balanceHistory) {
    memcpy(allHistory->s_history + allHistory->s_history_len,
           balanceHistory,
           sizeof(BalanceHistory));
    allHistory->s_history_len++;
}

timestamp_t get_end_timestamp(AllHistory *allHistory) {
    timestamp_t result = 0;

    for (int i = 0; i < allHistory->s_history_len; i++) {
        BalanceHistory balanceHistory = allHistory->s_history[i];
//        if (balanceHistory.s_history_len > result)
//            result = balanceHistory.s_history_len;
        BalanceState balanceState = balanceHistory.s_history[balanceHistory.s_history_len-1];
        if (balanceState.s_time > result)
            result = balanceState.s_time;
    }

    return result;
}

void complete_AllHistory(AllHistory *allHistory) {
    timestamp_t end_timestamp = get_end_timestamp(allHistory);

    printf("[bank_worker:complete_AllHistory] end_timestamp = %d\n", end_timestamp);

    for (int i = 0; i < allHistory->s_history_len; i++) {
        complete_BalanceHistory((BalanceHistory *) allHistory->s_history + i ,end_timestamp);
    }
}



/*void init_BalanceHistory(BalanceHistory *balanceHistory, local_id id) {

}*/

void append_BalanceHistory_wrapper(BalanceHistory *balanceHistory, BalanceState balanceState) {
    balanceHistory->s_history[balanceHistory->s_history_len] = balanceState;
    balanceHistory->s_history_len++;
}

void append_BalanceHistory(BalanceHistory *balanceHistory, balance_t balance, timestamp_t timestamp) {
    BalanceState balanceState = {
            .s_balance = balance,
            .s_time = timestamp,
            .s_balance_pending_in = 0
    };

    append_BalanceHistory_wrapper(balanceHistory, balanceState);
}

void optimise_BalanceHistory(BalanceHistory *balanceHistory) {
    uint8_t new_len = 0;
    BalanceState new_history[MAX_T + 1];

    for (int i = 0; i < balanceHistory->s_history_len; i++) {
        BalanceState balanceState = balanceHistory->s_history[i];

        if (new_len == 0) {
            new_history[new_len++] = balanceState;
        } else {
            BalanceState last_new_BalanceState = new_history[new_len - 1];
            if (balanceState.s_time != last_new_BalanceState.s_time) {
                new_history[new_len++] = balanceState;
            }
        }
    }

    balanceHistory->s_history_len = new_len;
    memcpy(balanceHistory->s_history, new_history, sizeof(new_history));
}

void complete_BalanceHistory(BalanceHistory *balanceHistory, timestamp_t end_timestamp) {
    uint8_t new_len = 0;
    BalanceState new_history[MAX_T + 1];

    BalanceState new_balanceState = {
        .s_time = -1,
        .s_balance = 0,
        .s_balance_pending_in = 0
    };

    for (int i = 0; i < balanceHistory->s_history_len; i++) {
        BalanceState balanceState = balanceHistory->s_history[i];

        while (balanceState.s_time - new_balanceState.s_time > 1) {
            new_balanceState.s_time++;
            new_history[new_len] = new_balanceState;
            new_len++;
        }

        new_history[new_len] = balanceState;
        new_len++;
        new_balanceState = balanceState;
    }

    while (new_balanceState.s_time < end_timestamp) {
        new_balanceState.s_time++;
        new_history[new_len] = new_balanceState;
        new_len++;
    }

    balanceHistory->s_history_len = new_len;
    memcpy(balanceHistory->s_history, new_history, sizeof(new_history));
}

void print_BalanceHistory(BalanceHistory *balanceHistory) {
    for (int i = 0; i < balanceHistory->s_history_len; i++) {
        BalanceState balanceState = balanceHistory->s_history[i];
        printf("[bank_worker:print_BalanceHistory] %d  TIME=%d , BALANCE=%d\n",
               balanceHistory->s_id,
               balanceState.s_time, balanceState.s_balance);
    }
}
