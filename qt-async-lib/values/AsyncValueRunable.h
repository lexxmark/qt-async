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

#ifndef ASYNC_VALUE_RUNABLE_H
#define ASYNC_VALUE_RUNABLE_H

#include "AsyncValueTemplate.h"
#include "AsyncError.h"
#include "AsyncProgress.h"
#include <functional>

template <typename ValueType_t, typename ErrorType_t = AsyncError, typename ProgressType_t = AsyncProgressRerun, typename TrackErrorsPolicy_t = AsyncTrackErrorsPolicyDefault>
class AsyncValueRunableAbstract : public AsyncValueTemplate<ValueType_t, ErrorType_t, ProgressType_t, TrackErrorsPolicy_t>
{
public:
    using ValueType = ValueType_t;
    using ErrorType = ErrorType_t;
    using ProgressType = ProgressType_t;
    using ThisType = AsyncValueRunableAbstract<ValueType_t, ErrorType_t, ProgressType_t, TrackErrorsPolicy_t>;
    using BaseType = AsyncValueTemplate<ValueType_t, ErrorType_t, ProgressType_t, TrackErrorsPolicy_t>;
    using RunFnType = std::function<void(ProgressType&, ThisType&)>;

    // constructors
    using BaseType::BaseType;

    void run()
    {
        bool isInProgress = BaseType::accessProgress([](ProgressType& progress) {
            // if we are in progress already -> just request rerun
            progress.requestRerun();
        });

        if (isInProgress)
            return;

        // run later
        deferImpl([this] (ProgressType& progress, ThisType&) {

            for (;;)
            {
                // try to calculate value
                runImpl(progress);
                // if no rerun was requested -> we good to exit
                if (!progress.resetIfRerunRequested())
                    break;
            }

        });
    }

protected:
    virtual void deferImpl(RunFnType&& func) = 0;
    virtual void runImpl(ProgressType& progress) = 0;
};


template <typename ValueType_t, typename ErrorType_t = AsyncError, typename ProgressType_t = AsyncProgressRerun, typename TrackErrorsPolicy_t = AsyncTrackErrorsPolicyDefault>
class AsyncValueRunableFn : public AsyncValueTemplate<ValueType_t, ErrorType_t, ProgressType_t, TrackErrorsPolicy_t>
{
public:
    using ValueType = ValueType_t;
    using ErrorType = ErrorType_t;
    using ProgressType = ProgressType_t;
    using ThisType = AsyncValueRunableFn<ValueType_t, ErrorType_t, ProgressType_t, TrackErrorsPolicy_t>;
    using BaseType = AsyncValueTemplate<ValueType_t, ErrorType_t, ProgressType_t, TrackErrorsPolicy_t>;
    using RunFnType = std::function<void(ProgressType&, ThisType&)>;
    using DeferFnType = std::function<void(const RunFnType&)>;

    // constructors
    using BaseType::BaseType;

    DeferFnType deferFn;
    RunFnType runFn;

    void run()
    {
        bool isInProgress = BaseType::accessProgress([](ProgressType& progress) {
            // if we are in progress already -> just request rerun
            progress.requestRerun();
        });

        if (isInProgress)
            return;

        // run later
        deferFn([this] (ProgressType& progress, ThisType& value) {

            for (;;)
            {
                // try to calculate value
                runFn(progress, value);
                // if no rerun was requested -> we good to exit
                if (!progress.resetIfRerunRequested())
                    break;
            }

        });
    }
};

#endif // ASYNC_VALUE_RUNABLE_H
