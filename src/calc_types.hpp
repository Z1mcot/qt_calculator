#pragma once

#include <QChar>
#include <QString>
#include <stdexcept>

enum class CalcOp {
    Add,
    Subtract,
    Multiply,
    Divide
};

struct CalcRequest {
    qint64 id = 0;
    CalcOp op = CalcOp::Add;
    double a = 0.0;
    double b = 0.0;
    QString expression;
    bool isExpression = false;

    char getOpName() const {
        switch (op) {
        case CalcOp::Add: return '+';
        case CalcOp::Subtract: return '-';
        case CalcOp::Multiply: return '*';
        case CalcOp::Divide: return '/';
        default:
            throw std::logic_error("Unresolved operation name");
        }
    }
};

struct CalcResult {
    qint64 id = 0;
    bool ok = false;
    double value = 0.0;
    QString error;
    QString exprText;
};
