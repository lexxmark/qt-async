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

#ifndef ASYNC_VALUE_TEMPLATE_H
#define ASYNC_VALUE_TEMPLATE_H

#include <memory>
#include "AsyncValueBase.h"
#include "AsyncTrackErrorsPolicy.h"

struct AsyncNoOp
{
    template <typename T>
    void operator()(const T&) const {}
};

struct AsyncInitByValue {};
struct AsyncInitByError {};


template <typename ValueType_t, typename ErrorType_t, typename ProgressType_t, typename TrackErrorsPolicy_t = AsyncTrackErrorsPolicyDefault>
class AsyncValueTemplate : public AsyncValueBase
{
public:
    using ValueType = ValueType_t;
    using ErrorType = ErrorType_t;
    using ProgressType = ProgressType_t;

    template <typename... Args>
    explicit AsyncValueTemplate(QObject* parent, AsyncInitByValue, Args&& ...arguments)
        : AsyncValueBase(ASYNC_VALUE_STATE::VALUE, parent)
    {
        emplaceValue(std::forward<Args>(arguments)...);
    }

    template <typename... Args>
    explicit AsyncValueTemplate(AsyncInitByValue, Args&& ...arguments)
        : AsyncValueBase(ASYNC_VALUE_STATE::VALUE, nullptr)
    {
        emplaceValue(std::forward<Args>(arguments)...);
    }

    ~AsyncValueTemplate()
    {
        if (m_state == ASYNC_VALUE_STATE::PROGRESS)
            m_trackErrors.inProgressWhileDestruct();
    }

    template <typename... Args>
    void emplaceValue(Args&& ...arguments)
    {
        moveValue(std::make_unique<ValueType>(std::forward<Args>(arguments)...));
    }

    void moveValue(std::unique_ptr<ValueType> value)
    {
        m_trackErrors.trackEmitDeadlock();

        Content oldContent;

        QMutexLocker writeLocker(&m_writeLock);
        {
            QWriteLocker locker(&m_contentLock);

            oldContent = std::move(m_content);
            m_content.value = std::move(value);

            // don't change state until stopProgress happen
            if (m_state == ASYNC_VALUE_STATE::PROGRESS)
                return;

            m_state = ASYNC_VALUE_STATE::VALUE;
        }

        emitStateChanged();

        // notify all waiters
        if (m_waiter)
            m_waiter->waitValue.wakeAll();
    }

    template <typename... Args>
    explicit AsyncValueTemplate(QObject* parent, AsyncInitByError, Args&& ...arguments)
        : AsyncValueBase(ASYNC_VALUE_STATE::ERROR, parent)
    {
        emplaceError(std::forward<Args>(arguments)...);
    }

    template <typename... Args>
    explicit AsyncValueTemplate(AsyncInitByError, Args&& ...arguments)
        : AsyncValueBase(ASYNC_VALUE_STATE::ERROR, nullptr)
    {
        emplaceError(std::forward<Args>(arguments)...);
    }

    template <typename... Args>
    void emplaceError(Args&& ...arguments)
    {
        moveError(std::make_unique<ErrorType>(std::forward<Args>(arguments)...));
    }

    void moveError(std::unique_ptr<ErrorType> error)
    {
        m_trackErrors.trackEmitDeadlock();

        Content oldContent;

        QMutexLocker writeLocker(&m_writeLock);
        {
            QWriteLocker locker(&m_contentLock);

            oldContent = std::move(m_content);
            m_content.error = std::move(error);

            // don't change state until stopProgress happen
            if (m_state == ASYNC_VALUE_STATE::PROGRESS)
                return;

            m_state = ASYNC_VALUE_STATE::ERROR;
        }

        emitStateChanged();

        // notify all waiters
        if (m_waiter)
            m_waiter->waitValue.wakeAll();
    }

    bool startProgress(std::unique_ptr<ProgressType> progress)
    {
        Q_ASSERT(progress);

        m_trackErrors.trackEmitDeadlock();

        Content oldContent;

        QMutexLocker writeLocker(&m_writeLock);

        if (m_state == ASYNC_VALUE_STATE::PROGRESS)
        {
            m_trackErrors.startProgressWhileInProgress();
            return false;
        }

        {
            QWriteLocker locker(&m_contentLock);

            oldContent = std::move(m_content);
            m_progress = std::move(progress);
            m_state = ASYNC_VALUE_STATE::PROGRESS;

#ifdef QT_DEBUG
            Q_ASSERT(!m_progress->isInUse() && "Progress is used already");
            m_progress->setInUse(true);
#endif
        }

        emitStateChanged();

        return true;
    }

    bool completeProgress(ProgressType* progress)
    {
#ifdef QT_DEBUG
        Q_ASSERT(progress);
        Q_ASSERT(progress->isInUse() && "Progress should be used");
        progress->setInUse(false);
#endif

        QMutexLocker writeLocker(&m_writeLock);

        if (progress != m_progress.get())
        {
            m_trackErrors.tryCompleteAlienProgress();
            return false;
        }

        {
            QWriteLocker locker(&m_contentLock);

            if (m_content.value)
                m_state = ASYNC_VALUE_STATE::VALUE;
            else if (m_content.error)
                m_state = ASYNC_VALUE_STATE::ERROR;
            else
            {
                m_trackErrors.incompleteProgress();
                return false;
            }

            m_progress = nullptr;
        }

        emitStateChanged();

        // notify all waiters
        if (m_waiter)
            m_waiter->waitValue.wakeAll();

        return true;
    }

    template <typename ValuePred, typename ErrorPred, typename ProgressPred>
    void access(ValuePred valuePred, ErrorPred errorPred, ProgressPred progressPred)
    {
        QReadLocker locker(&m_contentLock);

        switch (m_state)
        {
        case ASYNC_VALUE_STATE::VALUE:
            valuePred(*m_content.value);
            break;

        case ASYNC_VALUE_STATE::ERROR:
            errorPred(*m_content.error);
            break;

        case ASYNC_VALUE_STATE::PROGRESS:
            progressPred(*m_progress);
            break;
        }
    }

    template <typename ValuePred, typename ErrorPred>
    bool access(ValuePred valuePred, ErrorPred errorPred)
    {
        QReadLocker locker(&m_contentLock);

        switch (m_state)
        {
        case ASYNC_VALUE_STATE::VALUE:
            valuePred(*m_content.value);
            return true;

        case ASYNC_VALUE_STATE::ERROR:
            errorPred(*m_content.error);
            return true;

        default:
            return false;
        }
    }

    template <typename Pred>
    bool access(Pred valuePred)
    {
        QReadLocker locker(&m_contentLock);

        if (m_state != ASYNC_VALUE_STATE::VALUE)
            return false;

        valuePred(*m_content.value);
        return true;
     }

    template <typename Pred>
    bool accessValue(Pred valuePred)
    {
        return access(valuePred);
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

    template <typename Pred>
    bool accessProgress(Pred progressPred)
    {
        QReadLocker locker(&m_contentLock);

        if (m_state != ASYNC_VALUE_STATE::PROGRESS)
            return false;

        progressPred(*m_progress);
        return true;
     }

    template <typename ValuePred, typename ErrorPred>
    void wait(ValuePred valuePred, ErrorPred errorPred)
    {
        // easy case we have value or error
        if (access(valuePred, errorPred))
            return;

        // lock async value
        QMutexLocker writeLocker(&m_writeLock);
        // check easy case again
        if (access(valuePred, errorPred))
            return;

        // if we are the first waiters
        // create Waiter on stack and assign it to m_waiter
        // all subsequent waiters will use this one
        if (!m_waiter)
        {
            Waiter theWaiter;

            SCOPE_EXIT {
                if (m_waiter->subWaiters > 0)
                {
                    do
                    {
                        // repeatedly wait for all sub waiters
                        // because of spurious wakeups
                        m_waiter->waitSubWaiters.wait(&m_writeLock);
                    } while (m_waiter->subWaiters != 0);
                }

                // unregister self as main waiter
                m_waiter = nullptr;
            };

            // register self as main waiter
            m_waiter = &theWaiter;

            do
            {
                // repeatedly wait for value or error
                // because of spurious wakeups
                m_waiter->waitValue.wait(&m_writeLock);
            } while (!access(valuePred, errorPred));
        }
        else
        {
            SCOPE_EXIT {
                // unregister self as subwaiter
                m_waiter->subWaiters -= 1;
                // if no subwaiters -> notify main waiter to release
                if (m_waiter->subWaiters == 0)
                    m_waiter->waitSubWaiters.wakeAll();
            };

            // register self as subwaiter
            m_waiter->subWaiters += 1;

            do
            {
                // repeatedly wait for value or error
                // because of spurious wakeups
                m_waiter->waitValue.wait(&m_writeLock);
            } while (!access(valuePred, errorPred));
        }
    }

    void wait()
    {
        wait(AsyncNoOp(), AsyncNoOp());
    }

    void stopAndWait()
    {
        accessProgress([](ProgressType& progress){
            progress.requestStop();
        });
        wait();
    }

private:
    void emitStateChanged()
    {
        using EmitGuardType = typename TrackErrorsPolicy_t::EmitGuardType;
        EmitGuardType emitGuard(m_trackErrors);

        emit stateChanged(m_state);
    }

    struct Content
    {
        std::unique_ptr<ValueType> value;
        std::unique_ptr<ErrorType> error;
    };
    Content m_content;

    std::unique_ptr<ProgressType> m_progress;

    TrackErrorsPolicy_t m_trackErrors;
};

#endif // ASYNC_VALUE_TEMPLATE_H
