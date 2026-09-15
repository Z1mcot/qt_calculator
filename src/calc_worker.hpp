#pragma once

#include "calc_engine.hpp"
#include "calc_types.hpp"
#include "thread_queue.hpp"
#include <QThread>
#include <atomic>
#include <queue>

class CalcWorker : public QObject {
    Q_OBJECT
public:
    CalcWorker(ThreadSafeQueue<CalcRequest>& requests, ThreadSafeQueue<CalcResult>& results,
                 QObject* parent = nullptr);

    void setDelaySeconds(int seconds) { delaySeconds_.store(seconds); }
    void setEngine(ICalcEngine* engine) { engine_.store(engine); }

signals:
    void resultReady();
    void finished();

public slots:
    /// Важное замечание тут бесконечный цикл, и event loop рабочего потока фактически не крутится
    /// Т.е. если дернуть слот stop() через сигнал, то он не отработает
    void runCalculations();
    void stop();

private:
    ThreadSafeQueue<CalcRequest>& requests_;
    ThreadSafeQueue<CalcResult>& results_;
    std::atomic<bool> running_{true};
    std::atomic<int> delaySeconds_{0};
    std::atomic<ICalcEngine *> engine_{nullptr};

    std::queue<CalcRequest> subOperationQueue_;

    bool parseExpressionToSubOperations(const QString& expression, QString& error);
    CalcResult processExpressionRequest(const CalcRequest& request);
};
