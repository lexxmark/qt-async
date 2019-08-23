This library has some usefull Qt classes and widgets for asyncronious operations.

[![qt-async demo video](https://img.youtube.com/vi/aTXOpmVRXq0/maxresdefault.jpg)](https://youtu.be/aTXOpmVRXq0)

# Brief overview
The library introduces *async values*. Like std::future<T> *async value* holds the *result* of async operation or *error* (if operation was unsuccessfull).
Also it holds *progress* if async operation is still executing and has *stateChanged* signal that fires when async value is switching between progress, result and error.


Like `future<T> std::async(Callable fn)` qt-async library has a set of functions `bool asyncValueRunXXX(AsyncValue& value, Callable fn, ProgressArgs... args)` to make asyncronious calculation for the *value*.
All these functions switch *value* to a progress state (and pass *args* to progress constructor) and invoke `fn(progress, value)` so *fn* implementation can use *progress* to report executing progress or ask if execution was requested to stop.
*value* parameter is used to set a result of the operation or an error if calculation has failed.


To represent such *async values* in GUI qt-async library has *async widgets*. Every time *async value* state changes, *async widget* creates sub-widget for that state (ProgressWidget, ErrorWidget or ValueWidget).
Similar to *async value* where user should supply 'fn' that actually calculates value, user should supply to *async widget* a factory how to create ValueWidget (it can be overriden virtual function or std::function).

# Basic use

Declare async value object that holds values of type QString and initialized with string "Hello World!":
```C++
using AsyncQString = AsyncValue<QString>;
AsyncQString value(AsyncInitByValue{}, "Hello World!");
```

When you need to calculate value, call one of the `asyncValueRunXXX` functions:
```C++
    bool success = asyncValueRunThreadPool(value, [](AsyncProgress& progress, AsyncQString& value) {

        // a long calculations
        for (auto i : {0, 1, 2, 3, 4})
        {
            // report progress i/5
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

        // do final processing
        progress.setProgress(1.f);
        progress.setMessage("Processing...");
        QThread::sleep(1);

        QString val = "Loaded value is 42";
        // set calculated value
        value.emplaceValue(std::move(val));

    }, "Loading...", ASYNC_CAN_REQUEST_STOP::YES);
```
The available functions are:
* [asyncValueRunThread](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/values/AsyncValueRunThread.h#L23) - creates QThread, does calculations and deletes QThread (don't use this function)
* [asyncValueRunThreadPool](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/values/AsyncValueRunThreadPool.h#L24) - does calculation in a Qt thread pool
* [asyncValueRunNetwork](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/values/AsyncValueRunNetwork.h#L24) - waits QNetworkReply and does calculation from it.
See [runInThread](https://github.com/lexxmark/qt-async/blob/40af2b9e0a07f8d5cae1e62e039c36012b4234d0/tests/TestAsyncValue.cpp#L48) and [runInThreadPool](https://github.com/lexxmark/qt-async/blob/40af2b9e0a07f8d5cae1e62e039c36012b4234d0/tests/TestAsyncValue.cpp#L62) tests for examples.

Somewhere in GUI code declare async widget:
```C++
        // create widget
        auto valueWidget = new AsyncWidgetFn<AsyncQString>(parent);
        
        // set callback that creates sub-widget to show QString value
        valueWidget->createValueWidget = [](QString& value, QWidget* parent) {
            // create QLabel
            return AsyncWidgetProxy::createLabel(value, parent);
        };

        // assign async value with widget
        valueWidget->setValue(&value);
```

Instead of callbacks you can derive widget class from [AsyncWidgetBase](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/widgets/AsyncWidgetBase.h#L24) or [AsyncWidget](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/widgets/AsyncWidget.h#L27) classes and override 4 functions:
  
```C++
    // creates widget to show value
    virtual QWidget* createValueWidgetImpl(ValueType& value, QWidget* parent);
    
    // creates widget to show error
    virtual QWidget* createErrorWidgetImpl(ErrorType& error, QWidget* parent);
    
    // creates widget to show progress
    virtual QWidget* createProgressWidgetImpl(ProgressType& progress, QWidget* parent);
    
    // creates widget when no async value has been assigned to async widget
    virtual QWidget* createNoAsyncValueWidgetImpl(QWidget* parent);
```

# AsyncValue API
Most of the `AsyncValue` functions can be found in [AsyncValueTemplate<...>](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/values/AsyncValueTemplate.h#L35) base class.

User can initialize `AsyncValue` using value or error, `AsyncInitByValue` and `AsyncInitByError` tag classes are used to distinguish these two cases:
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

There is `stateChanged` signal that is emitted when async value's state changes between value, error and progress. NOTE: you cannot modify async value in it's stateChanged signal handler otherwise deadlock will happen.
```C++
    QObject::connect(value, &AsyncValueBase::stateChanged, [](ASYNC_VALUE_STATE state) {
        // async value state change handler
    });
```

To get the content of the async value use `access` functions and supply callables to access either value or error or progress:
```C++
    // get any content of the async value
    value.access([](int value) { /* access int value here */ },
                 [](AsyncError& error) { /* access error here */ },
                 [](AsyncProgress& progress) { /* access progress here */ });
                 
   // get value of the async value
   bool success = value.accessValue([](int value) { /* access int value here */ });
```

User can assign a value using the following functions:
```C++
    AsyncValue<std::string> value(...);
    
    // create value by passing value's constructor parameters
    value.emplaceValue(5, 'b');
	
    // or create value and pass it to async value
    auto str = std::make_unique<std::string>(5, 'b');
    value.moveValue(std::move(str));
```
User can assign an error in a similar way:
```C++
    AsyncValue<std::string> value(...);
    
    // create error by passing error's constructor parameters
    value.emplaceError("Some error hapenned");
	
    // or create error and pass it to async value
    auto err = std::make_unique<AsyncError>("Some error hapenned");
    value.moveError(std::move(err));
```
`startProgress` and `completeProgress` functions are used by `asyncValueRunXXX` functions to start and finish progress:
```C++
    template <typename AsyncValueType, typename Func, typename... ProgressArgs>
    bool asyncValueRunThreadPool(QThreadPool *pool, AsyncValueType& value, Func&& func, ProgressArgs&& ...progressArgs)
    {
        // create progress
        auto progress = std::make_unique<typename AsyncValueType::ProgressType>(std::forward<ProgressArgs>(progressArgs)...);
        auto progressPtr = progress.get();
        
        // try to switch async value to progress state
        if (!value.startProgress(std::move(progress)))
            return false;

        QtConcurrent::run(pool, [&value, progressPtr, func = std::forward<Func>(func)](){
            SCOPE_EXIT {
                // finish progress
                value.completeProgress(progressPtr);
            };

            // run calculation
            func(*progressPtr, value);
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

[AsyncValueRunableAbstract](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/values/AsyncValueRunable.h#L26) and [AsyncValueRunableFn](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/values/AsyncValueRunable.h#L71) classes are used in this case.
```C++
    using AsyncQPixmap = AsyncValueRunableFn<QPixmap>;
    AsyncQPixmap value(AsyncInitByError{}, "Select image file path.");
    
    // set callback to start async value calculation
    value.deferFn = [&value](const AsyncQPixmap::RunFnType& fn) {
        asyncValueRunThread(value, fn, "Loading image...", ASYNC_CAN_REQUEST_STOP::NO);
    };
    // set callback to calculate actual value
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
This does more than just calls value calculation in async manner. If previous calculation is not completed yet it tries to stop and rerun calculation. It doesn't block calling thread. Here is a code of the run function:
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
# Advanced example
In the [MyPixmap.h](https://github.com/lexxmark/qt-async/blob/master/demo/mypixmap.h) file you can find a complete example how to adopt async values and widgets for your needs.

Let's say you have some class to load and store QPixmap. Loading image from external url should be done asynchronously. Here we just inherit our class from `AsyncValueRunableAbstract<QPixmap>` class and override `deferImpl` and `runImpl` functions. When image url has changed we call `run` to perform image loading in a separate thread:
```C++
class MyPixmap : public AsyncValueRunableAbstract<QPixmap>
{
public:
    // init pixmap with an error
    MyPixmap()
        : AsyncValueRunableAbstract<QPixmap>(AsyncInitByError{}, "Select image file path.")
    {
    }

    // returns umage url
    // threadsafe
    QString imageUrl() const
    {
        QReadLocker locker(&m_urlLock);
        return m_imageUrl;
    }

    // sets new image url and requests image loading
    // threadsafe
    void setImageUrl(QString url)
    {
        // change image url
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

    // actual image loading will be performed in a separate thread
    void deferImpl(RunFnType&& func) final
    {
        asyncValueRunThread(*this, func, "Loading image...", ASYNC_CAN_REQUEST_STOP::NO);
    }

    // image loading code
    void runImpl(ProgressType& progress) final
    {
        auto url = imageUrl();

        QImage image(url);

        for (auto i : {0, 1, 2, 3})
        {
            if (progress.isRerunRequested())
            {
                // exit and retry load image with a new path
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
```
The widget for `MyPixmap` class could be implemented like this:
```C++
class MyPixmapWidget : public AsyncWidget<MyPixmap>
{
public:
    MyPixmapWidget(QWidget* parent, MyPixmap* value)
        : AsyncWidget<MyPixmap>(parent)
    {
        setValue(value);
    }

protected:
    // creates QLabel to show QPixmap image
    QWidget* createValueWidgetImpl(QPixmap& value, QWidget* parent) final
    {
        auto label = new QLabel(parent);
        label->setAlignment(Qt::AlignCenter);
        label->setPixmap(value);
        label->setStyleSheet("border: 1px solid black");
        return label;
    }
};
```

In Demo application we have 2nd tab that shows GUI for MyPixmap instance. Once user changes image url using button or editbox, the MyPixmapWidget will change its content to progress widget and show error or image on loading completion. 

# Customizations
All async value classes are inherited from `AsyncValueTemplate` template class:
```C++
template <typename ValueType_t, typename ErrorType_t, typename ProgressType_t, typename TrackErrorsPolicy_t>
class AsyncValueTemplate : public AsyncValueBase
{
    ...
};
```
So users can override all type parameters to better adopt async values to different environments.

By default `ErrorType_t` parameter is an [AsyncError](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/values/AsyncError.h#L22) class:
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
The `ProgressType_t` parameter represented by [AsyncProgress](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/values/AsyncProgress.h#L29) with the following functions:
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

`TrackErrorsPolicy_t` parameter is used to customize reaction to inconsistent or incorrect situations. By default [AsyncTrackErrorsPolicyDefault](https://github.com/lexxmark/qt-async/blob/master/qt-async-lib/values/AsyncTrackErrorsPolicy.h#L39) class is used:
```C++
 struct AsyncTrackErrorsPolicy
{
    // class to catch emit deadlocks
    struct EmitGuard
    {
        EmitGuard(AsyncTrackErrorsPolicyNone&) {}
    };

    using EmitGuardType = EmitGuard;

    // called when async value is changing inside stateChanged signal handler
    // this incorrect situation will lead to a deadlock 
    void trackEmitDeadlock() const;
    
    // called when async value is in progress and destructing at the same time
    // usually user should call value.stopAndWait(); before value destruction
    void inProgressWhileDestruct() const;
    
    // called if startProgress is called while value is in progress already
    void startProgressWhileInProgress() const;
    
    // called when completeProgress is called with different progress object
    void tryCompleteAlienProgress() const;
    
    // called inside value.completeProgress function if niether value nor error was assigned to async value
    void incompleteProgress() const;
};
```

To use async values with different asynchronious API or frameworks you can create `asynValueRunXXX` like function.
The schema is simple:
```C++
template <typename AsyncValueType, ...>
bool asyncValueRunMyFramework(AsyncValueType& value, Routine func, ...)
{
    // create progress
    auto progress = std::make_unique<typename AsyncValueType::ProgressType>(...);
    auto progressPtr = progress.get();
    
    // try to switch value to progress state
    if (!value.startProgress(std::move(progress)))
        return false;

    // move async value and calculation routine to MyFramework
    MyFramework::AsyncCall([&value, progressPtr, func = std::forward<Func>(func)]() {
        // invoke routine
        func(...);
        
        // finalize progress
        value.completeProgress(progressPtr);
    });
}
```
