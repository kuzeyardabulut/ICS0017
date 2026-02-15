#include "logic/ExchangeService.hpp"
#include "ui/ConsoleUI.hpp"
#include "repository/CurrencyRepository.hpp"
#include "repository/TransactionRepository.hpp"
#include "repository/ReceiptRepository.hpp"
#include "data/Currency.hpp"
#include "AppConfig.hpp"

static void seedCurrencies(CurrencyRepository &repo) {
    repo.add(Currency(0, "LOC", {200, 100, 50, 20, 10, 5, 2, 1}, 50000.0, 10000.0, 1.0, 1.0));
    repo.add(Currency(1, "USD", {100, 50, 20, 10, 5, 2, 1}, 10000.0, 2000.0, 41.36, 41.45));
    repo.add(Currency(2, "EUR", {500, 200, 100, 50, 20, 10, 5}, 8000.0, 1500.0, 48.38, 48.60));
    repo.add(Currency(3, "GBP", {50, 20, 10, 5, 2, 1}, 3000.0, 500.0, 55.91, 56.26));
    repo.add(Currency(4, "JPY", {10000, 5000, 2000, 1000, 500, 100, 50}, 1000000.0, 200000.0, 0.27, 0.28));
}

int main() {
    CurrencyRepository currencyRepo;
    TransactionRepository transactionRepo;
    ReceiptRepository receiptRepo;

    seedCurrencies(currencyRepo);

    ExchangeService service(currencyRepo, transactionRepo, receiptRepo);

    std::string loadError;
    service.loadTransactions(AppConfig::transactionsFile, loadError);

    ConsoleUI ui(service);
    ui.run();

    std::string saveError;
    service.saveTransactions(AppConfig::transactionsFile, saveError);

    return 0;
}
