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

#include "AsyncWidgetProgressSpinner.h"
#include "../Config.h"
#include "../third_party/QtWaitingSpinner/waitingspinnerwidget.h"
#include <QTimer>

AsyncWidgetProgressSpinner::AsyncWidgetProgressSpinner(AsyncProgress& progress, QWidget* parent)
    : QFrame(parent),
      m_progress(progress)
{
    m_spinner = new WaitingSpinnerWidget(this, true, false);
    m_spinner->start();

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &AsyncWidgetProgressSpinner::updateContent);
    timer->start(ASYNC_PROGRESS_WIDGET_UPDATE_TIMEOUT);

    updateContent();
}

AsyncWidgetProgressSpinner::~AsyncWidgetProgressSpinner()
{
}

void AsyncWidgetProgressSpinner::updateContent()
{
    m_spinner->setText(m_progress.message());
}
