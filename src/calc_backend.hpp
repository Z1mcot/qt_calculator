#pragma once

#include "calc_types.hpp"
#include "external_engine.hpp"
#include "internal_engine.hpp"
#include "thread_queue.hpp"
#include "calc_worker.hpp"
#include <QObject>
#include <QTimer>
#include <memory>


class CalcBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(int requestQueueSize READ requestQueueSize NOTIFY queueSizesChanged)
    Q_PROPERTY(int resultQueueSize READ resultQueueSize NOTIFY queueSizesChanged)
    Q_PROPERTY(bool useExternalEngine READ useExternalEngine WRITE setUseExternalEngine NOTIFY engineChanged)
    Q_PROPERTY(bool externalEngineAvailable READ externalEngineAvailable CONSTANT)
    Q_PROPERTY(int calcDelaySeconds READ calcDelaySeconds WRITE setCalcDelaySeconds NOTIFY delayChanged)

public:
    explicit CalcBackend(QObject* parent = nullptr);
    ~CalcBackend() override;

    int requestQueueSize() const { return static_cast<int>(requestQueue_.size()); }
    int resultQueueSize() const { return static_cast<int>(resultQueue_.size()); }

    bool useExternalEngine() const { return useExternal_; }
    void setUseExternalEngine(bool value);

    bool externalEngineAvailable() const { return externalEngine_ && externalEngine_->isLoaded(); }

    int calcDelaySeconds() const { return delaySeconds_; }
    void setCalcDelaySeconds(int value);

    // Запускает вычисление выражения "a op b"
    Q_INVOKABLE void submitRequest(const QString& opStr, double a, double b);

    // Разбирает выражение вида "10 / 2 * 5" слева направо без приоритета операций
    Q_INVOKABLE void submitExpression(const QString& expression);

    // Если на вход пришло невалидное выражение
    Q_INVOKABLE void reportInputError(const QString& message);

signals:
    void consoleMessage(const QString& text, const QString& color);
    void queueSizesChanged();
    void engineChanged();
    void delayChanged();

private slots:
    void drainResults();

private:
    ThreadSafeQueue<CalcRequest> requestQueue_;
    ThreadSafeQueue<CalcResult> resultQueue_;

    InternalEngine internalEngine_;
    std::unique_ptr<ExternalEngine> externalEngine_;
    bool useExternal_ = false;

    int delaySeconds_ = 0;
    qint64 nextId_ = 1;

    CalcWorker* worker_;
    QThread* workerThread_;
    QTimer pollTimer_;

    void updateWorkerEngine();
};
