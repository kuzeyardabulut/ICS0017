#pragma once

#include <string>

namespace AppConfig {
    // files are stored under the data/ subdirectory to keep repository root clean
    // caller must ensure the directory exists (e.g. create before first run)
    inline std::string transactionsFile = "data/transactions.csv";
    inline std::string receiptsFile = "data/receipts.txt";
    inline std::string currenciesFile = "data/currencies.csv";
}
