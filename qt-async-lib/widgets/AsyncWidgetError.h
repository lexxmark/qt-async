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

#ifndef ASYNC_WIDGET_ERROR_H
#define ASYNC_WIDGET_ERROR_H

#include <QLabel>
#include "values/AsyncError.h"

class AsyncWidgetError : public QLabel
{
    Q_OBJECT
    Q_DISABLE_COPY(AsyncWidgetError)

public:
    explicit AsyncWidgetError(const AsyncError& error, QWidget* parent);

private:
    const AsyncError& m_error;
};

#endif // ASYNC_WIDGET_ERROR_H
