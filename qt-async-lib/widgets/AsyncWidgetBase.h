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

#ifndef ASYNC_WIDGET_BASE_H
#define ASYNC_WIDGET_BASE_H

#include "AsyncWidgetProxy.h"
#include "values/AsyncValueBase.h"

template <typename AsyncValueType>
class AsyncWidgetBase : public AsyncWidgetProxy
{
public:
    using AsyncWidgetProxy::AsyncWidgetProxy;

    void setValue(AsyncValueType* asyncValue)
    {
        if (m_asyncValue == asyncValue)
            return;

        if (m_asyncValue)
            QObject::disconnect(m_asyncValue, &AsyncValueBase::stateChanged, this, &AsyncWidgetBase::onValueStateChanged);
        setContentWidget(nullptr);

        m_asyncValue = asyncValue;
        if (m_asyncValue)
            QObject::connect(m_asyncValue, &AsyncValueBase::stateChanged, this, &AsyncWidgetBase::onValueStateChanged);
        updateContent();
    }

protected:
    using ValueType = typename AsyncValueType::ValueType;
    using ErrorType = typename AsyncValueType::ErrorType;
    using ProgressType = typename AsyncValueType::ProgressType;

    virtual QWidget* createValueWidgetImpl(ValueType& value, QWidget* parent) = 0;
    virtual QWidget* createErrorWidgetImpl(ErrorType& error, QWidget* parent) = 0;
    virtual QWidget* createProgressWidgetImpl(ProgressType& progress, QWidget* parent) = 0;
    virtual QWidget* createNoAsyncValueWidgetImpl(QWidget* parent) { return createLabel("<no value>", parent); }

private:
    void onValueStateChanged(ASYNC_VALUE_STATE /*state*/)
    {
        updateContent();
    }

    void updateContent()
    {
        if (!m_asyncValue)
        {
            setContentWidget(createNoAsyncValueWidgetImpl(this));
            return;
        }

        QWidget* newWidget = nullptr;

        m_asyncValue->access([&newWidget, this](ValueType& value){
            newWidget = createValueWidgetImpl(value, this);
        }, [&newWidget, this](ErrorType& error){
            newWidget = createErrorWidgetImpl(error, this);
        }, [&newWidget, this](ProgressType& progress){
            newWidget = createProgressWidgetImpl(progress, this);
        });

        Q_ASSERT(newWidget);
        if (!newWidget)
            newWidget = createLabel("<no widget>", this);

        setContentWidget(newWidget);
    }

    AsyncValueType* m_asyncValue = nullptr;
};

#endif // ASYNC_WIDGET_BASE_H
