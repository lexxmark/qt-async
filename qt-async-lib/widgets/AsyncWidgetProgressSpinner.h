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

#ifndef ASYNC_WIDGET_PROGRESS_SPINNER_H
#define ASYNC_WIDGET_PROGRESS_SPINNER_H

#include <QFrame>
#include "values/AsyncProgress.h"

class WaitingSpinnerWidget;

class AsyncWidgetProgressSpinner : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(AsyncWidgetProgressSpinner)

public:
    explicit AsyncWidgetProgressSpinner(AsyncProgress& progress, QWidget* parent);
    ~AsyncWidgetProgressSpinner();

private:
    void updateContent();

    AsyncProgress& m_progress;

    WaitingSpinnerWidget* m_spinner = nullptr;
};

#endif // ASYNC_WIDGET_PROGRESS_SPINNER_H
