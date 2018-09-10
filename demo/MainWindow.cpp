#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDesktopWidget>
#include <thread>
#include "widgets/AsyncWidget.h"
#include "values/AsyncValueObtain.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_value(this, AsyncInitByValue{}, "Hello World!")
{
    ui->setupUi(this);
    move(QApplication::desktop()->availableGeometry().center() - rect().center());

    auto valueWidget = QSharedPointer<AsyncWidget<QString>>::create(ui->widget);
    ui->widget->setContentWidget(valueWidget);
    valueWidget->setValue(&m_value);

    QObject::connect(&m_value, &AsyncQString::stateChanged, this, &MainWindow::OnAsyncValueChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnAsyncValueChanged(ASYNC_VALUE_STATE state)
{
    ui->errorBttn->setEnabled(state != ASYNC_VALUE_STATE::PROGRESS);
    ui->valueBttn->setEnabled(state != ASYNC_VALUE_STATE::PROGRESS);
    ui->stopBttn->setEnabled(state == ASYNC_VALUE_STATE::PROGRESS);
}

void MainWindow::on_errorBttn_clicked()
{
    m_value.emplaceError("Error happened!");
}

void MainWindow::on_valueBttn_clicked()
{
    m_value.emplaceValue("A new value assigned!");
}

void MainWindow::on_startBttn_clicked()
{
    asyncValueObtain(QThreadPool::globalInstance(), m_value, [](AsyncProgress& progress, AsyncValue<QString>& value){

        using namespace std::chrono_literals;
        for (auto i : {0, 1, 2, 3, 4})
        {
            progress.setProgress((float)i/4.f);
            if (progress.isStopRequested())
            {
                value.emplaceError("Stopped");
                return;
            }

            std::this_thread::sleep_for(1s);
        }

        QString val = "Result";
        value.emplaceValue(std::move(val));
    }, "Loading...", true);
}


void MainWindow::on_stopBttn_clicked()
{
    m_value.accessProgress([](AsyncProgress& progress){
        progress.requestStop();
    });
}
