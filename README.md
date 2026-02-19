# Currency Exchange Store  
**ICS0025 Course Project**

---

## 1. Project Overview

This project implements a layered **currency exchange office system** developed according to the ICS0025 course requirements.

The system:

- Manages multiple currencies  
- Executes exchange transactions  
- Generates receipts  
- Stores transaction history  
- Persists data to files  
- Provides both Console UI and optional Qt GUI  

The application is delivered as a **fully integrated final version** while conceptually satisfying all intermediate course stages.

---

## 2. System Architecture

The project strictly follows the required layered architecture:

UI → Logic → Repository → Data → Data File

### UI Layer
- `ConsoleUI` (console interface)  
- `QtUI` (Qt widget-based GUI, optional)  
- Handles only user interaction  
- Does NOT contain business logic  
- Does NOT perform file I/O  

### Logic Layer
- `ExchangeService`
- Implements:
  - Validation  
  - Business rules  
  - Calculations  
  - Coordination between repositories  
- Does NOT perform UI operations  
- Does NOT directly access files  

### Repository Layer
- `CurrencyRepository`  
- `TransactionRepository`  
- `ReceiptRepository`  
- Stores entity objects (not strings)  
- Provides CRUD-style methods  
- Handles file persistence  

### Data Layer
- `Currency`  
- `Transaction`  
- `Receipt`  
- Plain data classes with unique IDs  
- No business logic  

### Data Files
- `transactions.csv`  
- `receipts.txt`  
- Saved using atomic write strategy (temporary file + rename)

All communication flows through `ExchangeService`.

---

## 3. Main System Algorithm

**Main operation: Execute Currency Exchange**

1. User selects exchange operation in UI.  
2. UI collects source currency, target currency, and amount.  
3. UI sends request to `ExchangeService`.  
4. Logic layer validates:
   - Amount > 0  
   - Currencies exist  
   - Source ≠ Target  
   - Sufficient reserve available  
5. Logic layer calculates exchange result.  
6. A `Transaction` object is created.  
7. Repository updates currency balances.  
8. Transaction is stored.  
9. A `Receipt` is generated and stored.  
10. Result is returned to UI and displayed.  

This operation mutates system state and persists data.

---

## 4. Compliance with Course Requirements  
*(Integrated Final Version)*

The project is implemented directly as a final integrated system rather than as separate release folders.

However, it satisfies all conceptual requirements defined in Release 1–3 and the Final stage.

### Core Structured Architecture (Release 1 concepts)

- Layered architecture implemented  
- Console UI available  
- End-to-end exchange scenario  
- Multiple entity classes  
- In-memory repositories  
- Buildable CMake project  

### Designed for Change (Release 2 concepts)

- Modular logic layer (`ExchangeService`)  
- Use of STL algorithms and lambda expressions  
- Clear separation of concerns  
- Easily extendable structure  

### Robust Behavior (Release 3 concepts)

- Input validation  
- Handling invalid IDs  
- Handling insufficient reserves  
- Border value checks  
- Safe state updates  
- Clear error messages  

### Integrated Product (Final Stage)

- Qt-based UI  
- Repository `saveToFile()` and `loadFromFile()`  
- Atomic file persistence (.tmp → rename)  
- UI never performs file operations  
- Persistence test scenario passes  

---

## 5. Error Handling Strategy

- UI validates basic input format.  
- Logic layer validates business constraints.  
- Repository methods return status and error messages.  
- Atomic file writes prevent data corruption.  
- Failed operations do not corrupt system state.  

All errors are reported clearly to the user.

---

## 6. Persistence Implementation

Persistence is handled inside the repository layer.

Implementation details:

- Data is written to a temporary file  
- File is flushed and closed  
- Temporary file is atomically renamed  
- On program start, repositories load stored data  

### Persistence Test Scenario

1. Run program  
2. Execute exchange  
3. Save data  
4. Close program  
5. Run program again  
6. Stored transaction remains available  

---

## 7. Build & Run Instructions

### Requirements

- C++17 compatible compiler  
- CMake 3.10+  
- Optional Qt 5/6  

---

### Build Console Version

```bash
# create data directory for persistence
mkdir -p data

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
./exchange_store_cp1

Build Qt Version

cmake -DENABLE_QT=ON ..
cmake --build .
./exchange_store_qt

## 8. Design Decisions

- `std::vector` is used in repositories for predictable iteration and simplicity.  
- STL algorithms such as `std::find_if` and `std::accumulate` are used for searching and calculations.  
- Clear separation of layers improves maintainability and readability.  
- `AppConfig` centralizes file names and prevents hardcoded paths.  
- Atomic file rename strategy ensures safe persistence and prevents data corruption.  

---

## 9. Architecture Compliance

The project complies with all ICS0025 architectural constraints:

- UI layer does not access repositories directly.  
- Business logic is isolated in `ExchangeService`.  
- Repositories store entity objects (not strings).  
- Data classes contain no business logic.  
- Only the `src/` directory is compiled.  
- CMake sets `CMAKE_CXX_STANDARD 17`.  

All communication strictly follows:

UI → Logic → Repository → Data → File  

---

## 10. Conclusion

This project delivers a complete, buildable, and robust currency exchange application that satisfies all ICS0025 requirements.

The implementation demonstrates:

- Clean layered architecture  
- Proper separation of concerns  
- Extensibility  
- Robust error handling  
- Safe file persistence  
- Optional Qt GUI integration  

The system is ready for evaluation.