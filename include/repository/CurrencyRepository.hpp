#pragma once

#include <string>
#include <vector>
#include "data/Currency.hpp"

class CurrencyRepository {
public:
    void add(const Currency &currency);
    Currency *findById(int id);
    Currency *findByCode(const std::string &code);
    const std::vector<Currency> &getAll() const;

private:
    std::vector<Currency> currencies_;
};
