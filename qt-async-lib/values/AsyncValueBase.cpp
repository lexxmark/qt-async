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

#include "AsyncValueBase.h"
#include <QMetaType>

static auto async_value_state_type_id = qRegisterMetaType<ASYNC_VALUE_STATE>("ASYNC_VALUE_STATE");

AsyncValueBase::AsyncValueBase(QObject *parent)
    : QObject(parent)
{
}

