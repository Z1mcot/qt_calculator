#include "calc_backend.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QThread>

namespace {
constexpr int kPollIntervalMs = 100;

QString defaultExternalLibraryPath() {
#if defined(Q_OS_WIN)
    return QDir(QCoreApplication::applicationDirPath()).filePath("doit.dll");
#elif defined(Q_OS_APPLE)
    return QDir(QCoreApplication::applicationDirPath()).filePath("libdoit.dylib");
#else
    return QDir(QCoreApplication::applicationDirPath()).filePath("libdoit.so");
#endif
}
} // namespace

namespace {
bool isMathOperator(QChar ch) {
    return ch == QLatin1Char('+') || ch == QLatin1Char('-') ||
           ch == QLatin1Char('*') || ch == QLatin1Char('/');
}

bool parseNumber(const QString& text, int& pos, double& value) {
    const int start = pos;
    if (pos < text.size() && (text[pos] == QLatin1Char('+') || text[pos] == QLatin1Char('-'))) {
        ++pos;
    }

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
} // namespace

CalcBackend::CalcBackend(QObject* parent) : QObject(parent) {
    externalEngine_ = std::make_unique<ExternalEngine>(defaultExternalLibraryPath());
    if (!externalEngine_->isLoaded()) {
        emit consoleMessage(
            QStringLiteral("Внешняя библиотека не найдена (%1), доступен только внутренний движок")
                .arg(defaultExternalLibraryPath()),
            QStringLiteral("red"));
    }

    workerThread_ = new QThread(this);
    worker_ = new CalcWorker(requestQueue_, resultQueue_);
    worker_->moveToThread(workerThread_);
    updateWorkerEngine();
    worker_->setDelaySeconds(delaySeconds_);

    connect(workerThread_, &QThread::started, worker_, &CalcWorker::runCalculations);
    connect(worker_, &CalcWorker::resultReady, this, &CalcBackend::drainResults);
    connect(worker_, &CalcWorker::finished, workerThread_, &QThread::quit);
    connect(workerThread_, &QThread::finished, worker_, &QObject::deleteLater);

    workerThread_->start();

    connect(&pollTimer_, &QTimer::timeout, this, [this] { emit queueSizesChanged(); });
    pollTimer_.start(kPollIntervalMs);
}

CalcBackend::~CalcBackend() {
    if (workerThread_ && workerThread_->isRunning()) {
        if (worker_) {
            worker_->stop();
        }
        workerThread_->quit();
        workerThread_->wait();
    }

    if (worker_) {
        delete worker_;
        worker_ = nullptr;
    }
}

void CalcBackend::setUseExternalEngine(bool value) {
    if (value == useExternal_)
        return;

    if (value && !externalEngineAvailable()) {
        emit consoleMessage(QStringLiteral("Внешняя библиотека недоступна"), QStringLiteral("red"));
        return;
    }

    useExternal_ = value;
    updateWorkerEngine();
    emit engineChanged();
}

void CalcBackend::setCalcDelaySeconds(int value) {
    if (value == delaySeconds_)
        return;
    delaySeconds_ = value;
    worker_->setDelaySeconds(delaySeconds_);
    emit delayChanged();
}

void CalcBackend::updateWorkerEngine() {
    ICalcEngine* engine = useExternal_ ? static_cast<ICalcEngine *>(externalEngine_.get())
                                        : static_cast<ICalcEngine *>(&internalEngine_);
    worker_->setEngine(engine);
}

void CalcBackend::submitRequest(const QString& opStr, double a, double b) {
    if (opStr.size() != 1 || QStringLiteral("+-*/").indexOf(opStr) < 0) {
        emit consoleMessage(QStringLiteral("Некорректная операция"), QStringLiteral("red"));
        return;
    }

    CalcRequest request;
    request.id = nextId_++;

    switch (opStr.at(0).unicode()) {
    case u'+':
        request.op = CalcOp::Add;
        break;
    case u'-':
        request.op = CalcOp::Subtract;
        break;
    case u'*':
        request.op = CalcOp::Multiply;
        break;
    case u'/':
        request.op = CalcOp::Divide;
        break;
    default:
        Q_ASSERT(false);
    }

    request.a = a;
    request.b = b;

    const QString text = QStringLiteral("Запрос #%1: %2 %3 %4").arg(request.id).arg(a).arg(request.getOpName()).arg(b);
    requestQueue_.push(request);
    emit consoleMessage(text, QStringLiteral("green"));
    emit queueSizesChanged();
}

void CalcBackend::submitExpression(const QString& expression) {
    const QString trimmed = expression.trimmed();
    if (trimmed.isEmpty()) {
        reportInputError(QStringLiteral("Пустой запрос: выражение не задано"));
        return;
    }

    CalcRequest request;
    request.id = nextId_++;
    request.expression = trimmed;
    request.isExpression = true;

    requestQueue_.push(request);
    emit consoleMessage(QStringLiteral("Запрос #%1: %2").arg(request.id).arg(trimmed), QStringLiteral("green"));
    emit queueSizesChanged();
}

void CalcBackend::reportInputError(const QString &message) {
    emit consoleMessage(message, QStringLiteral("red"));
}

void CalcBackend::drainResults() {
    while (auto result = resultQueue_.tryPop()) {
        if (result->ok) {
            emit consoleMessage(
                QStringLiteral("Результат #%1: %2 = %3").arg(result->id).arg(result->exprText).arg(result->value),
                QStringLiteral("blue"));
        } else {
            emit consoleMessage(
                QStringLiteral("Ошибка #%1 (%2): %3").arg(result->id).arg(result->exprText, result->error),
                QStringLiteral("red"));
        }
    }
    emit queueSizesChanged();
}
