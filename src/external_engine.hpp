#pragma once

#include "calc_engine.hpp"
#include <QLibrary>

// Loads and calls:  double DoIt(int TypeWork, double OperandA, double OperandB) noexcept(false)
// from an external shared library (.so on Linux, .dll on Windows).
// TypeWork is passed as the ASCII code of the operator character ('+','-','*','/').
class ExternalEngine : public ICalcEngine {
public:
    explicit ExternalEngine(const QString& libraryPath);

    bool isLoaded() const { return doItFunc_ != nullptr; }
    QString lastError() const { return library_.errorString(); }

public:
    double doIt(const CalcRequest& request) noexcept(false) override;

private:
    using DoItFunc = double (*)(int, double, double);

    QLibrary library_;
    DoItFunc doItFunc_ = nullptr;
};
