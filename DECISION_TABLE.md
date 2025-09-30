Decision table: currency exchange store

Rows: Input conditions / Actions

- Inputs considered:
  - operation type: exchange, add_tx, list_date, search_tx
  - currency indices in valid range
  - sufficient reserve for payout
  - partial flag (0/1)
  - CSV format version (old/no-txid, new/with-txid)

- Actions:
  - Accept/Reject transaction
  - Persist transaction to CSV
  - Generate receipt
  - Update balances
  - Return error codes / messages

Table (selected rows):

1) Exchange, currencies valid, reserve sufficient, partial=0 => Accept, update balances, persist csv (tx_id), generate receipt
2) Exchange, currencies valid, reserve insufficient => Reject, no change, show error
3) Add_tx (manual), currencies valid => Append to CSV, optionally update last_tx_id
4) List_date, valid date file exists => Read CSV (both formats) and print rows
5) Search_tx, valid tx_id => Locate row in today's CSV (or specified date) and print details
6) Any CSV read with malformed rows => Skip row, log warning

Notes:
- CSV compatibility: parser accepts both legacy (no tx_id) and new format.
- Transaction ID persistence ensures unique IDs across runs.
