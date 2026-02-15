#pragma once

#include <string>

namespace AppConfig {
    // Default filenames; can be modified by tests or main if needed
    inline std::string transactionsFile = "transactions.csv";
    inline std::string receiptsFile = "receipts.txt";
}
