#include "calc_worker.hpp"
#include <stdexcept>
#include <thread>

namespace {
bool isMathOperator(QChar ch) {
    return ch == QLatin1Char('+') || ch == QLatin1Char('-') ||
           ch == QLatin1Char('*') || ch == QLatin1Char('/');
}

bool parseNumber(const QString& text, int& pos, double& value) {
    const int start = pos;
    if (pos < text.size() && (text[pos] == QLatin1Char('+') || text[pos] == QLatin1Char('-')))
        ++pos;

    bool sawDigit = false;
    bool sawDot = false;
    while (pos < text.size()) {
        const QChar ch = text[pos];
        if (ch.isDigit()) {
            sawDigit = true;
            ++pos;
            continue;
        }
        if (ch == QLatin1Char('.') && !sawDot) {
            sawDot = true;
            ++pos;
            continue;
        }
        break;
    }

    if (!sawDigit)
        return false;

    const QString token = text.mid(start, pos - start);
    bool ok = false;
    value = token.toDouble(&ok);
    return ok;
}
}

CalcWorker::CalcWorker(ThreadSafeQueue<CalcRequest>& requests, ThreadSafeQueue<CalcResult>& results,
                            QObject* parent)
    : QObject(parent), requests_(requests), results_(results) {}

void CalcWorker::stop() {
    running_.store(false, std::memory_order_release);
}

bool CalcWorker::parseExpressionToSubOperations(const QString& expression, QString& error) {
    subOperationQueue_ = {};

    QString text = expression.trimmed();
    if (text.isEmpty()) {
        error = "Пустое выражение";
        return false;
    }

    int pos = 0;
    double currentValue = 0.0;
    if (!parseNumber(text, pos, currentValue)) {
        error = "Ожидалось число в начале выражения";
        return false;
    }

    while (pos < text.size()) {
        while (pos < text.size() && text[pos].isSpace())
            ++pos;

        if (pos >= text.size())
            break;

        if (!isMathOperator(text[pos])) {
            error = QStringLiteral("Некорректный оператор в позиции %1").arg(pos + 1);
            return false;
        }

        const QChar opCh = text[pos++];
        while (pos < text.size() && text[pos].isSpace())
            ++pos;

        double nextValue = 0.0;
        if (!parseNumber(text, pos, nextValue)) {
            error = QStringLiteral("Ожидалось число после оператора %1").arg(opCh);
            return false;
        }

        CalcRequest step;
        step.op = (opCh == QLatin1Char('+')) ? CalcOp::Add :
                  (opCh == QLatin1Char('-')) ? CalcOp::Subtract :
                  (opCh == QLatin1Char('*')) ? CalcOp::Multiply : CalcOp::Divide;
        step.a = currentValue;
        step.b = nextValue;
        subOperationQueue_.push(step);

        currentValue = nextValue;
    }

    if (subOperationQueue_.empty()) {
        error = "В выражении нет операций";
        return false;
    }

    return true;
}

CalcResult CalcWorker::processExpressionRequest(const CalcRequest& request) {
    CalcResult result;
    result.id = request.id;
    result.exprText = request.expression;

    QString error;
    if (!parseExpressionToSubOperations(request.expression, error)) {
        result.ok = false;
        result.error = error;
        return result;
    }

    ICalcEngine* engine = engine_.load(std::memory_order_acquire);
    try {
        if (!engine)
            throw std::logic_error("No calculation engine selected");

        double accumulator = 0.0;
        bool first = true;
        while (!subOperationQueue_.empty()) {
            CalcRequest step = subOperationQueue_.front();
            subOperationQueue_.pop();

            if (first) {
                accumulator = step.a;
                first = false;
            }

            step.a = accumulator;
            accumulator = engine->doIt(step);
        }

        result.value = accumulator;
        result.ok = true;
    } catch (const std::exception& e) {
        result.ok = false;
        result.error = QString::fromStdString(e.what());
    }

    return result;
}

void CalcWorker::runCalculations() {
    using namespace std::chrono_literals;

    while (running_.load(std::memory_order_acquire)) {
        auto maybeRequest = requests_.waitPop(200ms);
        if (!maybeRequest)
            continue;

        const auto request = maybeRequest.value();

        const int seconds = delaySeconds_.load(std::memory_order_acquire);
        const auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
        while (running_.load(std::memory_order_acquire) &&
               std::chrono::steady_clock::now() < endTime) {
            std::this_thread::sleep_for(10ms);
        }

        CalcResult result;
        if (request.isExpression) {
            result = processExpressionRequest(request);
        } else {
            result.id = request.id;
            result.exprText = QStringLiteral("%1 %2 %3")
                                   .arg(request.a)
                                   .arg(request.getOpName())
                                   .arg(request.b);

            ICalcEngine* engine = engine_.load(std::memory_order_acquire);
            try {
                if (!engine)
                    throw std::logic_error("No calculation engine selected");

                result.value = engine->doIt(request);
                result.ok = true;
            } catch (const std::exception& e) {
                result.ok = false;
                result.error = QString::fromStdString(e.what());
            }
        }

        results_.push(result);
        emit resultReady();
    }

    emit finished();
}
