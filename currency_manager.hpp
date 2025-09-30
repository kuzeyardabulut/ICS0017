#pragma once

#include "utils.hpp"

class CurrencyManager {
public:
    CurrencyManager() = default;

    // Convert amount 'from' -> 'to' via LOC; returns amount in 'to' currency.
    double convert_via_local(int from, int to, double amount_from,
                             double *rate_from_loc, double *rate_to_loc,
                             double *profit_delta_loc) const;

    // Check currency reserves against critical minimums and print alerts.
    void check_criticals() const;
};
