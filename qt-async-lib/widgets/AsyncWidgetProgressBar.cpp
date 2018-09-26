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

#include "AsyncWidgetProgressBar.h"
#include "../Config.h"
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QTimeLine>

AsyncWidgetProgressBar::AsyncWidgetProgressBar(AsyncProgress& progress, QWidget* parent)
    : QFrame(parent),
      m_progress(progress)
{
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(11, 11, 11, 11);

    // top spacer
    {
        auto spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        layout->addItem(spacer);
    }

    // content
    {
        // message
        {
            m_message = new QLabel(this);
            layout->addWidget(m_message);
        }

        {
            auto subLayout = new QHBoxLayout(this);
            subLayout->setSpacing(6);

            // progress
            {
                m_progressBar = new QProgressBar(this);
                m_progressBar->setTextVisible(false);
                m_progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                subLayout->addWidget(m_progressBar);

                m_progressBarTimeLine = new QTimeLine(300, this);
                QObject::connect(m_progressBarTimeLine, &QTimeLine::frameChanged, m_progressBar, &QProgressBar::setValue);
            }

            // stop button
            {
                m_stop = new QPushButton(this);
                m_stop->setText("Stop");
                subLayout->addWidget(m_stop);
                QObject::connect(m_stop, &QPushButton::clicked, this, &AsyncWidgetProgressBar::onStopClicked);
            }

            layout->addLayout(subLayout);
        }
    }

    // bottom spacer
    {
        auto spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        layout->addItem(spacer);
    }

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &AsyncWidgetProgressBar::updateContent);
    timer->start(ASYNC_PROGRESS_WIDGET_UPDATE_TIMEOUT);

    updateContent();
}

void AsyncWidgetProgressBar::onStopClicked(bool /*checked*/)
{
    m_progress.requestStop();
}

void AsyncWidgetProgressBar::updateContent()
{
    if (m_progress.isStopRequested())
        m_message->setText(m_progress.message()+"(Stopping...)");
    else
        m_message->setText(m_progress.message());

    m_stop->setVisible(m_progress.canRequestStop());

    m_progressBarTimeLine->stop();
    m_progressBarTimeLine->setFrameRange(m_progressBar->value(), int(m_progress.progress() * 100.f));
    m_progressBarTimeLine->start();
}
