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

#include "AsyncValue.h"
#include <QWidget>

class WidgetProxy : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(WidgetProxy)

public:
    WidgetProxy(QWidget* parent = nullptr);

    QWidget* content() const { return m_content.data(); }
    void setContent(QSharedPointer<QWidget> content);

protected:
    void resizeEvent(QResizeEvent *event) override;

    static QSharedPointer<QWidget> createLabel(QString text, QWidget* parent);

private:
   QSharedPointer<QWidget> m_content;
};

template <typename T>
class AsyncValueWidgetBase : public WidgetProxy
{
public:
    AsyncValueWidgetBase(QWidget* parent = nullptr)
        : WidgetProxy(parent)
    {
    }

    void setValue(AsyncValue<T>* value)
    {
        if (m_value == value)
            return;

        if (m_value)
            QObject::disconnect(m_value, &AsyncValueBase::stateChanged, this, &AsyncValueWidgetBase::onValueStateChanged);
        setContent(nullptr);

        m_value = value;
        if (m_value)
            QObject::connect(m_value, &AsyncValueBase::stateChanged, this, &AsyncValueWidgetBase::onValueStateChanged);
        updateContent();
    }

protected:
    virtual QSharedPointer<QWidget> createValueContentImpl(T& value, QWidget* parent) = 0;
    virtual QSharedPointer<QWidget> createProgressContentImpl(AsyncProgress& progress, QWidget* parent) = 0;
    virtual QSharedPointer<QWidget> createErrorContentImpl(AsyncError& error, QWidget* parent) = 0;
    virtual QSharedPointer<QWidget> createNoValueContentImpl(QWidget* parent) { return createLabel("<no value>", parent); }

private:
    void onValueStateChanged(ASYNC_VALUE_STATE /*state*/)
    {
        updateContent();
    }

    void updateContent()
    {
        if (!m_value)
        {
            setContent(createNoValueContentImpl(this));
            return;
        }

        QSharedPointer<QWidget> newContent;

        m_value->access([&newContent, this](T& value){
            newContent = createValueContentImpl(value, this);
        }, [&newContent, this](AsyncProgress& progress){
            newContent = createProgressContentImpl(progress, this);
        }, [&newContent, this](AsyncError& error){
            newContent = createErrorContentImpl(error, this);
        });

        Q_ASSERT(newContent);
        if (!newContent)
            newContent = createLabel("<no content>", this);

        setContent(newContent);
    }

    AsyncValue<T>* m_value = nullptr;
};

template <typename T>
class AsyncValueWidget : public AsyncValueWidgetBase<T>
{
public:
    using AsyncValueWidgetBase<T>::AsyncValueWidgetBase;

protected:
    QSharedPointer<QWidget> createValueContentImpl(T& /*value*/, QWidget* parent) override
    {
        return createLabel("<value>", parent);
    }
    QSharedPointer<QWidget> createProgressContentImpl(AsyncProgress& progress, QWidget* parent) override
    {
        return createLabel(progress.message(), parent);
    }
    QSharedPointer<QWidget> createErrorContentImpl(AsyncError& error, QWidget* parent) override
    {
        return createLabel(error.message(), parent);
    }
};

#endif // ASYNC_WIDGET_H
