#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDesktopWidget>
#include <thread>
#include "AsyncWidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    move(QApplication::desktop()->availableGeometry().center() - rect().center());

    m_value.setValue("Hello");
    auto valueWidget = QSharedPointer<AsyncValueWidget<QString>>::create(ui->widget);
    ui->widget->setContent(valueWidget);
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
    m_value.setError("Error happened!");
}

void MainWindow::on_valueBttn_clicked()
{
    m_value.emplaceValue("A new value assigned!");
}

void MainWindow::on_startBttn_clicked()
{
    aquireAsyncValue(QThreadPool::globalInstance(), m_value, "Loading...", true, [](AsyncProgress& progress, AsyncValue<QString>& value){

        using namespace std::chrono_literals;
        for (auto i : {0, 1, 2, 3, 4})
        {
            progress.setProgress((float)i/4.f);
            if (progress.isStopRequested())
            {
                value.setError("Stopped");
                return;
            }

            std::this_thread::sleep_for(1s);
        }

        QString val = "Result";
        value.setValue(std::move(val));
    });
}


void MainWindow::on_stopBttn_clicked()
{
    m_value.accessProgress([](AsyncProgress& progress){
        progress.requestStop();
    });
}
