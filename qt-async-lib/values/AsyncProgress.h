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

#ifndef ASYNC_PROGRESS_H
#define ASYNC_PROGRESS_H

#include <QObject>
#include <QReadWriteLock>

enum class ASYNC_CAN_REQUEST_STOP
{
    YES,
    NO
};

class AsyncProgress
{
    Q_DISABLE_COPY(AsyncProgress)

public:
    AsyncProgress(QString message, ASYNC_CAN_REQUEST_STOP canRequestStop)
        : m_message(std::move(message)),
          m_canRequestStop(canRequestStop)
    {}

    ~AsyncProgress()
    {
#ifdef QT_DEBUG
        Q_ASSERT(!m_isInUse && "Progress is still used.");
#endif
    }

    QString message() const { QReadLocker locker(&m_lock); return m_message; }
    float progress() const { QReadLocker locker(&m_lock); return m_progress; }
    bool canRequestStop() const { QReadLocker locker(&m_lock); return m_canRequestStop == ASYNC_CAN_REQUEST_STOP::YES; }
    bool isStopRequested() const { QReadLocker locker(&m_lock); return m_isStopRequested; }

    void setMessage(QString message) { QWriteLocker locker(&m_lock); m_message = std::move(message); }
    void setProgress(float progress) { QWriteLocker locker(&m_lock); m_progress = progress; }
    template <typename Num>
    void setProgress(Num current, Num total)
    {
        if (total != 0)
            setProgress(static_cast<float>(current) / static_cast<float>(total));
    }
    void requestStop() { QWriteLocker locker(&m_lock); m_isStopRequested = true; }

#ifdef QT_DEBUG
    bool isInUse() const { QReadLocker locker(&m_lock); return m_isInUse; }
    void setInUse(bool inUse) { QWriteLocker locker(&m_lock); m_isInUse = inUse; }
#endif

protected:
    mutable QReadWriteLock m_lock;

    QString m_message;
    float m_progress = 0.f;
    ASYNC_CAN_REQUEST_STOP m_canRequestStop = ASYNC_CAN_REQUEST_STOP::YES;
    bool m_isStopRequested = false;

#ifdef QT_DEBUG
    bool m_isInUse = false;
#endif
};

class AsyncProgressRerun : public AsyncProgress
{
    Q_DISABLE_COPY(AsyncProgressRerun)

public:
    using AsyncProgress::AsyncProgress;

    ~AsyncProgressRerun()
    {
        Q_ASSERT(!m_isRerunRequested && "Rerun had been requested but not resolved");
    }

    bool isRerunRequested() const
    {
        QReadLocker locker(&m_lock);
        return m_isRerunRequested;
    }

    void requestRerun()
    {
        QWriteLocker locker(&m_lock);

        m_isRerunRequested = true;
        m_isStopRequested = true;
    }

    bool resetIfRerunRequested()
    {
        QWriteLocker locker(&m_lock);

        if (!m_isRerunRequested)
            return false;

        m_isStopRequested = false;
        m_isRerunRequested = false;

        return true;
    }

protected:
    bool m_isRerunRequested = false;
};

#endif // ASYNC_PROGRESS_H
