#include "currency_manager.hpp"
#include <iostream>

double CurrencyManager::convert_via_local(int from, int to, double amount_from,
                                          double *rate_from_loc, double *rate_to_loc,
                                          double *profit_delta_loc) const {
    double loc_in = amount_from * currencies[from].buy_to_loc;
    double amount_to = loc_in / currencies[to].sell_to_loc;

    if (rate_from_loc) *rate_from_loc = currencies[from].buy_to_loc;
    if (rate_to_loc)   *rate_to_loc   = currencies[to].sell_to_loc;

    double cost_loc = amount_to * currencies[to].buy_to_loc;
    double profit_delta = loc_in - cost_loc;
    if (profit_delta_loc) *profit_delta_loc = profit_delta;
    return amount_to;
}

void CurrencyManager::check_criticals() const {
    for (int i = 0; i < MAX_CUR; ++i) {
        if (currencies[i].bal < currencies[i].critical_min) {
            std::cout << "[-] ALERT: " << currencies[i].name << " reserve below critical minimum (" << currencies[i].bal << " < " << currencies[i].critical_min << ")\n";
        }
    }
}
