#include "repository/ReceiptRepository.hpp"
#include <algorithm>
#include <fstream>

int ReceiptRepository::add(Receipt receipt) {
    receipt.id = nextId_++;
    receipts_.push_back(receipt);
    return receipt.id;
}

Receipt *ReceiptRepository::findById(int id) {
    auto it = std::find_if(receipts_.begin(), receipts_.end(),
                           [id](const Receipt &r) { return r.id == id; });
    if (it == receipts_.end()) return nullptr;
    return &(*it);
}

const std::vector<Receipt> &ReceiptRepository::getAll() const {
    return receipts_;
}

bool ReceiptRepository::appendToFile(const std::string &filename, const Receipt &receipt, std::string &errorMessage) const {
    std::ofstream out(filename, std::ios::app);
    if (!out) {
        errorMessage = "Failed to open file for writing: " + filename;
        return false;
    }

    out << receipt.toString() << "\n";
    return true;
}
