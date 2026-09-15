#include <stdexcept>

#if defined(_WIN32)
#define DOIT_EXPORT extern "C" __declspec(dllexport)
#else
#define DOIT_EXPORT extern "C" __attribute__((visibility("default")))
#endif

// TypeWork carries the ASCII code of the operator character: '+', '-', '*', '/'.
DOIT_EXPORT double DoIt(int TypeWork, double OperandA, double OperandB) noexcept(false) {
    switch (static_cast<char>(TypeWork)) {
    case '+':
        return OperandA + OperandB;
    case '-':
        return OperandA - OperandB;
    case '*':
        return OperandA * OperandB;
    case '/':
        if (OperandB == 0.0)
            throw std::logic_error("Division by zero");
        return OperandA / OperandB;
    default:
        throw std::logic_error("Unknown operation code");
    }
}
