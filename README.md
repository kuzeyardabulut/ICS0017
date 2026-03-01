===========================================================
CURRENCY EXCHANGE STORE — FULL PROJECT REPORT (ICS0025)
FINAL RELEASE: Qt GUI + CONSOLE + FILE PERSISTENCE
===========================================================

1) PROJECT OVERVIEW
-------------------
Currency Exchange Store is a layered C++ application that simulates a small currency exchange office.
It supports two user interfaces:
  - Qt GUI (final release)
  - Console UI (menu-driven)

The system allows users to execute currency exchanges, record transactions, and generate simple
daily/monthly reports. The final release includes file persistence so that transactions and currency
state remain available after restarting the application.

Key goals of the final release:
  - Qt GUI executable builds and runs successfully
  - Data persistence works (transactions and currency state survive restart)
  - Clear, repeatable build/run instructions for evaluation


2) FEATURES IMPLEMENTED
-----------------------
A) Exchange operations
  - Execute exchange between currency codes (e.g., USD <-> LOC, EUR <-> LOC, etc.)
  - Optional partial exchange (if enabled via UI checkbox)
  - Reserve validation: an exchange can fail when the target currency does not have enough reserve
    ("Insufficient reserve in target currency").

B) Transaction management
  - Transactions are created and stored with:
      - unique transaction id
      - date and time
      - from currency and to currency
      - amount_from and amount_to
      - related rates
      - partial flag and partial remainder (if used)
  - List Transactions (Qt button / Console menu)

C) Reports
  - Daily Report (input format: YYYY-MM-DD)
  - Monthly Report (input format: YYYY-MM)

D) Persistence (Final Release)
  - Currency state and transaction history are persisted to disk under the data/ directory.
  - Transactions are saved atomically using a temporary file (tmp -> flush/close -> rename).


3) ARCHITECTURE (LAYERED DESIGN)
--------------------------------
The project follows a layered architecture:

  UI (Qt / Console)
      |
      v
  Logic (ExchangeService)
      |
      v
  Repositories (CurrencyRepository, TransactionRepository, ReceiptRepository)
      |
      v
  Data files (data/currencies.csv, data/transactions.csv, data/receipts.txt)

A) UI layer
  - Qt GUI:
      - User enters:
          From code, To code, Amount
          (Optional) Partial exchange + partial amount
          Report date (YYYY-MM-DD) / report month (YYYY-MM)
      - Buttons:
          Execute Exchange
          List Transactions
          Daily Report
          Monthly Report
      - UI displays results and error messages in the Output panel.
  - Console UI:
      - Menu-based interface to run the same operations.
      - Used for reserve adjustment in troubleshooting cases.

B) Logic layer (ExchangeService)
  - Validates requests:
      - currency codes exist
      - amount is valid
      - rate is valid
      - target currency has enough reserve
  - Performs computations:
      - converts from -> LOC using buyToLOC
      - converts LOC -> target using target sellToLOC (division)
      - produces amountTo, updates reserves, and creates transaction records
  - Triggers persistence (load/save) through repository interfaces.

C) Repository layer
  - CurrencyRepository
      - stores currencies in memory
      - loads/saves currency state to data/currencies.csv
      - ensures both buyToLOC and sellToLOC are correctly set when loading 4-column CSV format
  - TransactionRepository
      - stores transactions in memory
      - loads/saves transactions to data/transactions.csv
      - uses atomic persistence on save (tmp file -> rename)
  - ReceiptRepository (if used)
      - stores receipts and loads/saves to data/receipts.txt

D) Data model
  - Currency
      - code (e.g., LOC, USD, EUR)
      - balance/reserve
      - buyToLOC and sellToLOC rates (used in exchange computation)
      - critical minimum (if used)
      - startBalance (used for initialization / reporting)
  - Transaction
      - id, timestamp, from/to, amounts, rates, partial flag, etc.
  - Receipt
      - receipt records if used by the project


4) DATA PERSISTENCE (FINAL RELEASE)
-----------------------------------
Persistent data files:
  - data/currencies.csv     (currency reserves and rates)
  - data/transactions.csv   (transaction history)
  - data/receipts.txt       (receipts, if used)

4.1) When load/save happens
  - On startup:
      - currencies are loaded from data/currencies.csv
      - transactions are loaded from data/transactions.csv
      - if a file does not exist, the app seeds default currencies (initial state)
  - On exit:
      - currencies are saved back to data/currencies.csv
      - transactions are saved back to data/transactions.csv

4.2) Atomic persistence (transactions)
Transactions are saved atomically using a temporary file approach:
  - write to: data/transactions.csv.tmp
  - flush + close
  - rename .tmp -> data/transactions.csv
This prevents corrupted files if the program exits mid-write.

4.3) Currency CSV format (backward compatible)
The repo supports a 4-column format:
  code,balance,rate,critical_minimum

When loading this format:
  - buyToLOC  = rate
  - sellToLOC = rate
  - startBalance = balance (so runtime state matches loaded state)

Example (illustrative):
  USD,11010.0,41.36,2000
  LOC,49586.4,1.0,10000


5) BUILD + RUN (macOS)
----------------------

(0) IMPORTANT: Always run commands from the project root (where CMakeLists.txt is).

5.1) Prerequisites
Install Qt via Homebrew:
  brew install qt

5.2) Build Qt GUI (Final Release)
QT_PREFIX="$(brew --prefix qt)"
cmake -S . -B build -DENABLE_QT=ON -DCMAKE_PREFIX_PATH="$QT_PREFIX"
cmake --build build

Run Qt GUI:
  ./build/exchange_store_qt

5.3) Build Console Version
cmake -S . -B build
cmake --build build

Run Console:
  ./build/exchange_store_cp1

5.4) Check binaries exist
ls -la build | grep exchange

Expected binaries:
  build/exchange_store_qt
  build/exchange_store_cp1


6) HOW TO USE (Qt GUI)
----------------------
Start the app:
  ./build/exchange_store_qt

6.1) Demo exchange #1 (USD -> LOC)
  From code: USD
  To code:   LOC
  Amount:    10
  Click: Execute Exchange
Expected: "Exchange completed successfully" + Transaction ID.

6.2) Demo exchange #2 (LOC -> USD)
  From code: LOC
  To code:   USD
  Amount:    10
  Click: Execute Exchange
Expected: "Exchange completed successfully" + Transaction ID.

6.3) List transactions
  Click: List Transactions
Expected: previously created transactions appear in Output.

6.4) Reports
  - Daily Report:
      - Enter date as YYYY-MM-DD
      - Click Daily Report
  - Monthly Report:
      - Enter month as YYYY-MM
      - Click Monthly Report


7) PERSISTENCE VERIFICATION (GRADER CHECKLIST)
----------------------------------------------
1) Run Qt GUI and execute at least one exchange.
2) Close the Qt app.
3) Reopen Qt GUI:
     ./build/exchange_store_qt
4) Click: List Transactions
   -> Previous transactions must still appear (data is persisted).

Optional file inspection:
  ls -la data
  tail -n 5 data/transactions.csv
  head -n 5 data/currencies.csv


8) TROUBLESHOOTING
------------------
A) exchange_store_qt missing
  - Qt was not found or Qt build was not enabled.
  - Rebuild using:
      QT_PREFIX="$(brew --prefix qt)"
      cmake -S . -B build -DENABLE_QT=ON -DCMAKE_PREFIX_PATH="$QT_PREFIX"
      cmake --build build

B) "Insufficient reserve in target currency"
  - Means the target currency does not have enough reserve for the payout.
  - Options:
      1) Reduce the amount
      2) Adjust reserves using console version:
           ./build/exchange_store_cp1
         Then choose menu item "Adjust reserves" and increase the target currency.

C) "Invalid exchange rate: target currency sellToLOC is zero or negative."
  - Means loaded currency rate was missing/invalid.
  - Reset demo state (see section 9) to regenerate consistent currency state.

D) Data folder / file not found
  - Ensure data directory exists:
      mkdir -p data


9) DEMO RESET (OPTIONAL — DELETES SAVED DATA)
---------------------------------------------
This removes saved state and re-seeds initial state on next run:
  rm -f data/currencies.csv data/transactions.csv
  mkdir -p data
  ./build/exchange_store_qt

END OF REPORT