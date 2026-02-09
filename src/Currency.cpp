#include "data/Currency.hpp"
#include <sstream>

std::string Currency::toString() const {
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(6);
    out << code << " | balance=" << balance
        << " | buy->loc=" << buyToLoc
        << " | sell->loc=" << sellToLoc;
    return out.str();
}
