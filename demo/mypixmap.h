#ifndef MYPIXMAP_H
#define MYPIXMAP_H

#include "values/AsyncValueRunable.h"
#include "values/AsyncValueRunThread.h"
#include "widgets/AsyncWidget.h"
#include <QBitmap>

class MyPixmap : public AsyncValueRunableAbstract<QPixmap>
{
public:
    MyPixmap()
        : AsyncValueRunableAbstract<QPixmap>(AsyncInitByError{}, "Select image file path.")
    {
    }

    QString imageUrl() const
    {
        QReadLocker locker(&m_urlLock);
        return m_imageUrl;
    }

    void setImageUrl(QString url)
    {
        {
            QWriteLocker locker(&m_urlLock);
            m_imageUrl = url;
        }

        // reload image
        run();
    }

protected:
    // hide run from public
    using AsyncValueRunableAbstract<QPixmap>::run;

    void deferImpl(RunFnType&& func) final
    {
        asyncValueRunThread(*this, func, "Loading image...", ASYNC_CAN_REQUEST_STOP::NO);
    }

    void runImpl(ProgressType& progress) final
    {
        auto url = imageUrl();

        QImage image(url);

        for (auto i : {0, 1, 2, 3})
        {
            if (progress.isRerunRequested())
            {
                // exit and retry load image with new path
                return ;
            }

            progress.setProgress(i, 4);

            // do some heavy work
            QThread::sleep(1);
        }

        if (image.isNull())
            emplaceError(QString("Cannot load image from file '%1'.").arg(url));
        else
            emplaceValue(QPixmap::fromImage(image));
    }

private:
    mutable QReadWriteLock m_urlLock;
    QString m_imageUrl;
};

class MyPixmapWidget : public AsyncWidget<MyPixmap>
{
public:
    MyPixmapWidget(QWidget* parent, MyPixmap* value)
        : AsyncWidget<MyPixmap>(parent)
    {
        setValue(value);
    }

protected:
    QWidget* createValueWidgetImpl(ValueType& value, QWidget* parent) final
    {
        auto label = new QLabel(parent);
        label->setAlignment(Qt::AlignCenter);
        label->setPixmap(value);
        label->setStyleSheet("border: 1px solid black");
        return label;
    }
};

#endif // MYPIXMAP_H
