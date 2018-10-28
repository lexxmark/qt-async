This library has some usefull Qt classes and widgets to show asyncronious operations.

[![qt-async demo video](https://img.youtube.com/vi/aTXOpmVRXq0/maxresdefault.jpg)](https://youtu.be/aTXOpmVRXq0)

# Basic use

Declare async value object:
```C++
using AsyncQString = AsyncValue<QString>;
AsyncQString value(AsyncInitByValue{}, "Hello World!");
```

When you need to calculate value, call one of the asyncXXX functions:
```C++
    asyncValueRunThreadPool(value, [](AsyncProgress& progress, AsyncQString& value) {

        // a long calculations
        for (auto i : {0, 1, 2, 3, 4})
        {
            // report progress
            progress.setProgress(i, 5);
            // check if calculation was stopped
            if (progress.isStopRequested())
            {
                // report error
                value.emplaceError("Stopped");
                return;
            }

            QThread::sleep(1);
        }

        progress.setProgress(1.f);
        progress.setMessage("Processing...");
        QThread::sleep(1);

        QString val = "Loaded value is 42";
        // set calculated value
        value.emplaceValue(std::move(val));

    }, "Loading...", ASYNC_CAN_REQUEST_STOP::YES);
```

Somewhere in GUI code declare async widget:
```C++
        // create widget
        auto valueWidget = new AsyncWidgetFn<AsyncQString>(ui->widget);
        
        // set callback that creates widget to show value
        valueWidget->createValueWidget = [](QString& value, QWidget* parent) {
            // create QLabel
            return AsyncWidgetProxy::createLabel(value, parent);
        };

        // assign value with widget
        valueWidget->setValue(&m_value);
```

Instead of callbacks you can derive widget class from AsyncWidgetBase or AsyncWidget classes and override four functions:
  
```C++
    // creates widget to show value
    virtual QWidget* createValueWidgetImpl(ValueType& value, QWidget* parent) = 0;
    
    // creates widget to show error
    virtual QWidget* createErrorWidgetImpl(ErrorType& error, QWidget* parent) = 0;
    
    // creates widget to show progress
    virtual QWidget* createProgressWidgetImpl(ProgressType& progress, QWidget* parent) = 0;
    
    // create widget when no value has been assigned
    virtual QWidget* createNoAsyncValueWidgetImpl(QWidget* parent) { return createLabel("<no value>", parent); }

```

# Runnable values
Usually it's more convinient to hide details how value is calculated. AsyncValueRunableAbstract and AsyncValueRunableFn classes are used in this case.
```C++
    using AsyncQPixmap = AsyncValueRunableFn<QPixmap>;
    AsyncQPixmap value(AsyncInitByError{}, "Select image file path.");
    
    // set callback to run async value calculation
    value.deferFn = [&value](const AsyncQPixmap::RunFnType& fn) {
        asyncValueRunThread(value, fn, "Loading image...", ASYNC_CAN_REQUEST_STOP::NO);
    };
    // set callback to calculate value
    value.runFn = [](AsyncProgressRerun& progress, AsyncQPixmap& value) {

        auto url = getImageUrl();
        QImage image(url);

        for (auto i : {0, 1, 2, 3})
        {
            if (progress.isRerunRequested())
            {
                // exit and retry load image with new url
                return ;
            }

            progress.setProgress(i, 4);

            // do some heavy work
            QThread::sleep(1);
        }

        if (image.isNull())
            value.emplaceError(QString("Cannot load image from file '%1'.").arg(url));
        else
            value.emplaceValue(QPixmap::fromImage(image));
    };
```
To calculate value user need to call run() method:
```C++
    value.run();
```
This does more than just calls value calculation in async manner. If previous calculation is not completed yet it tries to stop and rerun calculation non blocking calling thread. Here is code of the run function:
```C++
    void run()
    {
        bool isInProgress = accessProgress([](ProgressType& progress) {
            // if we are in progress already -> just request rerun
            progress.requestRerun();
        });

        if (isInProgress)
            return;

        // run later
        deferFn([this] (ProgressType& progress, ThisType& value) {

            for (;;)
            {
                // try to calculate value
                runFn(progress, value);
                // if no rerun was requested -> we good to exit
                if (!progress.resetIfRerunRequested())
                    break;
            }

        });
    }
```
The code is quite straightforward.

You can use the same widgets to show runnable values in GUI:
```C++
        auto valueWidget = new AsyncWidgetFn<AsyncQPixmap>(ui->widget);

        valueWidget->createValueWidget = [](QPixmap& value, QWidget* parent) {
            auto label = new QLabel(parent);
            label->setAlignment(Qt::AlignCenter);
            label->setPixmap(value);
            label->setStyleSheet("border: 1px solid black");
            return label;
        };

        valueWidget->setValue(&value);
```

# Customizations
All async value classes are inherited from AsyncValueTemplate template class:
```C++
template <typename ValueType_t, typename ErrorType_t, typename ProgressType_t, typename TrackErrorsPolicy_t>
class AsyncValueTemplate : public AsyncValueBase
{
    ...
};
```
So users can override all type parameters to better adopt async values to their environment.


