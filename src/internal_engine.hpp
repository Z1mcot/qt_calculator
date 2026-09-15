#pragma once

#include "calc_engine.hpp"
#include <stdexcept>

class InternalEngine : public ICalcEngine {
public:
    double doIt(const CalcRequest& request) noexcept(false) override {
        switch (request.op) {
        case CalcOp::Add:
            return request.a + request.b;
        case CalcOp::Subtract:
            return request.a - request.b;
        case CalcOp::Multiply:
            return request.a * request.b;
        case CalcOp::Divide:
            if (request.b == 0.0)
                throw std::logic_error("Division by zero");
            return request.a / request.b;
        default:
            throw std::logic_error("Unknown operation");
        }
    }
};
