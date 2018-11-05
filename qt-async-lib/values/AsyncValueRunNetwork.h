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

#ifndef ASYNC_VALUE_RUN_NETWORK_H
#define ASYNC_VALUE_RUN_NETWORK_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "../third_party/scope_exit.h"

template <typename AsyncValueType, typename Func, typename... ProgressArgs>
bool asyncValueRunNetwork(QNetworkReply* reply, AsyncValueType& value, Func&& func, ProgressArgs&& ...progressArgs)
{
    auto progress = std::make_unique<typename AsyncValueType::ProgressType>(std::forward<ProgressArgs>(progressArgs)...);
    auto progressPtr = progress.get();

    if (!value.startProgress(std::move(progress)))
        return false;

    // forward progress
    QObject::connect(reply, &QNetworkReply::downloadProgress, [progressPtr](qint64 bytesReceived, qint64 bytesTotal){
        progressPtr->setProgress(bytesReceived, bytesTotal);
    });

    // post processing
    QObject::connect(reply, &QNetworkReply::finished, [ reply,
                                                        &value,
                                                        progressPtr,
                                                        func = std::forward<Func>(func)](){
        SCOPE_EXIT {
            reply->deleteLater();
            // finish progress
            value.completeProgress(progressPtr);
        };

        func(*reply, value);
    });

    return true;
}

template <typename AsyncValueType, typename Func, typename... ProgressArgs>
bool asyncValueRunNetwork(QNetworkAccessManager* networkManager, const QNetworkRequest &request, AsyncValueType& value, Func&& func, ProgressArgs&& ...progressArgs)
{
    auto reply = networkManager->get(request);

    if (!reply)
        return false;

    return asyncValueRunNetwork(reply, value, std::forward<Func>(func), std::forward<ProgressArgs>(progressArgs)...);
}

#endif // ASYNC_VALUE_RUN_NETWORK_H
