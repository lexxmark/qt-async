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

class AsyncProgress
{
    Q_DISABLE_COPY(AsyncProgress)

public:
    AsyncProgress(QString message, bool canRequestStop)
        : m_message(std::move(message)),
          m_canRequestStop(canRequestStop)
    {}

    QString message() const { return m_message; }
    float progress() const { return m_progress; }
    bool canRequestStop() const { return m_canRequestStop; }
    bool isStopRequested() const { return m_isStopRequested; }

    void setMessage(QString message) { m_message = std::move(message); }
    void setProgress(float progress) { m_progress = progress; }
    void requestStop() { m_isStopRequested = true; }

private:
    QString m_message;
    float m_progress = 0.f;
    bool m_canRequestStop = true;
    bool m_isStopRequested = false;
};

#endif // ASYNC_PROGRESS_H
