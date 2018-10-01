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
        Q_ASSERT(!m_isInUse && "Progress is still used.");
    }

    QString message() const { return m_message; }
    float progress() const { return m_progress; }
    bool canRequestStop() const { return m_canRequestStop == ASYNC_CAN_REQUEST_STOP::YES; }
    bool isStopRequested() const { return m_isStopRequested; }

    void setMessage(QString message) { m_message = std::move(message); }
    void setProgress(float progress) { m_progress = progress; }
    template <typename Num>
    void setProgress(Num current, Num total) { setProgress(static_cast<float>(current) / static_cast<float>(total)); }
    void requestStop() { m_isStopRequested = true; }

    bool isInUse() const { return m_isInUse; }
    void setInUse(bool inUse) { m_isInUse = inUse; }

private:
    QString m_message;
    float m_progress = 0.f;
    ASYNC_CAN_REQUEST_STOP m_canRequestStop = ASYNC_CAN_REQUEST_STOP::YES;
    bool m_isStopRequested = false;
    bool m_isInUse = false;
};

#endif // ASYNC_PROGRESS_H
