#pragma once

#include "utils.hpp"

class CurrencyManager {
public:
    CurrencyManager() = default;

    double convert_via_local(int from, int to, double amount_from,
                             double *rate_from_loc, double *rate_to_loc,
                             double *profit_delta_loc) const;

    void check_criticals() const;
};
