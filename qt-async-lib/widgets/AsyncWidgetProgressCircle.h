/*
   Copyright (c) 2019 Alex Zhondin <lexxmark.dev@gmail.com>

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

#ifndef ASYNC_WIDGET_PROGRESS_CIRCLE_H
#define ASYNC_WIDGET_PROGRESS_CIRCLE_H

#include <QFrame>
#include "values/AsyncProgress.h"

class ProgressCircle;
class QResizeEvent;

class AsyncWidgetProgressCircle : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(AsyncWidgetProgressCircle)

public:
    explicit AsyncWidgetProgressCircle(AsyncProgress& progress, QWidget* parent);
    ~AsyncWidgetProgressCircle();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateContent();

    AsyncProgress& m_progress;

    ProgressCircle* m_progressCircle = nullptr;
};

#endif // ASYNC_WIDGET_PROGRESS_CIRCLE_H
