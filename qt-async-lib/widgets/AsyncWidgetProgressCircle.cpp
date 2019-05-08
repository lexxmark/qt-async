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

#include "AsyncWidgetProgressCircle.h"
#include "../Config.h"
#include "../third_party/QtProgressCircle/ProgressCircle.h"
#include <QTimer>

AsyncWidgetProgressCircle::AsyncWidgetProgressCircle(AsyncProgress& progress, QWidget* parent)
    : QFrame(parent),
      m_progress(progress)
{
    m_progressCircle = new ProgressCircle(this);
    m_progressCircle->setMaximum(100);
    m_progressCircle->setValue(0);

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &AsyncWidgetProgressCircle::updateContent);
    timer->start(ASYNC_PROGRESS_WIDGET_UPDATE_TIMEOUT);

    updateContent();
}

AsyncWidgetProgressCircle::~AsyncWidgetProgressCircle()
{
}

void AsyncWidgetProgressCircle::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    // center progress circle widget
    auto circleRect = rect();
    circleRect.setTop(circleRect.top() + circleRect.height() / 3);
    circleRect.setBottom(circleRect.bottom() - circleRect.height() / 3);
    circleRect.setLeft(circleRect.left() + circleRect.width() / 3);
    circleRect.setRight(circleRect.right() - circleRect.width() / 3);
    m_progressCircle->setGeometry(circleRect);
}

void AsyncWidgetProgressCircle::updateContent()
{
    m_progressCircle->setValue(static_cast<int>(m_progress.progress()*100.f));
    if (m_progress.isStopRequested())
        m_progressCircle->setColor(QColor(255, 128, 64));
}
