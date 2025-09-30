#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <cstdio>

constexpr int MAX_CUR = 5;
constexpr int MAX_NAME = 8;

enum { CUR_LOC = 0, CUR_USD = 1, CUR_EUR = 2, CUR_GBP = 3, CUR_JPY = 4 };

struct Transaction {
    int id{0};
    char date[11]{};
    char time[9]{};
    int from_cur{0};
    int to_cur{0};
    double amount_from{0.0};
    double amount_to{0.0};
    double rate{0.0};
    double profit{0.0};
};

struct Currency {
    char name[MAX_NAME];
    int d_count{0};
    const int *denoms{nullptr};
    double start_bal{0.0};
    double bal{0.0};
    double critical_min{0.0};
    double buy_to_loc{0.0};
    double sell_to_loc{0.0};
};

extern std::vector<Currency> currencies;
extern const char * const CUR_NAME[MAX_CUR];
extern const int * const DENOMS[MAX_CUR];
extern const int D_COUNT[MAX_CUR];
extern double profit_loc;
extern char current_date[64];
extern int last_transaction_id;

void init_defaults();
int ask_int(const char *prompt, int min, int max);
double ask_double(const char *prompt, double min, double max);
void clear_input();

void make_daily_csv_name(const char *date_text, char *out, size_t cap);
void generate_receipt(const Transaction &t, const char *date_text);
void generate_daily_summary(const char *date_text);
double csv_sum_profit_for_date(const char *date_text, int *tx_count_out);
void ensure_csv_header(FILE *f);
void csv_log_transaction(const char *date_text, int tx_id, int from, int to,
                         double amt_from, double amt_to, double rate_from_loc, double rate_to_loc,
                         int partial, double remainder_loc_for_client, double profit_delta_loc);

int csv_list_transactions_for_date(const char *date_text);
int csv_find_transaction_by_id(const char *date_text, int tx_id);
int csv_append_manual_transaction(const char *date_text, int tx_id,
                                  const char *time_text, const char *from_code, const char *to_code,
                                  double amt_from, double amt_to, double rate_from_loc, double rate_to_loc,
                                  int partial, double remainder_loc, double profit_loc_delta);

void save_last_tx_id(int id);
int load_last_tx_id();

// Called at program exit to persist runtime state
void shutdown();

extern const int DENOMS_LOC[];
extern const int DENOMS_USD[];
extern const int DENOMS_EUR[];
extern const int DENOMS_GBP[];
extern const int DENOMS_JPY[];
