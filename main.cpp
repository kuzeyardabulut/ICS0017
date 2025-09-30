#include <iostream>
#include <cstring>
#include <ctime>
#include "utils.hpp"
#include "currency_manager.hpp"

static int choose_currency(const char *prompt) {
    std::cout << prompt << std::endl;
    for (int i = 0; i < MAX_CUR; ++i) {
        std::cout << "  " << i << ") " << CUR_NAME[i] << std::endl;
    }
    int c = ask_int("Select currency index:", 0, MAX_CUR-1);
    return c;
}

// CurrencyManager encapsulates conversions and reserve checks
static CurrencyManager cm;

static void pay_in_denoms(int cur, double amount) {
    long long total = (long long)(amount);
    double fractional = amount - total;
    const int *den = DENOMS[cur];
    int n = D_COUNT[cur];

    if (total <= 0 || !den) {
        std::cout << "[-] No denomination breakdown available for this amount." << std::endl;
        return;
    }

    std::cout << "\n=== Denomination Breakdown for " << CUR_NAME[cur] << " " << amount << " ===\n";
    std::cout << "Notes/Coins Required:\n";

    int total_pieces = 0;
    for (int i = 0; i < n; ++i) {
        int d = den[i];
        if (d <= 0) continue;
        long long cnt = total / d;
        if (cnt > 0) {
            std::cout << "  " << d << " " << (d >= 20 ? "note(s)" : "coin(s)") << " x " << cnt << "\n";
            total -= cnt * d;
            total_pieces += (int)cnt;
        }
    }

    if (fractional > 0.009) {
        std::cout << "\nFractional amount: " << fractional << " " << CUR_NAME[cur] << "\n";
    }
    if (total > 0) {
        std::cout << "\nWarning: Remainder of " << total << " cannot be broken down further\n";
    }

    std::cout << "\nTotal pieces to handle: " << total_pieces << "\n";
    std::cout << "=======================================\n\n";
}

static void handle_receipt(int tx_id, const char *date_text, int from, int to, double amt_from,
                         double amt_to, double rate) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_str[9];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

    Transaction trans;
    trans.id = tx_id;
    trans.from_cur = from;
    trans.to_cur = to;
    trans.amount_from = amt_from;
    trans.amount_to = amt_to;
    trans.rate = rate;
    std::snprintf(trans.date, sizeof(trans.date), "%s", date_text);
    std::snprintf(trans.time, sizeof(trans.time), "%s", time_str);

    generate_receipt(trans, date_text);
}

static void scenario_exchange(void) {
    int from = choose_currency("Currency you GIVE to the cashier (from client):");
    int to   = choose_currency("Currency you WANT to receive (to client):");
    double amt_from = ask_double("Enter amount to exchange:", 0.01, 1e12);

    if (from == to) {
        std::cout << "[-] From and To currencies are the same. Nothing to do." << std::endl;
        return;
    }

    double rate_from_loc = 0.0, rate_to_loc = 0.0, profit_delta = 0.0;
    double amt_to = cm.convert_via_local(from, to, amt_from, &rate_from_loc, &rate_to_loc, &profit_delta);

    if (currencies[to].bal < amt_to) {
        std::cout << "[-] Insufficient reserve of " << CUR_NAME[to] << ". Available: " << currencies[to].bal << ", Needed: " << amt_to << std::endl;
        return;
    }

    int partial = ask_int("Partial exchange? 1=Yes, 0=No:", 0, 1);
    double part_fixed_to = 0.0;
    double remainder_loc_for_client = 0.0;

    if (partial) {
        part_fixed_to = ask_double("Enter how many units of the target currency to receive now:", 0.0, amt_to);
        double loc_value_total = amt_from * currencies[from].buy_to_loc;
        double loc_value_given_as_to = part_fixed_to * currencies[to].sell_to_loc;
        if (loc_value_given_as_to > loc_value_total + 1e-9) {
            std::cout << "[-] Chosen partial amount exceeds exchangeable value. Aborting." << std::endl;
            return;
        }
        remainder_loc_for_client = loc_value_total - loc_value_given_as_to;
        amt_to = part_fixed_to;
    }

    currencies[from].bal += amt_from;
    currencies[to].bal   -= amt_to;
    if (partial) {
        if (currencies[CUR_LOC].bal < remainder_loc_for_client) {
            std::cout << "[-] Insufficient LOC reserve for partial payout remainder (need " << remainder_loc_for_client << " LOC)." << std::endl;
            currencies[from].bal -= amt_from;
            currencies[to].bal   += amt_to;
            return;
        }
        currencies[CUR_LOC].bal -= remainder_loc_for_client;
    }

    profit_loc += profit_delta;

    double effective_rate = amt_to / amt_from;
    int tx_id = ++last_transaction_id;
    handle_receipt(tx_id, current_date, from, to, amt_from, amt_to, effective_rate);
    save_last_tx_id(last_transaction_id);

    if (partial) {
        std::cout << "Partial payout details: " << amt_to << " " << CUR_NAME[to] << " paid; remainder to client: " << remainder_loc_for_client << " LOC\n";
    }

    csv_log_transaction(current_date, tx_id, from, to, amt_from, amt_to,
                rate_from_loc, rate_to_loc,
                partial, remainder_loc_for_client, profit_delta);

    int want_denoms = ask_int("Would you like a denomination breakdown for the payout currency? 1=Yes,0=No:", 0, 1);
    if (want_denoms) {
        pay_in_denoms(to, amt_to);
        if (partial && remainder_loc_for_client > 0.0) {
            int want_loc = ask_int("Breakdown for the LOC remainder as well? 1=Yes,0=No:", 0, 1);
            if (want_loc) pay_in_denoms(CUR_LOC, remainder_loc_for_client);
        }
    }

    cm.check_criticals();
}

static void scenario_show_rates(void) {
    std::cout << "\n[*] Current Exchange Rates (relative to LOC)\n";
    std::cout << "Index  Code   BUY->LOC        SELL->LOC\n";
    for (int i = 0; i < MAX_CUR; ++i) {
        std::printf("%5d  %-5s  %12.6f  %12.6f\n", i, CUR_NAME[i], currencies[i].buy_to_loc, currencies[i].sell_to_loc);
    }
    std::cout << "Note: BUY->LOC is what the desk credits in LOC per 1 unit when client gives that currency.\n";
    std::cout << "      SELL->LOC is what the client must pay in LOC per 1 unit of that currency they receive.\n\n";
}

static void scenario_mgmt_set_rates(void) {
    std::cout << "\n--- Management: Set Rates (relative to LOC) ---\n";
    std::cout << "For each currency, enter BUY->LOC then SELL->LOC (must be > 0 and SELL >= BUY).\n";
    for (int i = 0; i < MAX_CUR; ++i) {
        std::cout << "Currency " << CUR_NAME[i] << ":\n";
        double buy = ask_double("  BUY->LOC:", 0.000001, 1e12);
        double sell = ask_double("  SELL->LOC (>= BUY):", buy, 1e12);
        currencies[i].buy_to_loc = buy;
        currencies[i].sell_to_loc = sell;
    }
    std::cout << "[*] Rates updated.\n\n";
}

static void scenario_mgmt_reserves(void) {
    std::cout << "\n--- Management: Adjust Reserves ---\n";
    int idx = choose_currency("Select currency to modify:");
    double delta = ask_double("Positive to add to reserve, negative to remove:", -1e12, 1e12);
    if (currencies[idx].bal + delta < 0) {
        std::cout << "[-] Operation would make reserve negative. Aborted." << std::endl;
        return;
    }
    currencies[idx].bal += delta;
    if (delta >= 0) std::cout << "Added " << delta << " " << CUR_NAME[idx] << " to reserves.\n";
    else            std::cout << "Removed " << -delta << " " << CUR_NAME[idx] << " from reserves.\n";
}

static void scenario_mgmt_crit(void) {
    std::cout << "\n--- Management: Set Critical Minimums ---\n";
    for (int i = 0; i < MAX_CUR; ++i) {
        std::cout << "Currency " << CUR_NAME[i] << ":\n";
        currencies[i].critical_min = ask_double("  Critical minimum:", 0.0, 1e12);
    }
    std::cout << "[*] Critical minimums updated.\n\n";
}

static void scenario_show_balances(void) {
    std::cout << "\n[*] Current Balances\n";
    // Print balances using fixed notation to avoid scientific format for large integers
    std::ios oldState(nullptr);
    oldState.copyfmt(std::cout);
    std::cout.setf(std::ios::fixed); std::cout.precision(6);
    for (int i = 0; i < MAX_CUR; ++i) {
        std::cout << CUR_NAME[i] << ": " << currencies[i].bal << "\n";
    }
    std::cout.copyfmt(oldState);
    std::cout << "\n";
}

static void scenario_help(void) {
    std::cout << "\n=== Currency Exchange System - Help ===\n\n";
    std::cout << "Available Operations:\n\n";
    std::cout << "1. Perform Exchange\n";
    std::cout << "2. Show Rates\n";
    std::cout << "3. Management: Set Rates\n";
    std::cout << "4. Management: Adjust Reserves\n";
    std::cout << "5. Management: Set Critical Minimums\n";
    std::cout << "6. Show Balances\n";
    std::cout << "7. End-of-day Report\n";
    std::cout << "8. Help/About\n";
    std::cout << "Press Enter to continue...";
    std::string dummy; std::getline(std::cin, dummy);
}

void scenario_end_of_day(const char *current_date) {
    generate_daily_summary(current_date);
    cm.check_criticals();
}

static void show_menu(void) {
    std::cout << "============================================\n";
    std::cout << "   [*] Currency Exchange - Main Menu\n";
    std::cout << "   Date: " << current_date << "\n";
    std::cout << "============================================\n";
    std::cout << " 1) Perform an exchange\n";
    std::cout << " 2) Show rates\n";
    std::cout << " 3) Management: set rates\n";
    std::cout << " 4) Management: adjust reserves\n";
    std::cout << " 5) Management: set critical minimums\n";
    std::cout << " 6) Show balances\n";
    std::cout << " 7) End-of-day report\n";
    std::cout << " 8) Help/About\n";
    std::cout << " 9) Add manual transaction (append to CSV)\n";
    std::cout << "10) List transactions for a date\n";
    std::cout << "11) Search transaction by ID (today)\n";
    std::cout << " 0) Exit\n";
}

int main() {
    init_defaults();
    // Ensure we persist runtime state on normal exit
    std::atexit(shutdown);
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", tm_info);

    while (true) {
        show_menu();
        int choice = ask_int("Choose option:", 0, 11);
        switch (choice) {
            case 0: return 0;
            case 1: scenario_exchange(); break;
            case 2: scenario_show_rates(); break;
            case 3: scenario_mgmt_set_rates(); break;
            case 4: scenario_mgmt_reserves(); break;
            case 5: scenario_mgmt_crit(); break;
            case 6: scenario_show_balances(); break;
            case 7: scenario_end_of_day(current_date); break;
            case 8: scenario_help(); break;
            case 9: {
                int from = choose_currency("From currency index:");
                int to = choose_currency("To currency index:");
                double amt_from = ask_double("Amount from:", 0.0, 1e12);
                double amt_to = ask_double("Amount to:", 0.0, 1e12);
                time_t tt = time(NULL);
                struct tm *tm2 = localtime(&tt);
                char timestr[16];
                strftime(timestr, sizeof(timestr), "%H:%M:%S", tm2);
                int txid = ++last_transaction_id;
                csv_append_manual_transaction(current_date, txid, timestr, CUR_NAME[from], CUR_NAME[to],
                                              amt_from, amt_to, currencies[from].buy_to_loc, currencies[to].sell_to_loc,
                                              0, 0.0, 0.0);
                save_last_tx_id(last_transaction_id);
                std::cout << "Added transaction id " << txid << "\n";
                break;
            }
            case 10: {
                char datebuf[32];
                std::cout << "Enter date (YYYY-MM-DD) or press Enter for today: ";
                if (!std::cin.getline(datebuf, sizeof(datebuf))) break;
                size_t L = strlen(datebuf); while (L && (datebuf[L-1]=='\n' || datebuf[L-1]=='\r')) datebuf[--L] = '\0';
                if (L == 0) std::snprintf(datebuf, sizeof(datebuf), "%s", current_date);
                csv_list_transactions_for_date(datebuf);
                break;
            }
            case 11: {
                int qid = ask_int("Enter transaction ID to search:", 1, 1000000000);
                int found = csv_find_transaction_by_id(current_date, qid);
                if (!found) std::cout << "Transaction " << qid << " not found for " << current_date << "\n";
                break;
            }
            default: std::cout << "Unknown option\n"; break;
        }
    }

    return 0;
}
