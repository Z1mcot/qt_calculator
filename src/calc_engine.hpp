#pragma once

#include "calc_types.hpp"
#include <QChar>

// Интерфейс для внутреннего движка и внешней библиотеки
class ICalcEngine {
public:
    virtual ~ICalcEngine() = default;
    virtual double doIt(const CalcRequest& typeWork) noexcept(false) = 0;
};
