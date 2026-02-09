# ICS0025 — Currency Exchange Store

## Main System Algorithm (Release 1)

1. User selects command in UI menu
2. UI reads required input values
3. UI sends input to Logic Layer
4. Logic Layer validates input
5. Logic Layer checks business rules
6. Logic Layer requests data from Repository
7. Logic Layer updates data and creates objects
8. Repository stores updated data
9. Logic Layer returns result
10. UI prints confirmation / receipt / report

The main scenario is **currency exchange**, which creates and stores a `Transaction` and updates currency balances.

## Architecture

UI Layer → Logic Layer → Repository → Data

This project uses:
- UI Layer: Console menu and input parsing
- Logic Layer: `ExchangeService`
- Repository: `CurrencyRepository`, `TransactionRepository`, `ReceiptRepository`
- Data: `Currency`, `Transaction`, `Receipt`

Repository storage choice: `std::vector` with `std::find_if` for simplicity and clear iteration.

## Build and Run (CMake)

```bash
cmake -S . -B build
cmake --build build
./build/exchange_store_cp1
```

### Qt UI (Final Release)

If Qt is installed, you can build the Qt UI target:

```bash
cmake -S . -B build -DENABLE_QT=ON
cmake --build build
./build/exchange_store_qt
```

## Persistence

Transactions are saved to `transactions.csv` on exit and loaded on start. Receipts are appended to `receipts.txt`.

## Release 2 Extension Point

Added a **monthly report** in addition to the daily report. This demonstrates a new report type with minimal refactoring.

## Release 3 Robustness

- Input validation in UI (numbers and ranges)
- Logic layer checks for invalid requests, missing currencies, and impossible operations
- Clear error messages returned to UI