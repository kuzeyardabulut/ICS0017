#include "ui/ConsoleUI.hpp"
#include <iostream>
#include <limits>

ConsoleUI::ConsoleUI(ExchangeService &service) : service_(service) {}

int ConsoleUI::readInt(const std::string &prompt, int min, int max) const {
    while (true) {
        std::cout << prompt << " ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "Input error. Try again.\n";
            continue;
        }
        try {
            size_t pos = 0;
            int value = std::stoi(line, &pos);
            if (pos != line.size()) throw std::invalid_argument("extra");
            if (value < min || value > max) {
                std::cout << "Value must be between " << min << " and " << max << ".\n";
                continue;
            }
            return value;
        } catch (...) {
            std::cout << "Invalid number. Try again.\n";
        }
    }
}

double ConsoleUI::readDouble(const std::string &prompt, double min, double max) const {
    while (true) {
        std::cout << prompt << " ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "Input error. Try again.\n";
            continue;
        }
        try {
            size_t pos = 0;
            double value = std::stod(line, &pos);
            if (pos != line.size()) throw std::invalid_argument("extra");
            if (value < min || value > max) {
                std::cout << "Value must be between " << min << " and " << max << ".\n";
                continue;
            }
            return value;
        } catch (...) {
            std::cout << "Invalid number. Try again.\n";
        }
    }
}

std::string ConsoleUI::readString(const std::string &prompt) const {
    std::cout << prompt << " ";
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void ConsoleUI::showMenu() const {
    std::cout << "\n==============================\n";
    std::cout << " Currency Exchange - Menu\n";
    std::cout << "==============================\n";
    std::cout << " 1) Execute exchange\n";
    std::cout << " 2) List transactions\n";
    std::cout << " 3) Search transaction by ID\n";
    std::cout << " 4) Show balances\n";
    std::cout << " 5) Set rates\n";
    std::cout << " 6) Adjust reserves\n";
    std::cout << " 7) Set critical minimums\n";
    std::cout << " 8) End-of-day report\n";
    std::cout << " 9) Monthly report\n";
    std::cout << " 0) Exit\n";
}

void ConsoleUI::run() {
    while (true) {
        showMenu();
        int choice = readInt("Choose option:", 0, 9);
        switch (choice) {
            case 0: return;
            case 1: handleExchange(); break;
            case 2: handleListTransactions(); break;
            case 3: handleSearchTransaction(); break;
            case 4: handleShowBalances(); break;
            case 5: handleSetRates(); break;
            case 6: handleAdjustReserves(); break;
            case 7: handleSetCriticalMins(); break;
            case 8: handleDailyReport(); break;
            case 9: handleMonthlyReport(); break;
            default: std::cout << "Unknown option.\n"; break;
        }
    }
}

void ConsoleUI::handleExchange() {
    auto currencies = service_.listCurrencies();
    if (currencies.empty()) {
        std::cout << "No currencies available.\n";
        return;
    }

    std::cout << "Available currencies:\n";
    for (const auto &c : currencies) {
        std::cout << " - " << c.code << "\n";
    }

    ExchangeRequest request;
    request.fromCode = readString("From currency code:");
    request.toCode = readString("To currency code:");
    request.amountFrom = readDouble("Amount to exchange:", 0.01, 1e12);
    int partialFlag = readInt("Partial exchange? 1=Yes, 0=No:", 0, 1);
    request.partial = (partialFlag == 1);
    if (request.partial) {
        request.partialToAmount = readDouble("Target currency amount to receive now:", 0.0, 1e12);
    }

    ExchangeResult result = service_.executeExchange(request);
    if (!result.success) {
        std::cout << "[-] " << result.message << "\n";
        return;
    }

    std::cout << "[+] " << result.message << "\n";
    std::cout << "Transaction ID: " << result.transaction.id << "\n";
    std::cout.setf(std::ios::fixed); std::cout.precision(6);
    std::cout << "From: " << result.transaction.amountFrom << " " << result.transaction.fromCode << "\n";
    std::cout << "To:   " << result.transaction.amountTo << " " << result.transaction.toCode << "\n";
    if (result.transaction.partial) {
        std::cout << "Remainder in LOC: " << result.remainderLoc << "\n";
    }
    std::cout.unsetf(std::ios::floatfield);

    if (!result.warnings.empty()) {
        std::cout << "Warnings:\n";
        for (const auto &w : result.warnings) {
            std::cout << "  - " << w << "\n";
        }
    }
}

void ConsoleUI::handleListTransactions() {
    auto transactions = service_.listTransactions();
    if (transactions.empty()) {
        std::cout << "No transactions found.\n";
        return;
    }
    for (const auto &t : transactions) {
        std::cout << t.toString() << "\n";
    }
}

void ConsoleUI::handleSearchTransaction() {
    int id = readInt("Transaction ID:", 1, 1000000000);
    Transaction *t = service_.findTransactionById(id);
    if (!t) {
        std::cout << "Transaction not found.\n";
        return;
    }
    std::cout << t->toString() << "\n";
}

void ConsoleUI::handleShowBalances() {
    auto currencies = service_.listCurrencies();
    if (currencies.empty()) {
        std::cout << "No currencies available.\n";
        return;
    }
    std::cout.setf(std::ios::fixed); std::cout.precision(6);
    for (const auto &c : currencies) {
        std::cout << c.code << ": " << c.balance << "\n";
    }
    std::cout.unsetf(std::ios::floatfield);
}

void ConsoleUI::handleSetRates() {
    std::string code = readString("Currency code:");
    double buy = readDouble("BUY->LOC:", 0.000001, 1e12);
    double sell = readDouble("SELL->LOC (>= BUY):", buy, 1e12);
    OperationResult result = service_.setRates(code, buy, sell);
    if (!result.success) {
        std::cout << "[-] " << result.message << "\n";
        return;
    }
    std::cout << "[+] " << result.message << "\n";
}

void ConsoleUI::handleAdjustReserves() {
    std::string code = readString("Currency code:");
    double delta = readDouble("Positive to add, negative to remove:", -1e12, 1e12);
    OperationResult result = service_.adjustReserve(code, delta);
    if (!result.success) {
        std::cout << "[-] " << result.message << "\n";
        return;
    }
    std::cout << "[+] " << result.message << "\n";
}

void ConsoleUI::handleSetCriticalMins() {
    std::string code = readString("Currency code:");
    double criticalMin = readDouble("Critical minimum:", 0.0, 1e12);
    OperationResult result = service_.setCriticalMin(code, criticalMin);
    if (!result.success) {
        std::cout << "[-] " << result.message << "\n";
        return;
    }
    std::cout << "[+] " << result.message << "\n";
}

void ConsoleUI::handleDailyReport() {
    std::string date = readString("Enter date (YYYY-MM-DD), empty for today:");
    if (date.empty()) {
        date = ExchangeService::currentDate();
    }
    ReportSummary summary = service_.generateDailySummary(date);
    std::cout << "\n=== End-of-day report for " << date << " ===\n";
    std::cout << "Total Transactions: " << summary.dayTxCount << "\n";
    std::cout << "Total Profit (LOC): " << summary.dayProfit << "\n";
    std::cout << "Month-to-date Transactions: " << summary.monthTxCount << "\n";
    std::cout << "Month-to-date Profit (LOC): " << summary.monthProfit << "\n";
    std::cout << "Cashier monthly bonus (5%): " << summary.cashierBonus << "\n";
    std::cout << "Month net profit: " << summary.monthNetProfit << "\n";
}

static bool isValidYearMonth(const std::string &value) {
    if (value.size() != 7 || value[4] != '-') return false;
    for (size_t i = 0; i < value.size(); ++i) {
        if (i == 4) continue;
        if (value[i] < '0' || value[i] > '9') return false;
    }
    int month = std::stoi(value.substr(5, 2));
    return month >= 1 && month <= 12;
}

void ConsoleUI::handleMonthlyReport() {
    std::string yearMonth = readString("Enter month (YYYY-MM):");
    if (!isValidYearMonth(yearMonth)) {
        std::cout << "Invalid month format. Use YYYY-MM.\n";
        return;
    }

    ReportSummary summary = service_.generateMonthlySummary(yearMonth);
    std::cout << "\n=== Monthly report for " << yearMonth << " ===\n";
    std::cout << "Month-to-date Transactions: " << summary.monthTxCount << "\n";
    std::cout << "Month-to-date Profit (LOC): " << summary.monthProfit << "\n";
    std::cout << "Cashier monthly bonus (5%): " << summary.cashierBonus << "\n";
    std::cout << "Month net profit: " << summary.monthNetProfit << "\n";
}
