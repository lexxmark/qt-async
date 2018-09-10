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

#ifndef ASYNC_WIDGET_H
#define ASYNC_WIDGET_H

#include "values/AsyncValue.h"
#include "AsyncWidgetBase.h"

template <typename ValueType>
class AsyncWidget : public AsyncWidgetBase<AsyncValue<ValueType>>
{
public:
    using AsyncWidgetBase<AsyncValue<ValueType>>::AsyncWidgetBase;

protected:
    QSharedPointer<QWidget> createValueWidgetImpl(ValueType& /*value*/, QWidget* parent) override
    {
        return createLabel("<value>", parent);
    }

    QSharedPointer<QWidget> createErrorWidgetImpl(AsyncError& error, QWidget* parent) override
    {
        return createLabel(error.text(), parent);
    }

    QSharedPointer<QWidget> createProgressWidgetImpl(AsyncProgress& progress, QWidget* parent) override
    {
        return createLabel(progress.message(), parent);
    }
};

#endif // ASYNC_WIDGET_H
