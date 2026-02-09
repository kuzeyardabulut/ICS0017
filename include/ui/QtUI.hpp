#pragma once

#include <QWidget>
#include "logic/ExchangeService.hpp"

class QLineEdit;
class QCheckBox;
class QTextEdit;
class QPushButton;

class QtUI : public QWidget {
    Q_OBJECT

public:
    explicit QtUI(ExchangeService &service, QWidget *parent = nullptr);

private slots:
    void onExecuteExchange();
    void onListTransactions();
    void onDailyReport();
    void onMonthlyReport();

private:
    ExchangeService &service_;

    QLineEdit *fromCodeEdit_;
    QLineEdit *toCodeEdit_;
    QLineEdit *amountEdit_;
    QCheckBox *partialCheck_;
    QLineEdit *partialAmountEdit_;
    QLineEdit *dateEdit_;
    QLineEdit *monthEdit_;
    QTextEdit *output_;

    void appendOutput(const QString &text);
};
