#include "external_engine.hpp"
#include <stdexcept>

ExternalEngine::ExternalEngine(const QString& libraryPath) : library_(libraryPath) {
    if (library_.load()) {
        doItFunc_ = reinterpret_cast<DoItFunc>(library_.resolve("DoIt"));
    }
}

double ExternalEngine::doIt(const CalcRequest& request) {
    if (!doItFunc_)
        throw std::logic_error("External library is not loaded: " + library_.errorString().toStdString());

    try {
        return doItFunc_(static_cast<int>(request.getOpName()), request.a, request.b);
    } catch (const std::exception& e) {
        throw std::logic_error(std::string("External library error: ") + e.what());
    } catch (...) {
        throw std::logic_error("External library threw an unrecognized exception "
                               "(possible ABI/compiler mismatch)");
    }
}
