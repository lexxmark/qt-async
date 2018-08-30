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

#include "AsyncWidget.h"
#include <QLabel>
#include <QResizeEvent>

WidgetProxy::WidgetProxy(QWidget* parent)
    : QWidget (parent)
{
}

void WidgetProxy::setContent(QSharedPointer<QWidget> content)
{
    if (m_content == content)
        return;

    m_content = std::move(content);

    if (m_content)
    {
        m_content->setParent(this);
        m_content->setGeometry(rect());
        m_content->show();
    }
}

void WidgetProxy::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (m_content)
        m_content->setGeometry(QRect(QPoint(0, 0), event->size()));
}

QSharedPointer<QWidget> WidgetProxy::createLabel(QString text, QWidget* parent)
{
    auto label = QSharedPointer<QLabel>::create(text, parent);
    label->setFrameStyle(QFrame::Panel | QFrame::Plain);
    label->setLineWidth(2);
    return label;
}

