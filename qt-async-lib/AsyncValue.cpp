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

#include "AsyncValue.h"
#include <QMetaType>

static auto async_value_state_type_id = qRegisterMetaType<ASYNC_VALUE_STATE>("ASYNC_VALUE_STATE");

AsyncValueBase::AsyncValueBase(QObject *parent)
    : QObject(parent)
{
}

AsyncProgress::AsyncProgress(QString message, bool canRequestStop)
    : m_message(std::move(message)),
      m_canRequestStop(canRequestStop)
{
}

QString AsyncProgress::message() const
{
    return m_message;
}

float AsyncProgress::progress() const
{
    return m_progress;
}

bool AsyncProgress::canRequestStop() const
{
    return m_canRequestStop;
}

bool AsyncProgress::isStopRequested() const
{
    return m_isStopRequested;
}

void AsyncProgress::setMessage(QString message)
{
    m_message = message;
}

void AsyncProgress::setProgress(float progress)
{
    m_progress = progress;
}

void AsyncProgress::requestStop()
{
    m_isStopRequested = true;
}

