#include "data/Receipt.hpp"
#include <sstream>

std::string Receipt::toString() const {
    std::ostringstream out;
    out << "Receipt " << id << " (tx=" << transactionId << ") " << date << " " << time << "\n";
    out << text;
    return out.str();
}
