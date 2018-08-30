/*
   Copyright (c) 2018 Alex Zhondin <lexxmark.dev@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef ASYNC_VALUE_H
#define ASYNC_VALUE_H

#include <QMutex>
#include <QObject>
#include <QReadWriteLock>
#include <QThreadPool>
#include <QtConcurrent>

enum class ASYNC_VALUE_STATE
{
    VALUE,
    PROGRESS,
    ERROR
};
Q_DECLARE_METATYPE(ASYNC_VALUE_STATE);

class AsyncError
{
    Q_DISABLE_COPY(AsyncError)

public:
    AsyncError(QString message)
        : m_message(std::move(message))
    {}

    QString message() const { return m_message; }

private:
    QString m_message;
};

class AsyncProgress
{
    Q_DISABLE_COPY(AsyncProgress)

public:
    AsyncProgress(QString message, bool canRequestStop);

    QString message() const;
    float progress() const;
    bool canRequestStop() const;
    bool isStopRequested() const;

    void setMessage(QString message);
    void setProgress(float progress);
    void requestStop();

private:
    QString m_message;
    float m_progress = 0.f;
    bool m_canRequestStop = true;
    bool m_isStopRequested = false;
};

class AsyncValueBase : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AsyncValueBase)

public:
    AsyncValueBase(QObject* parent = nullptr);

signals:
    void stateChanged(ASYNC_VALUE_STATE state);
};

template <typename T>
class AsyncValue : public AsyncValueBase
{
public:
    explicit AsyncValue(QObject* parent = nullptr)
        : AsyncValue(ASYNC_VALUE_STATE::ERROR, parent)
    {
        setError("");
    }

    template <typename... Args>
    void emplaceValue(Args&& ...arguments)
    {
        moveValue(std::make_unique<T>(std::forward<Args>(arguments)...));
    }

    template <typename U>
    void setValue(U value)
    {
        moveValue(std::make_unique<T>(std::forward<U>(value)));
    }

    void moveValue(std::unique_ptr<T> value)
    {
        Content oldContent;

        QMutexLocker writeLocker(&m_writeLock);
        {
            QWriteLocker locker(&m_contentLock);

            oldContent = std::move(m_content);
            m_content.value = std::move(value);
            m_state = ASYNC_VALUE_STATE::VALUE;
        }

        emitStateChanged();
    }

    void setError(QString message)
    {
        Content oldContent;

        QMutexLocker writeLocker(&m_writeLock);
        {
            QWriteLocker locker(&m_contentLock);

            oldContent = std::move(m_content);
            m_content.error = std::make_unique<AsyncError>(std::move(message));
            m_state = ASYNC_VALUE_STATE::ERROR;
        }

        emitStateChanged();
    }

    AsyncProgress* startProgress(QString message, bool canRequestStop)
    {
        Content oldContent;

        QMutexLocker writeLocker(&m_writeLock);

        if (m_state == ASYNC_VALUE_STATE::PROGRESS)
        {
            Q_ASSERT(false && "Cannot start progress while in progress");
            return nullptr;
        }

        {
            QWriteLocker locker(&m_contentLock);

            oldContent = std::move(m_content);
            m_progress = std::make_unique<AsyncProgress>(std::move(message), canRequestStop);
            m_state = ASYNC_VALUE_STATE::PROGRESS;
        }

        emitStateChanged();

        return m_progress.get();
    }

    bool stopProgress(AsyncProgress* progress = nullptr)
    {
        QMutexLocker writeLocker(&m_writeLock);

        if (progress && (progress != m_progress.get()))
            return false;

        if (m_state == ASYNC_VALUE_STATE::PROGRESS)
            setError("No value or error assigned");

        return true;
    }

    template <typename ValuePred, typename ProgressPred, typename ErrorPred>
    void access(ValuePred valuePred, ProgressPred progressPred, ErrorPred errorPred)
    {
        QReadLocker locker(&m_contentLock);

        switch (m_state)
        {
        case ASYNC_VALUE_STATE::VALUE:
            valuePred(*m_content.value);
            break;

        case ASYNC_VALUE_STATE::PROGRESS:
            progressPred(*m_progress);
            break;

        case ASYNC_VALUE_STATE::ERROR:
            errorPred(*m_content.error);
            break;
        }
    }

    template <typename Pred>
    bool accessValue(Pred valuePred)
    {
        QReadLocker locker(&m_contentLock);

        if (m_state != ASYNC_VALUE_STATE::VALUE)
            return false;

        valuePred(*m_content.value);
        return true;
     }

    template <typename Pred>
    bool accessProgress(Pred progressPred)
    {
        QReadLocker locker(&m_contentLock);

        if (m_state != ASYNC_VALUE_STATE::PROGRESS)
            return false;

        progressPred(*m_progress);
        return true;
     }

    template <typename Pred>
    bool accessError(Pred errorPred)
    {
        QReadLocker locker(&m_contentLock);

        if (m_state != ASYNC_VALUE_STATE::ERROR)
            return false;

        errorPred(*m_content.error);
        return true;
     }

private:
    explicit AsyncValue(ASYNC_VALUE_STATE state, QObject* parent = nullptr)
        : AsyncValueBase(parent),
          m_writeLock(QMutex::Recursive),
          m_contentLock(QReadWriteLock::Recursive),
          m_state(state)
    {
    }

    void emitStateChanged()
    {
        emit stateChanged(m_state);
    }

    QMutex m_writeLock;
    QReadWriteLock m_contentLock;
    ASYNC_VALUE_STATE m_state;

    struct Content
    {
        std::unique_ptr<T> value;
        std::unique_ptr<AsyncError> error;
    };
    Content m_content;
    std::unique_ptr<AsyncProgress> m_progress;
};

template <typename T, typename Func>
void aquireAsyncValue(QThreadPool *pool, AsyncValue<T>& value, QString progressMessage, bool canRequestStop, Func func)
{
    auto progress = value.startProgress(std::move(progressMessage), canRequestStop);
    if (!progress)
        return;

    QtConcurrent::run(pool, [&value, progress = progress, func = std::move(func)](){
        func(*progress, value);
        value.stopProgress();
    });
}

#endif // ASYNC_VALUE_H
