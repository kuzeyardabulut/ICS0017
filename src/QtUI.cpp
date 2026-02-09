#include "ui/QtUI.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

QtUI::QtUI(ExchangeService &service, QWidget *parent)
    : QWidget(parent),
      service_(service) {
    setWindowTitle("Currency Exchange Store");

    auto *formLayout = new QFormLayout();
    fromCodeEdit_ = new QLineEdit();
    toCodeEdit_ = new QLineEdit();
    amountEdit_ = new QLineEdit();
    partialCheck_ = new QCheckBox("Partial exchange");
    partialAmountEdit_ = new QLineEdit();
    dateEdit_ = new QLineEdit();
    monthEdit_ = new QLineEdit();

    formLayout->addRow("From code:", fromCodeEdit_);
    formLayout->addRow("To code:", toCodeEdit_);
    formLayout->addRow("Amount:", amountEdit_);
    formLayout->addRow(partialCheck_);
    formLayout->addRow("Partial amount:", partialAmountEdit_);
    formLayout->addRow("Report date (YYYY-MM-DD):", dateEdit_);
    formLayout->addRow("Report month (YYYY-MM):", monthEdit_);

    auto *buttonsLayout = new QHBoxLayout();
    auto *exchangeButton = new QPushButton("Execute Exchange");
    auto *listButton = new QPushButton("List Transactions");
    auto *dailyButton = new QPushButton("Daily Report");
    auto *monthlyButton = new QPushButton("Monthly Report");
    buttonsLayout->addWidget(exchangeButton);
    buttonsLayout->addWidget(listButton);
    buttonsLayout->addWidget(dailyButton);
    buttonsLayout->addWidget(monthlyButton);

    output_ = new QTextEdit();
    output_->setReadOnly(true);

    auto *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addWidget(new QLabel("Output:"));
    mainLayout->addWidget(output_);
    setLayout(mainLayout);

    connect(exchangeButton, &QPushButton::clicked, this, &QtUI::onExecuteExchange);
    connect(listButton, &QPushButton::clicked, this, &QtUI::onListTransactions);
    connect(dailyButton, &QPushButton::clicked, this, &QtUI::onDailyReport);
    connect(monthlyButton, &QPushButton::clicked, this, &QtUI::onMonthlyReport);
}

void QtUI::appendOutput(const QString &text) {
    output_->append(text);
}

void QtUI::onExecuteExchange() {
    ExchangeRequest request;
    request.fromCode = fromCodeEdit_->text().trimmed().toStdString();
    request.toCode = toCodeEdit_->text().trimmed().toStdString();
    request.amountFrom = amountEdit_->text().trimmed().toDouble();
    request.partial = partialCheck_->isChecked();
    request.partialToAmount = partialAmountEdit_->text().trimmed().toDouble();

    ExchangeResult result = service_.executeExchange(request);
    if (!result.success) {
        appendOutput("[-] " + QString::fromStdString(result.message));
        return;
    }

    appendOutput("[+] " + QString::fromStdString(result.message));
    appendOutput("Transaction ID: " + QString::number(result.transaction.id));
    appendOutput("From: " + QString::number(result.transaction.amountFrom, 'f', 6) + " " + QString::fromStdString(result.transaction.fromCode));
    appendOutput("To: " + QString::number(result.transaction.amountTo, 'f', 6) + " " + QString::fromStdString(result.transaction.toCode));
    if (result.transaction.partial) {
        appendOutput("Remainder LOC: " + QString::number(result.remainderLoc, 'f', 6));
    }
    for (const auto &warning : result.warnings) {
        appendOutput("[!] " + QString::fromStdString(warning));
    }
}

void QtUI::onListTransactions() {
    auto transactions = service_.listTransactions();
    if (transactions.empty()) {
        appendOutput("No transactions available.");
        return;
    }
    for (const auto &t : transactions) {
        appendOutput(QString::fromStdString(t.toString()));
    }
}

void QtUI::onDailyReport() {
    QString date = dateEdit_->text().trimmed();
    if (date.isEmpty()) {
        date = QString::fromStdString(ExchangeService::currentDate());
    }

    ReportSummary summary = service_.generateDailySummary(date.toStdString());
    appendOutput("=== Daily report for " + date + " ===");
    appendOutput("Transactions: " + QString::number(summary.dayTxCount));
    appendOutput("Profit (LOC): " + QString::number(summary.dayProfit, 'f', 6));
    appendOutput("Month transactions: " + QString::number(summary.monthTxCount));
    appendOutput("Month profit (LOC): " + QString::number(summary.monthProfit, 'f', 6));
    appendOutput("Cashier bonus: " + QString::number(summary.cashierBonus, 'f', 6));
    appendOutput("Net profit: " + QString::number(summary.monthNetProfit, 'f', 6));
}

void QtUI::onMonthlyReport() {
    QString month = monthEdit_->text().trimmed();
    if (month.size() != 7) {
        appendOutput("Invalid month format. Use YYYY-MM.");
        return;
    }

    ReportSummary summary = service_.generateMonthlySummary(month.toStdString());
    appendOutput("=== Monthly report for " + month + " ===");
    appendOutput("Month transactions: " + QString::number(summary.monthTxCount));
    appendOutput("Month profit (LOC): " + QString::number(summary.monthProfit, 'f', 6));
    appendOutput("Cashier bonus: " + QString::number(summary.cashierBonus, 'f', 6));
    appendOutput("Net profit: " + QString::number(summary.monthNetProfit, 'f', 6));
}
