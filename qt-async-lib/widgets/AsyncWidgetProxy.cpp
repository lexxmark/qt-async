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

#include "AsyncWidgetProxy.h"
#include <QLabel>
#include <QResizeEvent>

void AsyncWidgetProxy::setContentWidget(QWidget* content)
{
    if (m_content == content)
        return;

    auto oldContent = m_content;
    if (oldContent)
        oldContent->hide();

    m_content = content;

    if (m_content)
    {
        m_content->setParent(this);
        m_content->setGeometry(rect());
        m_content->show();
    }

    if (oldContent)
        delete oldContent;
}

void AsyncWidgetProxy::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (m_content)
        m_content->setGeometry(QRect(QPoint(0, 0), event->size()));
}

QWidget* AsyncWidgetProxy::createLabel(QString text, QWidget* parent)
{
    auto label = new QLabel(text, parent);
    label->setAutoFillBackground(true);
    label->setFrameStyle(QFrame::Panel | QFrame::Plain);
    label->setLineWidth(1);
    label->setWordWrap(true);
    auto _palette = label->palette();
    _palette.setColor(QPalette::Normal, QPalette::Window, Qt::white);
    label->setPalette(_palette);
    label->setAlignment(Qt::AlignCenter);

    return label;
}

