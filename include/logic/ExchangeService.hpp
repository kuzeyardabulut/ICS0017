#pragma once

#include <string>
#include <vector>
#include "data/Transaction.hpp"
#include "repository/CurrencyRepository.hpp"
#include "repository/TransactionRepository.hpp"
#include "repository/ReceiptRepository.hpp"

struct ExchangeRequest {
    std::string fromCode;
    std::string toCode;
    double amountFrom{0.0};
    bool partial{false};
    double partialToAmount{0.0};
};

struct ExchangeResult {
    bool success{false};
    std::string message;
    Transaction transaction;
    double remainderLoc{0.0};
    std::vector<std::string> warnings;
};

struct OperationResult {
    bool success{false};
    std::string message;
    std::vector<std::string> warnings;
};

struct ReportSummary {
    int dayTxCount{0};
    double dayProfit{0.0};
    int monthTxCount{0};
    double monthProfit{0.0};
    double cashierBonus{0.0};
    double monthNetProfit{0.0};
};

class ExchangeService {
public:
    ExchangeService(CurrencyRepository &currencyRepo,
                    TransactionRepository &transactionRepo,
                    ReceiptRepository &receiptRepo);

    ExchangeResult executeExchange(const ExchangeRequest &request);

    OperationResult setRates(const std::string &code, double buyToLoc, double sellToLoc);
    OperationResult adjustReserve(const std::string &code, double delta);
    OperationResult setCriticalMin(const std::string &code, double criticalMin);

    std::vector<Transaction> listTransactions() const;
    Transaction *findTransactionById(int id);
    std::vector<Currency> listCurrencies() const;

    ReportSummary generateDailySummary(const std::string &date) const;
    ReportSummary generateMonthlySummary(const std::string &yearMonth) const;
    std::vector<std::string> checkCriticals() const;

    bool loadTransactions(const std::string &filename, std::string &errorMessage);
    bool saveTransactions(const std::string &filename, std::string &errorMessage) const;

    static std::string currentDate();

private:
    CurrencyRepository &currencyRepo_;
    TransactionRepository &transactionRepo_;
    ReceiptRepository &receiptRepo_;
    static std::string currentTime();
};
