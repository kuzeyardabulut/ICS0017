#pragma once

#include "logic/ExchangeService.hpp"

class ConsoleUI {
public:
    explicit ConsoleUI(ExchangeService &service);
    void run();

private:
    ExchangeService &service_;

    int readInt(const std::string &prompt, int min, int max) const;
    double readDouble(const std::string &prompt, double min, double max) const;
    std::string readString(const std::string &prompt) const;

    void showMenu() const;
    void handleExchange();
    void handleListTransactions();
    void handleSearchTransaction();
    void handleShowBalances();
    void handleSetRates();
    void handleAdjustReserves();
    void handleSetCriticalMins();
    void handleDailyReport();
    void handleMonthlyReport();
};
