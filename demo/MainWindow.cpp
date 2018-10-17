#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDesktopWidget>
#include <QFileDialog>
#include <QBitmap>
#include "widgets/AsyncWidget.h"
#include "widgets/AsyncWidgetProgressSpinner.h"
#include "values/AsyncValueRunThreadPool.h"
#include "values/AsyncValueRunThread.h"
#include "values/AsyncValueRunNetwork.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_value(this, AsyncInitByValue{}, "Hello World!"),
    m_valuePixmap(AsyncInitByError{}, "Select image file path.")
{
    ui->setupUi(this);
    move(QApplication::desktop()->availableGeometry().center() - rect().center());

    // init async image stuff
    {
        m_valuePixmap.deferFn = [this](const AsyncQPixmap::RunFnType& fn) {
            m_imageUrl = ui->imageUrl->text();
            asyncValueRunThread(m_valuePixmap, fn, "Loading image...", ASYNC_CAN_REQUEST_STOP::NO);
        };
        m_valuePixmap.runFn = [this](AsyncProgressRerun& progress, AsyncQPixmap& value) {

            QImage image(m_imageUrl);

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
                value.emplaceError(QString("Cannot load image from file '%1'.").arg(m_imageUrl));
            else
                value.emplaceValue(QPixmap::fromImage(image));
        };

        auto valueWidget = new AsyncWidgetFn<AsyncQPixmap>(ui->widget);

        valueWidget->createValueWidget = [](QPixmap& value, QWidget* parent) {
            auto label = new QLabel(parent);
            label->setAlignment(Qt::AlignCenter);
            label->setPixmap(value);
            label->setStyleSheet("border: 1px solid black");
            return label;
        };

        valueWidget->createProgressWidget = [this](AsyncProgress& progress, QWidget* parent)->QWidget* {
            switch (m_progressWidgetMode) {
            case PROGRESS_MODE::SPINNER_LINES:
                return new AsyncWidgetProgressSpinner(progress, parent);

            default:
                return new AsyncWidgetProgressBar(progress, parent);
            }
        };

        valueWidget->setValue(&m_valuePixmap);

        ui->imageAsyncWidget->setContentWidget(valueWidget);
    }


    {
        auto valueWidget = new AsyncWidgetFn<AsyncQString>(ui->widget);

        valueWidget->createValueWidget = [](QString& value, QWidget* parent) {
            return AsyncWidgetProxy::createLabel(value, parent);
        };

        valueWidget->createProgressWidget = [this](AsyncProgress& progress, QWidget* parent)->QWidget* {
            switch (m_progressWidgetMode) {
            case PROGRESS_MODE::SPINNER_LINES:
                return new AsyncWidgetProgressSpinner(progress, parent);

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

void MainWindow::on_imageUrlButton_clicked()
{
    auto filePath = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
    if (filePath.isNull())
        return;

    ui->imageUrl->setText(filePath);
}

void MainWindow::on_imageUrl_textChanged(const QString & arg)
{
    // copy url
    m_imageUrl = arg;
    // reload image
    m_valuePixmap.run();
}
