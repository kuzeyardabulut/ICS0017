#pragma once

#include <string>

struct Transaction {
    int id{0};
    std::string date;
    std::string time;
    std::string fromCode;
    std::string toCode;
    double amountFrom{0.0};
    double amountTo{0.0};
    double rateFromLoc{0.0};
    double rateToLoc{0.0};
    bool partial{false};
    double remainderLoc{0.0};
    double profitLoc{0.0};

    std::string toString() const;
};
