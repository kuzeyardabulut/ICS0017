#include "data/Transaction.hpp"
#include <sstream>

std::string Transaction::toString() const {
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(6);
    out << id << "," << date << "," << time << "," << fromCode << "," << toCode
        << "," << amountFrom << "," << amountTo
        << "," << rateFromLoc << "," << rateToLoc
        << "," << (partial ? 1 : 0) << "," << remainderLoc
        << "," << profitLoc;
    return out.str();
}
