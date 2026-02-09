#pragma once

#include <string>

struct Receipt {
    int id{0};
    int transactionId{0};
    std::string date;
    std::string time;
    std::string text;

    std::string toString() const;
};
