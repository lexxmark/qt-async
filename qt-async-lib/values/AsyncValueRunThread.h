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

#ifndef ASYNC_VALUE_RUN_THREAD_H
#define ASYNC_VALUE_RUN_THREAD_H

#include <QThread>
#include "../third_party/scope_exit.h"

template <typename AsyncValueType, typename Func, typename... ProgressArgs>
bool asyncValueRunThread(AsyncValueType& value, Func&& func, ProgressArgs&& ...progressArgs)
{
    auto progress = std::make_unique<typename AsyncValueType::ProgressType>(std::forward<ProgressArgs>(progressArgs)...);
    auto progressPtr = progress.get();

    if (!value.startProgress(std::move(progress)))
        return false;

    auto thread = QThread::create([&value, progressPtr, func = std::forward<Func>(func)]() {
        SCOPE_EXIT {
            // finish progress
            value.completeProgress(progressPtr);
        };

        // run calculation
        func(*progressPtr, value);
    });

    if (!thread)
        return false;

    // delete thread on complete
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    thread->start();

    return true;
}

#endif // ASYNC_VALUE_RUN_THREAD_H
