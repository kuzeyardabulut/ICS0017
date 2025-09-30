#include "utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>

#include <algorithm>

// Use MAX_CUR from utils.hpp to avoid duplication and potential mismatches

const char * const CUR_NAME[MAX_CUR] = { "LOC", "USD", "EUR", "GBP", "JPY" };
const int D_COUNT[MAX_CUR] =  { 9, 7, 7, 7, 7 };
const int DENOMS_LOC[] = { 200, 100, 50, 20, 10, 5, 2, 1, 0 };
const int DENOMS_USD[] = { 100, 50, 20, 10, 5, 2, 1 };
const int DENOMS_EUR[] = { 500, 200, 100, 50, 20, 10, 5 };
const int DENOMS_GBP[] = { 50, 20, 10, 5, 2, 1, 0 };
const int DENOMS_JPY[] = { 10000, 5000, 2000, 1000, 500, 100, 50 };
const int * const DENOMS[MAX_CUR] = { DENOMS_LOC, DENOMS_USD, DENOMS_EUR, DENOMS_GBP, DENOMS_JPY };

// Use vector for automatic lifetime management and RAII
std::vector<Currency> currencies;

double profit_loc = 0.0;
char current_date[64] = "N/A";
int last_transaction_id = 0;

static const char *RECEIPT_FILE = "receipts_%s.txt";

int load_last_tx_id(void) {
    std::ifstream f("last_tx_id.txt");
    if (!f) return 0;
    int id = 0;
    f >> id;
    if (!f) id = 0;
    return id;
}

void save_last_tx_id(int id) {
    std::ofstream f("last_tx_id.txt");
    if (!f) return;
    f << id << '\n';
}

void clear_input(void) {
    // In C++ we will clear the remaining input line from stdin
    std::cin.clear();
    std::string tmp;
    std::getline(std::cin, tmp);
}

int ask_int(const char *prompt, int min, int max) {
    int x = 0;
    std::string line;
    while (true) {
        if (prompt && prompt[0]) std::cout << prompt << " ";
        std::cout.flush();
        if (!std::getline(std::cin, line)) {
            std::cout << "Error reading input. Try again." << std::endl;
            continue;
        }
        std::stringstream ss(line);
        if (!(ss >> x)) {
            std::cout << "Invalid input. Try again." << std::endl;
            continue;
        }
        if (x < min || x > max) {
            std::cout << "Please enter a number in [" << min << ".." << max << "]" << std::endl;
            continue;
        }
        return x;
    }
}

double ask_double(const char *prompt, double min, double max) {
    std::string line;
    double x = 0.0;
    while (true) {
        if (prompt && prompt[0]) std::cout << prompt << " ";
        std::cout.flush();
        if (!std::getline(std::cin, line)) {
            std::cout << "Error reading input. Please try again." << std::endl;
            continue;
        }
        if (line.empty()) {
            std::cout << "Empty input not allowed. Please enter a number." << std::endl;
            continue;
        }
        char *endptr = nullptr;
        errno = 0;
        x = strtod(line.c_str(), &endptr);
        if (endptr == line.c_str() || (*endptr != '\0' && *endptr != '\n')) {
            std::cout << "Invalid input: Please enter a valid number." << std::endl;
            continue;
        }
        if (errno == ERANGE) {
            std::cout << "Number out of range. Try again." << std::endl;
            continue;
        }
        if (x < min || x > max) {
            std::cout << "Value must be between " << min << " and " << max << ". Please try again." << std::endl;
            continue;
        }
        return x;
    }
}

void init_defaults(void) {
    currencies.clear();
    currencies.resize(MAX_CUR);
    for (int i = 0; i < MAX_CUR; ++i) {
        const char *src = CUR_NAME[i];
        std::strncpy(currencies[i].name, src, MAX_NAME-1);
        currencies[i].name[MAX_NAME-1] = '\0';
        currencies[i].d_count = D_COUNT[i];
        currencies[i].denoms = DENOMS[i];
    }

    currencies[CUR_LOC].start_bal = 50000.0;
    currencies[CUR_USD].start_bal = 10000.0;
    currencies[CUR_EUR].start_bal = 8000.0;
    currencies[CUR_GBP].start_bal = 3000.0;
    currencies[CUR_JPY].start_bal = 1000000.0;

    for (int i = 0; i < MAX_CUR; ++i)
        currencies[i].bal = currencies[i].start_bal;

    currencies[CUR_LOC].critical_min = 10000.0;
    currencies[CUR_USD].critical_min = 2000.0;
    currencies[CUR_EUR].critical_min = 1500.0;
    currencies[CUR_GBP].critical_min = 500.0;
    currencies[CUR_JPY].critical_min = 200000.0;

    currencies[CUR_LOC].buy_to_loc = 1.0;   currencies[CUR_LOC].sell_to_loc = 1.0;
    currencies[CUR_USD].buy_to_loc = 41.36; currencies[CUR_USD].sell_to_loc = 41.45;
    currencies[CUR_EUR].buy_to_loc = 48.38; currencies[CUR_EUR].sell_to_loc = 48.60;
    currencies[CUR_GBP].buy_to_loc = 55.91; currencies[CUR_GBP].sell_to_loc = 56.26;
    currencies[CUR_JPY].buy_to_loc = 0.27;  currencies[CUR_JPY].sell_to_loc = 0.28;

    last_transaction_id = load_last_tx_id();
}

void shutdown() {
    // Persist last transaction id on exit
    save_last_tx_id(last_transaction_id);
}

void make_daily_csv_name(const char *date_text, char *out, size_t cap) {
    snprintf(out, cap, "sales_%s.csv", date_text);
}

void generate_receipt(const Transaction &t, const char *date_text) {
    char fname[128];
    snprintf(fname, sizeof(fname), RECEIPT_FILE, date_text);

    std::ofstream f(fname, std::ios::app);
    if (!f) {
        std::cerr << "Error opening receipt file: " << fname << std::endl;
        return;
    }

    f << "\n========= CURRENCY EXCHANGE RECEIPT =========\n";
    f << "Transaction ID: " << t.id << "\n";
    f << "Date: " << t.date << " " << t.time << "\n";
    f.setf(std::ios::fixed); f.precision(6);
    f << "From: " << t.amount_from << " " << CUR_NAME[t.from_cur] << "\n";
    f << "To: " << t.amount_to << " " << CUR_NAME[t.to_cur] << "\n";
    f << "Rate: 1 " << CUR_NAME[t.from_cur] << " = " << t.rate << " " << CUR_NAME[t.to_cur] << "\n";
    f.unsetf(std::ios::floatfield);
    f << "==========================================\n\n";

    f.close();

    std::cout << "\nReceipt generated and saved to " << fname << std::endl;
}

double csv_sum_profit_for_date(const char *date_text, int *tx_count_out) {
    char fname[128];
    make_daily_csv_name(date_text, fname, sizeof(fname));

    std::ifstream f(fname);
    if (!f) {
        if (tx_count_out) *tx_count_out = 0;
        return 0.0;
    }

    std::string line;
    // skip header line
    if (!std::getline(f, line)) { if (tx_count_out) *tx_count_out = 0; return 0.0; }

    double total_profit = 0.0;
    int count = 0;

    while (std::getline(f, line)) {
    (void)line; // line is used below via parts; avoid unused-variable warnings

        // Try new format first (with tx_id)
        // We'll parse by comma positions
        std::vector<std::string> parts;
        std::string token;
        std::istringstream partss(line);
        while (std::getline(partss, token, ',')) parts.push_back(token);
        if (parts.size() >= 12) {
            try {
                double prof = std::stod(parts[11]);
                total_profit += prof;
                count++;
                continue;
            } catch (...) {}
        }
        if (parts.size() >= 11) {
            try {
                double prof = std::stod(parts[10]);
                total_profit += prof;
                count++;
                continue;
            } catch (...) {}
        }
        // else skip
    }

    if (tx_count_out) *tx_count_out = count;
    return total_profit;
}

void ensure_csv_header(FILE *f) {
    long pos = ftell(f);
    if (pos == 0) {
        fprintf(f,
            "date,time,tx_id,from_currency,to_currency,amount_from,amount_to,rate_from_loc,rate_to_loc,partial,remainder_loc,profit_loc\n");
        fflush(f);
    }
}

void csv_log_transaction(
    const char *date_text,
    int tx_id,
    int from, int to,
    double amt_from, double amt_to,
    double rate_from_loc, double rate_to_loc,
    int partial, double remainder_loc_for_client,
    double profit_delta_loc
) {
    char fname[128];
    make_daily_csv_name(date_text, fname, sizeof(fname));

    FILE *f = fopen(fname, "a");
    if (!f) {
        std::fprintf(stderr, "CSV open failed (%s): %s\n", fname, std::strerror(errno));
        return;
    }
    ensure_csv_header(f);

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char timebuf[16];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm_info);

    std::fprintf(f, "%s,%s,%d,%s,%s,%.6f,%.6f,%.6f,%.6f,%d,%.6f,%.6f\n",
        date_text, timebuf, tx_id,
        CUR_NAME[from], CUR_NAME[to],
        amt_from, amt_to,
        rate_from_loc, rate_to_loc,
        partial ? 1 : 0, remainder_loc_for_client,
        profit_delta_loc
    );
    fclose(f);
}

int csv_list_transactions_for_date(const char *date_text) {
    char fname[128];
    make_daily_csv_name(date_text, fname, sizeof(fname));
    std::ifstream f(fname);
    if (!f) {
        std::fprintf(stderr, "Could not open %s for reading: %s\n", fname, std::strerror(errno));
        return -1;
    }

    std::string line;
    if (!std::getline(f, line)) { f.close(); return 0; }

    std::printf("Transactions in %s:\n", fname);
    int printed = 0;
    while (std::getline(f, line)) {
        // Trim newline/carriage returns already removed by getline
        if (line.empty()) continue;
        std::printf("%s\n", line.c_str());
        printed++;
    }
    f.close();
    return printed;
}

int csv_find_transaction_by_id(const char *date_text, int tx_id) {
    char fname[128];
    make_daily_csv_name(date_text, fname, sizeof(fname));
    std::ifstream f(fname);
    if (!f) {
        std::fprintf(stderr, "Could not open %s for searching: %s\n", fname, std::strerror(errno));
        return -1;
    }

    std::string line;
    if (!std::getline(f, line)) { f.close(); return 0; }

    int found = 0;
    while (std::getline(f, line)) {
        std::vector<std::string> parts;
        std::string token;
        std::istringstream partss(line);
        while (std::getline(partss, token, ',')) parts.push_back(token);
        if (parts.size() >= 3) {
            try {
                int id = std::stoi(parts[2]);
                if (id == tx_id) { std::printf("%s\n", line.c_str()); found = 1; break; }
            } catch (...) {}
        }
    }
    f.close();
    return found;
}

int csv_append_manual_transaction(const char *date_text, int tx_id,
                                  const char *time_text,
                                  const char *from_code, const char *to_code,
                                  double amt_from, double amt_to,
                                  double rate_from_loc, double rate_to_loc,
                                  int partial, double remainder_loc, double profit_loc_delta) {
    char fname[128];
    make_daily_csv_name(date_text, fname, sizeof(fname));
    std::ofstream f(fname, std::ios::app);
    if (!f) {
        std::fprintf(stderr, "Could not open %s for appending: %s\n", fname, std::strerror(errno));
        return -1;
    }
    // Ensure header exists
    // We'll open a FILE* to check position
    FILE *cf = fopen(fname, "r+");
    if (cf) { ensure_csv_header(cf); fclose(cf); }

    f << date_text << ',' << time_text << ',' << tx_id << ',' << from_code << ',' << to_code << ','
      << amt_from << ',' << amt_to << ',' << rate_from_loc << ',' << rate_to_loc << ',' << (partial ? 1 : 0) << ','
      << remainder_loc << ',' << profit_loc_delta << '\n';
    f.close();
    return 0;
}

void generate_daily_summary(const char *date_text) {
    int tx_count = 0;
    double total_profit = csv_sum_profit_for_date(date_text, &tx_count);

    std::cout << "\n=== End-of-day report for " << date_text << " ===\n";
    std::cout << "Total Transactions: " << tx_count << "\n";
    std::cout << "Total Profit (LOC): " << total_profit << "\n";
    std::cout << "(Transactions are read from sales_" << date_text << ".csv)\n\n";
    std::cout.flush();
}
