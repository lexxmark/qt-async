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

#ifndef ASYNC_WIDGET_PROGRESS_H
#define ASYNC_WIDGET_PROGRESS_H

#include <QFrame>
#include "values/AsyncProgress.h"

class QLabel;
class QProgressBar;
class QPushButton;
class QTimeLine;

class AsyncWidgetProgress : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(AsyncWidgetProgress)

public:
    explicit AsyncWidgetProgress(AsyncProgress& progress, QWidget* parent);

private slots:
    void onStopClicked(bool checked);

private:
    void update();

    AsyncProgress& m_progress;

    QLabel* m_message = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QPushButton* m_stop = nullptr;
    QTimeLine* m_progressBarTimeLine = nullptr;
};

#endif // ASYNC_WIDGET_PROGRESS_H
