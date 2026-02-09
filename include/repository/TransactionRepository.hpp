#pragma once

#include <string>
#include <vector>
#include "data/Transaction.hpp"

class TransactionRepository {
public:
    TransactionRepository();

    int add(Transaction transaction);
    Transaction *findById(int id);
    const std::vector<Transaction> &getAll() const;

    bool loadFromFile(const std::string &filename, std::string &errorMessage);
    bool saveToFile(const std::string &filename, std::string &errorMessage) const;

private:
    int nextId_{1};
    std::vector<Transaction> transactions_;
};
