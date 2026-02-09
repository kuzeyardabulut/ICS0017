#pragma once

#include <string>
#include <vector>

struct Currency {
    int id{0};
    std::string code;
    std::vector<int> denominations;
    double startBalance{0.0};
    double balance{0.0};
    double criticalMin{0.0};
    double buyToLoc{0.0};
    double sellToLoc{0.0};

    Currency() = default;
    Currency(int idValue,
             std::string codeValue,
             std::vector<int> denominationsValue,
             double startBalanceValue,
             double criticalMinValue,
             double buyToLocValue,
             double sellToLocValue)
        : id(idValue),
          code(std::move(codeValue)),
          denominations(std::move(denominationsValue)),
          startBalance(startBalanceValue),
          balance(startBalanceValue),
          criticalMin(criticalMinValue),
          buyToLoc(buyToLocValue),
          sellToLoc(sellToLocValue) {}

    std::string toString() const;
};
