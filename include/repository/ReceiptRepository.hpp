#pragma once

#include <string>
#include <vector>
#include "data/Receipt.hpp"

class ReceiptRepository {
public:
    int add(Receipt receipt);
    Receipt *findById(int id);
    const std::vector<Receipt> &getAll() const;

    bool appendToFile(const std::string &filename, const Receipt &receipt, std::string &errorMessage) const;

private:
    int nextId_{1};
    std::vector<Receipt> receipts_;
};
