This library has some usefull Qt classes and widgets to show asyncronious operations.

[![qt-async demo video](https://img.youtube.com/vi/aTXOpmVRXq0/maxresdefault.jpg)](https://youtu.be/aTXOpmVRXq0)

# Basic use

Declare async value object:
```C++
using AsyncQString = AsyncValue<QString>;
AsyncQString value(AsyncInitByValue{}, "Hello World!");
```

When you need to calculate value, call one of the *asyncXXX* functions:
```C++
    bool success = asyncValueRunThreadPool(value, [](AsyncProgress& progress, AsyncQString& value) {

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
        auto valueWidget = new AsyncWidgetFn<AsyncQString>(parent);
        
        // set callback that creates widget to show value
        valueWidget->createValueWidget = [](QString& value, QWidget* parent) {
            // create QLabel
            return AsyncWidgetProxy::createLabel(value, parent);
        };

        // assign async value with widget
        valueWidget->setValue(&value);
```

Instead of callbacks you can derive widget class from `AsyncWidgetBase` or `AsyncWidget` classes and override 4 functions:
  
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

# AsyncValue API
Most of the `AsyncValue` functions can be found in `AsyncValueTemplate<...>` base class.

User can initialize `AsyncValue` using value or error, `AsyncInitByValue` and `AsyncInitByError` tag classes are used to distinguish two cases:
```C++
    using AsyncInt = AsyncValue<int>;
    
    // init by value
    AsyncInt value(AsyncInitByValue{}, 12);
    // init by parent and value
    auto value = new AsyncInt(parent, AsyncInitByValue{}, 12);
    // init by error
    AsyncInt value(AsyncInitByError{}, "No int available");
    // init by parent and error
    auto value = new AsyncInt(parent, AsyncInitByError{}, "No int available");
```

There is `stateChanged` signal that is emitted when async value's state changes between value, error and progress.

To get the content of the async value use `access` functions and supply callables to access either value or error or progress:
```C++
    // get any content of the async value
    value.access([](int value) { /* access int value here */ },
                 [](AsyncError& error) { /* access error here */ },
                 [](AsyncProgress& progress) { /* access progress here */ });
                 
   // get value of the async value
   bool success = value.accessValue([](int value) { /* access int value here */ });
```

User can assign value using following functions:
```C++
    AsyncValue<std::string> value(...);
    
    // create value by passing value's constructor parameters
    value.emplaceValue(5, 'b');
    // or create value and pass it to async value
    auto str = std::make_unique<std::string>(5, 'b');
    value.moveValue(std::move(str));
```
User can assign error in a similar way:
```C++
    AsyncValue<std::string> value(...);
    
    // create error by passing error's constructor parameters
    value.emplaceError("Some error hapenned");
    // or create error and pass it to async value
    auto err = std::make_unique<AsyncError>("Some error hapenned");
    value.moveError(std::move(err));
```
`startProgress` and `completeProgress` functions are used by *asyncXXX* functions to start and finish progress:
```C++
    template <typename AsyncValueType, typename Func, typename... ProgressArgs>
    bool asyncValueRunThreadPool(QThreadPool *pool, AsyncValueType& value, Func&& func, ProgressArgs&& ...progressArgs)
    {
        // create progress object
        auto progress = std::make_unique<typename AsyncValueType::ProgressType>(std::forward<ProgressArgs>(progressArgs)...);
        // try to start progress
        if (!value.startProgress(progress.get()))
            return false;

        QtConcurrent::run(pool, [&value, progressPtr = progress.release(), func = std::forward<Func>(func)](){
            // make unique_ptr from raw ptr
            std::unique_ptr<typename AsyncValueType::ProgressType> progress(progressPtr);
            // run calculation
            func(*progress, value);
            // post progress stuff
            value.completeProgress(progress.get());
        });

        return true;
    }
```

Also user has an ability to wait async value for result:
```C++
    AsyncValue<int> value(...);
    ...
    value.wait([](int value) { /* access int value here */ },
               [](AsyncError& error) { /* access error here */ });
```

# Runnable values
Usually it's more convinient to hide details how value is calculated.

`AsyncValueRunableAbstract` and `AsyncValueRunableFn` classes are used in this case.
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
This does more than just calls value calculation in async manner. If previous calculation is not completed yet it tries to stop and rerun calculation. It not blocks calling thread. Here is a code of the run function:
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
                // if no rerun was requested -> we are good to exit
                if (!progress.resetIfRerunRequested())
                    break;
            }

        });
    }
```
The code is quite straightforward.

User can use the same widgets to show runnable values in GUI:
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
All async value classes are inherited from `AsyncValueTemplate` template class:
```C++
template <typename ValueType_t, typename ErrorType_t, typename ProgressType_t, typename TrackErrorsPolicy_t>
class AsyncValueTemplate : public AsyncValueBase
{
    ...
};
```
So users can override all type parameters to better adopt async values.

By default `ErrorType_t` parameter is a `AsyncError` class:
```C++
class AsyncError
{
public:
    AsyncError(QString text)
        : m_text(std::move(text))
    {}

    QString text() const { return m_text; }

private:
    QString m_text;
};
```
The `ProgressType_t` parameter represented by `AsyncProgress` with following functions:
```C++
    // the text describing the current progress
    QString message() const;
    // current progress position in [0, 1]
    float progress() const;
    // returns true if progress can handle stop requests
    bool canRequestStop() const;
    // returns true if progress was requested to stop
    bool isStopRequested() const;

    // sets text describing the current progress
    void setMessage(QString message);
    // sets current progress position in [0, 1]
    void setProgress(float progress);
    // sets current position using current step and total number of steps
    template <typename Num>
    void setProgress(Num current, Num total)
    {
        if (total != 0)
            setProgress(static_cast<float>(current) / static_cast<float>(total));
    }
    // requests stop of the current progress
    void requestStop();
```

