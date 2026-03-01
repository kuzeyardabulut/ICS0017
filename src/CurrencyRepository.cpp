 #include "repository/CurrencyRepository.hpp"
 #include <algorithm>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <sys/stat.h>

 void CurrencyRepository::add(const Currency &currency) {
     currencies_.push_back(currency);
 }

 Currency *CurrencyRepository::findById(int id) {
     auto it = std::find_if(currencies_.begin(), currencies_.end(),
                            [id](const Currency &c) { return c.id == id; });
     if (it == currencies_.end()) return nullptr;
     return &(*it);
 }

 Currency *CurrencyRepository::findByCode(const std::string &code) {
     auto it = std::find_if(currencies_.begin(), currencies_.end(),
                            [&code](const Currency &c) { return c.code == code; });
     if (it == currencies_.end()) return nullptr;
     return &(*it);
 }

 const std::vector<Currency> &CurrencyRepository::getAll() const {
     return currencies_;
 }

bool CurrencyRepository::loadFromFile(const std::string &filename, std::string &errorMessage) {
    currencies_.clear();
    std::ifstream in(filename);
    if (!in) {
        errorMessage = "File not found: " + filename;
        return false;
    }
    std::string line;
    if (!std::getline(in, line)) {
        errorMessage = "Empty file: " + filename;
        return false;
    }
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string code;
        double balance = 0, rate = 0, criticalMin = 0;
        if (!std::getline(ss, code, ',')) continue;
        std::string balStr, rateStr, critStr;
        if (!std::getline(ss, balStr, ',')) continue;
        if (!std::getline(ss, rateStr, ',')) continue;
        if (!std::getline(ss, critStr, ',')) continue;
        try {
            balance = std::stod(balStr);
            rate = std::stod(rateStr);
            criticalMin = std::stod(critStr);
        } catch (...) { continue; }
        Currency c;
        c.code = code;
        c.balance = balance;
        c.startBalance = balance; // Ensure startBalance matches balance after load
        c.buyToLoc = rate;
        c.sellToLoc = rate; // Fix: set sellToLoc for 4-column CSV
        c.criticalMin = criticalMin;
        currencies_.push_back(c);
    }
    return !currencies_.empty();
}

bool CurrencyRepository::saveToFile(const std::string &filename, std::string &errorMessage) const {
    // Ensure directory exists
    size_t slash = filename.find_last_of("/");
    if (slash != std::string::npos) {
        std::string dir = filename.substr(0, slash);
        struct stat st = {0};
        if (stat(dir.c_str(), &st) == -1) {
            mkdir(dir.c_str(), 0755);
        }
    }
    std::string tmp = filename + ".tmp";
    std::ofstream out(tmp, std::ios::trunc);
    if (!out) {
        errorMessage = "Failed to open temp file for writing: " + tmp;
        return false;
    }
    out << "code,balance,rate,critical_minimum\n";
    for (const auto& c : currencies_) {
        out << c.code << ',' << c.balance << ',' << c.buyToLoc << ',' << c.criticalMin << '\n';
    }
    out.flush();
    out.close();
    if (!out) {
        std::remove(tmp.c_str());
        errorMessage = "Failed to write temp file: " + tmp;
        return false;
    }
    if (std::rename(tmp.c_str(), filename.c_str()) != 0) {
        int err = errno;
        std::remove(tmp.c_str());
        errorMessage = "Failed to rename temp file to target: " + filename + " (errno=" + std::to_string(err) + ")";
        return false;
    }
    return true;
}
