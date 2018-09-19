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

#ifndef ASYNC_VALUE_OBTAIN_H
#define ASYNC_VALUE_OBTAIN_H

#include <QThreadPool>
#include <QtConcurrent>

template <typename AsyncValueType, typename Func, typename... ProgressArgs>
bool asyncValueObtain(QThreadPool *pool, AsyncValueType& value, Func&& func, ProgressArgs&& ...progressArgs)
{
    auto progress = value.startProgressEmplace(std::forward<ProgressArgs>(progressArgs)...);
    if (!progress)
        return false;

    QtConcurrent::run(pool, [&value, progress = progress, func = std::forward<Func>(func)](){
        func(*progress, value);
        value.stopProgress(progress);
    });

    return true;
}

template <typename AsyncValueType, typename Func, typename... ProgressArgs>
bool asyncValueObtain(AsyncValueType& value, Func&& func, ProgressArgs&& ...progressArgs)
{
    return asyncValueObtain(QThreadPool::globalInstance(), value, std::forward<Func>(func), std::forward<ProgressArgs>(progressArgs)...);
}

#endif // ASYNC_VALUE_OBTAIN_H
