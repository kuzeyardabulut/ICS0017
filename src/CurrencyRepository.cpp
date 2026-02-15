 #include "repository/CurrencyRepository.hpp"
 #include <algorithm>

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
