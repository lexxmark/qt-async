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

#include <functional>
#include "values/AsyncValue.h"
#include "AsyncWidgetBase.h"
#include "AsyncWidgetError.h"
#include "AsyncWidgetProgressBar.h"

template <typename AsyncValueType>
class AsyncWidget : public AsyncWidgetBase<AsyncValueType>
{
public:
    using ValueType = typename AsyncValueType::ValueType;
    using ErrorType = typename AsyncValueType::ErrorType;
    using ProgressType = typename AsyncValueType::ProgressType;

    using AsyncWidgetBase<AsyncValueType>::AsyncWidgetBase;

protected:
    QWidget* createValueWidgetImpl(ValueType& /*value*/, QWidget* parent) override
    {
        return this->createLabel("<value widget is not implemented>", parent);
    }

    QWidget* createErrorWidgetImpl(ErrorType& error, QWidget* parent) override
    {
        return new AsyncWidgetError(error, parent);
    }

    QWidget* createProgressWidgetImpl(ProgressType& progress, QWidget* parent) override
    {
        return new AsyncWidgetProgressBar(progress, parent);
    }
};

template <typename AsyncValueType>
class AsyncWidgetFn : public AsyncWidget<AsyncValueType>
{
public:
    using ValueType = typename AsyncValueType::ValueType;
    using ErrorType = typename AsyncValueType::ErrorType;
    using ProgressType = typename AsyncValueType::ProgressType;

    using AsyncWidget<AsyncValueType>::AsyncWidget;

    std::function<QWidget*(ValueType&, QWidget*)> createValueWidget;
    std::function<QWidget*(ErrorType&, QWidget*)> createErrorWidget;
    std::function<QWidget*(ProgressType&, QWidget*)> createProgressWidget;

protected:
    QWidget* createValueWidgetImpl(ValueType& value, QWidget* parent) override
    {
        if (createValueWidget)
            return createValueWidget(value, parent);
        else
            return AsyncWidget<AsyncValueType>::createValueWidgetImpl(value, parent);
    }

    QWidget* createErrorWidgetImpl(ErrorType& error, QWidget* parent) override
    {
        if (createErrorWidget)
            return createErrorWidget(error, parent);
        else
            return AsyncWidget<AsyncValueType>::createErrorWidgetImpl(error, parent);
    }

    QWidget* createProgressWidgetImpl(ProgressType& progress, QWidget* parent) override
    {
        if (createProgressWidget)
            return createProgressWidget(progress, parent);
        else
            return AsyncWidget<AsyncValueType>::createProgressWidgetImpl(progress, parent);
    }
};

#endif // ASYNC_WIDGET_H
