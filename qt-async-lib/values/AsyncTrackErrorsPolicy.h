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

#ifndef ASYNC_TRACK_ERRORS_POLICY_H
#define ASYNC_TRACK_ERRORS_POLICY_H

#include <QThread>
#include <stdexcept>

struct AsyncTrackErrorsPolicyNone
{
    struct EmitGuard
    {
        EmitGuard(AsyncTrackErrorsPolicyNone&) {}
    };

    using EmitGuardType = EmitGuard;

    void trackEmitDeadlock() const {}
    void inProgressWhileDestruct() const {}
    void startProgressWhileInProgress() const {}
    void tryCompleteAlienProgress() const {}
    void incompleteProgress() const {}
};

struct AsyncTrackErrorsPolicyDefault
{
    struct EmitGuard
    {
        EmitGuard(AsyncTrackErrorsPolicyDefault& owner)
            : m_owner(owner)
        {
            Q_ASSERT(!m_owner.m_emitThread && "Cannot emit while emitting");
            // save emitting thread
            m_owner.m_emitThread = QThread::currentThread();
        }

        ~EmitGuard()
        {
            // reset emitting thread
            m_owner.m_emitThread = nullptr;
         }

    private:
        AsyncTrackErrorsPolicyDefault& m_owner;
    };

    using EmitGuardType = EmitGuard;
    void trackEmitDeadlock() const
    {
        if (m_emitThread == QThread::currentThread())
            throw std::logic_error("Async value deadlock");
    }

    void inProgressWhileDestruct() const
    {
        Q_ASSERT(false && "Destructing value while it's in progress");
    }

    void startProgressWhileInProgress() const
    {
        Q_ASSERT(false && "Cannot start new progress while in progress");
    }

    void tryCompleteAlienProgress() const
    {
        Q_ASSERT(false && "Started progress differs with the progress passed to completeProgress");
    }

    [[noreturn]] void incompleteProgress() const
    {
        throw std::logic_error("Neither value no error was assigned at the progress stop");
    }

private:
    QThread* m_emitThread = nullptr;
};

#endif // ASYNC_TRACK_ERRORS_POLICY_H
