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

#ifndef ASYNC_CONFIG_H
#define ASYNC_CONFIG_H

#include <stdexcept>

#define ASYNC_PROGRESS_WIDGET_UPDATE_TIMEOUT 200

#define ASYNC_TRACK_DEADLOCK_AS_EXCEPTION

#ifdef ASYNC_TRACK_DEADLOCK_AS_ASSERT
    #define ASYNC_TRACK_DEADLOCK(condition) Q_ASSERT(condition)
#elif defined(ASYNC_TRACK_DEADLOCK_AS_EXCEPTION)
    #define ASYNC_TRACK_DEADLOCK(condition) async_impl::CheckDeadlock(condition)
#else
    #define ASYNC_TRACK_DEADLOCK(condition) /* no op */
#endif

namespace async_impl
{
    inline void CheckDeadlock(bool notDeadlock)
    {
        if (!notDeadlock)
            throw std::logic_error("Deadlock");
    }
}

#endif // ASYNC_CONFIG_H
