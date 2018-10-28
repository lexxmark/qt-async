This library has some usefull Qt classes and widgets to show asyncronious operations.

[![qt-async demo video](http://img.youtube.com/vi/aTXOpmVRXq0/0.jpg)](https://youtu.be/aTXOpmVRXq0)

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

        // optionally set callback that creates progress widget
        valueWidget->createProgressWidget = [this](AsyncProgress& progress, QWidget* parent)->QWidget* {
            return new AsyncWidgetProgressSpinner(progress, parent);
        };

        // optionally set callback that creates error widget
        valueWidget->createProgressWidget = [this](AsyncError& error, QWidget* parent)->QWidget* {
            return new AsyncWidgetError(error, parent);
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

# Rerunnable values
