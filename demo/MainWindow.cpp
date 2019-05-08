#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDesktopWidget>
#include <QFileDialog>
#include "widgets/AsyncWidget.h"
#include "widgets/AsyncWidgetProgressSpinner.h"
#include "widgets/AsyncWidgetProgressCircle.h"
#include "values/AsyncValueRunThreadPool.h"
#include "values/AsyncValueRunNetwork.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_value(this, AsyncInitByValue{}, "Hello World!")
{
    ui->setupUi(this);
    move(QApplication::desktop()->availableGeometry().center() - rect().center());

    // init pixmap widget
    {
        auto valueWidget = new MyPixmapWidget(ui->widget, &m_pixmap);
        ui->imageAsyncWidget->setContentWidget(valueWidget);
    }

    // init string widget
    {
        auto valueWidget = new AsyncWidgetFn<AsyncQString>(ui->widget);

        valueWidget->createValueWidget = [](QString& value, QWidget* parent) {
            return AsyncWidgetProxy::createLabel(value, parent);
        };

        valueWidget->createProgressWidget = [this](AsyncProgress& progress, QWidget* parent)->QWidget* {
            switch (m_progressWidgetMode) {
            case PROGRESS_MODE::SPINNER_LINES:
                return new AsyncWidgetProgressSpinner(progress, parent);
            case PROGRESS_MODE::PROGRESS_CIRCLE:
                return new AsyncWidgetProgressCircle(progress, parent);

            default:
                return new AsyncWidgetProgressBar(progress, parent);
            }
        };

        valueWidget->setValue(&m_value);

        ui->widget->setContentWidget(valueWidget);
    }

    QObject::connect(&m_value, &AsyncQString::stateChanged, this, &MainWindow::OnAsyncValueChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
    m_value.stopAndWait();
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
    asyncValueRunThreadPool(m_value, [](AsyncProgress& progress, AsyncValue<QString>& value){

        for (auto i : {0, 1, 2, 3, 4})
        {
            progress.setProgress(i, 5);
            if (progress.isStopRequested())
            {
                value.emplaceError("Stopped");
                return;
            }

            QThread::sleep(1);
        }

        progress.setProgress(1.f);
        progress.setMessage("Processing...");
        QThread::sleep(1);

        QString val = "Loaded value is 42";
        value.emplaceValue(std::move(val));

    }, "Loading...", ASYNC_CAN_REQUEST_STOP::YES);
}


void MainWindow::on_stopBttn_clicked()
{
    m_value.accessProgress([](AsyncProgress& progress){
        progress.requestStop();
    });
}

void MainWindow::on_radioButtonProgressBar_clicked()
{
    m_progressWidgetMode = PROGRESS_MODE::PROGRESS_BAR;
}

void MainWindow::on_radioButtonSpinnerLines_clicked()
{
    m_progressWidgetMode = PROGRESS_MODE::SPINNER_LINES;
}

void MainWindow::on_radioButtonCircle_clicked()
{
    m_progressWidgetMode = PROGRESS_MODE::PROGRESS_CIRCLE;
}

void MainWindow::on_imageUrlButton_clicked()
{
    auto filePath = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    if (filePath.isNull())
        return;

    ui->imageUrl->setText(filePath);
}

void MainWindow::on_imageUrl_textChanged(const QString & arg)
{
    m_pixmap.setImageUrl(arg);
}
