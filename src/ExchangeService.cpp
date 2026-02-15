#include "logic/ExchangeService.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <numeric>
#include <sstream>
#include "AppConfig.hpp"

ExchangeService::ExchangeService(CurrencyRepository &currencyRepo,
                                 TransactionRepository &transactionRepo,
                                 ReceiptRepository &receiptRepo)
    : currencyRepo_(currencyRepo),
      transactionRepo_(transactionRepo),
      receiptRepo_(receiptRepo) {}

ExchangeResult ExchangeService::executeExchange(const ExchangeRequest &request) {
    ExchangeResult result;

    if (request.amountFrom <= 0.0) {
        result.message = "Amount must be positive.";
        return result;
    }
    if (request.fromCode == request.toCode) {
        result.message = "From and To currencies must be different.";
        return result;
    }

    Currency *from = currencyRepo_.findByCode(request.fromCode);
    Currency *to = currencyRepo_.findByCode(request.toCode);
    if (!from || !to) {
        result.message = "Unknown currency code.";
        return result;
    }

    double locIn = request.amountFrom * from->buyToLoc;
    double amountTo = locIn / to->sellToLoc;
    double costLoc = amountTo * to->buyToLoc;
    double profitLoc = locIn - costLoc;

    if (to->balance < amountTo) {
        result.message = "Insufficient reserve in target currency.";
        return result;
    }

    double remainderLoc = 0.0;
    if (request.partial) {
        if (request.partialToAmount < 0.0 || request.partialToAmount > amountTo) {
            result.message = "Partial amount exceeds exchangeable value.";
            return result;
        }
        double locValueGiven = request.partialToAmount * to->sellToLoc;
        remainderLoc = locIn - locValueGiven;
        amountTo = request.partialToAmount;
        if (remainderLoc < 0.0) {
            result.message = "Partial amount exceeds exchangeable value.";
            return result;
        }
        Currency *locCurrency = currencyRepo_.findByCode("LOC");
        if (!locCurrency || locCurrency->balance < remainderLoc) {
            result.message = "Insufficient LOC reserve for partial payout remainder.";
            return result;
        }
        locCurrency->balance -= remainderLoc;
    }

    from->balance += request.amountFrom;
    to->balance -= amountTo;

    Transaction transaction;
    transaction.date = currentDate();
    transaction.time = currentTime();
    transaction.fromCode = from->code;
    transaction.toCode = to->code;
    transaction.amountFrom = request.amountFrom;
    transaction.amountTo = amountTo;
    transaction.rateFromLoc = from->buyToLoc;
    transaction.rateToLoc = to->sellToLoc;
    transaction.partial = request.partial;
    transaction.remainderLoc = remainderLoc;
    transaction.profitLoc = profitLoc;

    transaction.id = transactionRepo_.add(transaction);
    std::string txErr;
    if (!transactionRepo_.saveToFile(AppConfig::transactionsFile, txErr)) {
        transactionRepo_.removeById(transaction.id);
        from->balance -= request.amountFrom;
        to->balance += amountTo;
        if (request.partial) {
            Currency *locCurrency = currencyRepo_.findByCode("LOC");
            if (locCurrency) locCurrency->balance += remainderLoc;
        }
        result.message = "Failed to persist transaction: " + txErr;
        return result;
    }

    Receipt receipt;
    receipt.transactionId = transaction.id;
    receipt.date = transaction.date;
    receipt.time = transaction.time;
    std::ostringstream text;
    text.setf(std::ios::fixed);
    text.precision(6);
    text << "From: " << transaction.amountFrom << " " << transaction.fromCode << "\n";
    text << "To:   " << transaction.amountTo << " " << transaction.toCode << "\n";
    text << "Rate (BUY->LOC):  " << transaction.rateFromLoc << "\n";
    text << "Rate (SELL->LOC): " << transaction.rateToLoc << "\n";
    if (transaction.partial) {
        text << "Remainder in LOC: " << transaction.remainderLoc << "\n";
    }
    receipt.text = text.str();

    int receiptId = receiptRepo_.add(receipt);
    std::string receiptError;
    if (!receiptRepo_.appendToFile(AppConfig::receiptsFile, receipt, receiptError)) {
        transactionRepo_.removeById(transaction.id);
        std::string saveErr;
        transactionRepo_.saveToFile(AppConfig::transactionsFile, saveErr);
        receiptRepo_.removeById(receiptId);
        from->balance -= request.amountFrom;
        to->balance += amountTo;
        if (request.partial) {
            Currency *locCurrency = currencyRepo_.findByCode("LOC");
            if (locCurrency) locCurrency->balance += remainderLoc;
        }
        result.message = "Failed to persist receipt: " + receiptError;
        return result;
    }

    result.success = true;
    result.message = "Exchange completed successfully.";
    result.transaction = transaction;
    result.remainderLoc = remainderLoc;

    auto warnings = checkCriticals();
    result.warnings = warnings;
    return result;
}

OperationResult ExchangeService::setRates(const std::string &code, double buyToLoc, double sellToLoc) {
    OperationResult result;
    Currency *currency = currencyRepo_.findByCode(code);
    if (!currency) {
        result.message = "Currency not found.";
        return result;
    }
    if (buyToLoc <= 0.0 || sellToLoc <= 0.0 || sellToLoc < buyToLoc) {
        result.message = "Invalid rates. SELL must be >= BUY and both positive.";
        return result;
    }
    currency->buyToLoc = buyToLoc;
    currency->sellToLoc = sellToLoc;
    result.success = true;
    result.message = "Rates updated.";
    return result;
}

OperationResult ExchangeService::adjustReserve(const std::string &code, double delta) {
    OperationResult result;
    Currency *currency = currencyRepo_.findByCode(code);
    if (!currency) {
        result.message = "Currency not found.";
        return result;
    }
    if (currency->balance + delta < 0.0) {
        result.message = "Operation would make reserve negative.";
        return result;
    }
    currency->balance += delta;
    result.success = true;
    result.message = "Reserve updated.";
    return result;
}

OperationResult ExchangeService::setCriticalMin(const std::string &code, double criticalMin) {
    OperationResult result;
    Currency *currency = currencyRepo_.findByCode(code);
    if (!currency) {
        result.message = "Currency not found.";
        return result;
    }
    if (criticalMin < 0.0) {
        result.message = "Critical minimum cannot be negative.";
        return result;
    }
    currency->criticalMin = criticalMin;
    result.success = true;
    result.message = "Critical minimum updated.";
    return result;
}

std::vector<Transaction> ExchangeService::listTransactions() const {
    return transactionRepo_.getAll();
}

Transaction *ExchangeService::findTransactionById(int id) {
    return transactionRepo_.findById(id);
}

std::vector<Currency> ExchangeService::listCurrencies() const {
    const auto &all = currencyRepo_.getAll();
    return std::vector<Currency>(all.begin(), all.end());
}

ReportSummary ExchangeService::generateDailySummary(const std::string &date) const {
    ReportSummary summary;
    const auto &all = transactionRepo_.getAll();

    std::vector<Transaction> dayTx;
    std::copy_if(all.begin(), all.end(), std::back_inserter(dayTx),
                 [&date](const Transaction &t) { return t.date == date; });

    summary.dayTxCount = static_cast<int>(dayTx.size());
    summary.dayProfit = std::accumulate(dayTx.begin(), dayTx.end(), 0.0,
                                        [](double sum, const Transaction &t) { return sum + t.profitLoc; });

    if (date.size() >= 7) {
        std::string monthPrefix = date.substr(0, 7);
        std::vector<Transaction> monthTx;
        std::copy_if(all.begin(), all.end(), std::back_inserter(monthTx),
                     [&monthPrefix](const Transaction &t) { return t.date.compare(0, 7, monthPrefix) == 0; });

        summary.monthTxCount = static_cast<int>(monthTx.size());
        summary.monthProfit = std::accumulate(monthTx.begin(), monthTx.end(), 0.0,
                                              [](double sum, const Transaction &t) { return sum + t.profitLoc; });
        summary.cashierBonus = summary.monthProfit * 0.05;
        summary.monthNetProfit = summary.monthProfit - summary.cashierBonus;
    }

    return summary;
}

ReportSummary ExchangeService::generateMonthlySummary(const std::string &yearMonth) const {
    ReportSummary summary;
    const auto &all = transactionRepo_.getAll();

    if (yearMonth.size() != 7) {
        return summary;
    }

    std::vector<Transaction> monthTx;
    std::copy_if(all.begin(), all.end(), std::back_inserter(monthTx),
                 [&yearMonth](const Transaction &t) { return t.date.compare(0, 7, yearMonth) == 0; });

    summary.monthTxCount = static_cast<int>(monthTx.size());
    summary.monthProfit = std::accumulate(monthTx.begin(), monthTx.end(), 0.0,
                                          [](double sum, const Transaction &t) { return sum + t.profitLoc; });
    summary.cashierBonus = summary.monthProfit * 0.05;
    summary.monthNetProfit = summary.monthProfit - summary.cashierBonus;
    return summary;
}

std::vector<std::string> ExchangeService::checkCriticals() const {
    std::vector<std::string> warnings;
    for (const auto &currency : currencyRepo_.getAll()) {
        if (currency.balance < currency.criticalMin) {
            warnings.push_back("ALERT: " + currency.code + " reserve below critical minimum");
        }
    }
    return warnings;
}

bool ExchangeService::loadTransactions(const std::string &filename, std::string &errorMessage) {
    return transactionRepo_.loadFromFile(filename, errorMessage);
}

bool ExchangeService::saveTransactions(const std::string &filename, std::string &errorMessage) const {
    return transactionRepo_.saveToFile(filename, errorMessage);
}

std::string ExchangeService::currentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%d");
    return out.str();
}

std::string ExchangeService::currentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream out;
    out << std::put_time(&tm, "%H:%M:%S");
    return out.str();
}
