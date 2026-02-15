#include "repository/TransactionRepository.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cerrno>

TransactionRepository::TransactionRepository() = default;

int TransactionRepository::add(Transaction transaction) {
    transaction.id = nextId_++;
    transactions_.push_back(transaction);
    return transaction.id;
}

Transaction *TransactionRepository::findById(int id) {
    auto it = std::find_if(transactions_.begin(), transactions_.end(),
                           [id](const Transaction &t) { return t.id == id; });
    if (it == transactions_.end()) return nullptr;
    return &(*it);
}

const std::vector<Transaction> &TransactionRepository::getAll() const {
    return transactions_;
}

bool TransactionRepository::loadFromFile(const std::string &filename, std::string &errorMessage) {
    transactions_.clear();
    nextId_ = 1;

    std::ifstream in(filename);
    if (!in) {
        errorMessage = "File not found: " + filename;
        return false;
    }

    std::string line;
    if (!std::getline(in, line)) {
        errorMessage = "Empty file: " + filename;
        return false;
    }

    int maxId = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::vector<std::string> parts;
        std::string token;
        std::istringstream ss(line);
        while (std::getline(ss, token, ',')) {
            parts.push_back(token);
        }
        if (parts.size() < 12) {
            continue;
        }

        Transaction t;
        try {
            t.id = std::stoi(parts[0]);
            t.date = parts[1];
            t.time = parts[2];
            t.fromCode = parts[3];
            t.toCode = parts[4];
            t.amountFrom = std::stod(parts[5]);
            t.amountTo = std::stod(parts[6]);
            t.rateFromLoc = std::stod(parts[7]);
            t.rateToLoc = std::stod(parts[8]);
            t.partial = (std::stoi(parts[9]) != 0);
            t.remainderLoc = std::stod(parts[10]);
            t.profitLoc = std::stod(parts[11]);
        } catch (...) {
            continue;
        }

        transactions_.push_back(t);
        if (t.id > maxId) maxId = t.id;
    }

    nextId_ = maxId + 1;
    return true;
}

bool TransactionRepository::saveToFile(const std::string &filename, std::string &errorMessage) const {
    // Atomic save: write to temporary file and rename into place
    std::string tmp = filename + ".tmp";
    std::ofstream out(tmp, std::ios::trunc);
    if (!out) {
        errorMessage = "Failed to open temp file for writing: " + tmp;
        return false;
    }

    out << "id,date,time,from,to,amount_from,amount_to,rate_from_loc,rate_to_loc,partial,remainder_loc,profit_loc\n";
    for (const auto &t : transactions_) {
        out << t.id << ',' << t.date << ',' << t.time << ',' << t.fromCode << ',' << t.toCode << ','
            << t.amountFrom << ',' << t.amountTo << ',' << t.rateFromLoc << ',' << t.rateToLoc << ','
            << (t.partial ? 1 : 0) << ',' << t.remainderLoc << ',' << t.profitLoc << '\n';
    }

    out.flush();
    out.close();
    if (!out) {
        // writing/flush error
        std::remove(tmp.c_str());
        errorMessage = "Failed to write temp file: " + tmp;
        return false;
    }

    if (std::rename(tmp.c_str(), filename.c_str()) != 0) {
        int err = errno;
        std::remove(tmp.c_str());
        errorMessage = "Failed to rename temp file to target: " + filename + " (errno=" + std::to_string(err) + ")";
        return false;
    }

    return true;
}

bool TransactionRepository::removeById(int id) {
    auto it = std::find_if(transactions_.begin(), transactions_.end(),
                           [id](const Transaction &t) { return t.id == id; });
    if (it == transactions_.end()) return false;
    transactions_.erase(it);
    return true;
}
