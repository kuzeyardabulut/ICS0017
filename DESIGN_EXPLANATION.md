Design choices and explanation

Structures:
- Currency: stores name, denominations pointer, current balance, start balance, critical minimum, buy/sell rates. Rationale: centralizes per-currency runtime state and keeps code readable.

- Transaction: contains tx id, date, time, from/to indices, amounts, rate, profit. Rationale: keeps logged and receipt data together and simplifies CSV logging and receipt generation.

Functions:
- csv_log_transaction: single responsibility to append transaction rows to daily CSV file. Ensures header is present and writes a timestamp and tx_id.

- csv_sum_profit_for_date: encapsulates CSV parsing and deals with both old/new formats. Returns total profit and number of transactions. Rationale: isolates format handling and provides a single source of truth for summary reporting.

- generate_receipt: writes a human-readable receipt to a per-day receipts file. Rationale: persistence and audit trail.

- load_last_tx_id / save_last_tx_id: persist transaction id counter. Rationale: IDs must survive restarts for audit.

Error handling decisions:
- CSV read: skip malformed rows and continue; count only successfully parsed lines.
- CSV write: check fopen result and print stderr on error.
- Input: use fgets+strtod for doubles to avoid scanf pitfalls.

Testing approach:
- Unit tests target csv_sum_profit_for_date parsing both old and new CSV rows.
- Integration test: simulate adding a transaction and then listing the date to verify the row appears and the tx_id increments.


