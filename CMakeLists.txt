cmake_minimum_required(VERSION 3.10)
project(exchange_store_cp1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/qt_main.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/QtUI.cpp")

add_executable(exchange_store_cp1 ${SOURCES})
target_include_directories(exchange_store_cp1 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

option(ENABLE_QT "Build Qt UI" OFF)
if (ENABLE_QT)
	set(CMAKE_AUTOMOC ON)
	find_package(Qt6 QUIET COMPONENTS Widgets)
	if (Qt6_FOUND)
		set(QT_LIB Qt6::Widgets)
	else ()
		find_package(Qt5 QUIET COMPONENTS Widgets)
		if (Qt5_FOUND)
			set(QT_LIB Qt5::Widgets)
		endif ()
	endif ()

	if (QT_LIB)
		add_executable(exchange_store_qt
			"${CMAKE_CURRENT_SOURCE_DIR}/src/qt_main.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/QtUI.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/ExchangeService.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/Transaction.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/Currency.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/Receipt.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/CurrencyRepository.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/TransactionRepository.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/src/ReceiptRepository.cpp"
		)
		target_include_directories(exchange_store_qt PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
		target_link_libraries(exchange_store_qt PRIVATE ${QT_LIB})
	endif ()
endif ()
