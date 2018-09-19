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

#include "AsyncWidgetError.h"

AsyncWidgetError::AsyncWidgetError(const AsyncError& error, QWidget* parent)
    : QLabel(parent),
      m_error(error)
{
    setAutoFillBackground(true);
    setFrameStyle(QFrame::Panel | QFrame::Plain);
    setLineWidth(1);
    setWordWrap(true);
    auto _palette = palette();
    _palette.setColor(QPalette::Normal, QPalette::Window, Qt::white);
    setPalette(_palette);
    setAlignment(Qt::AlignCenter);

    setText(m_error.text());
}
