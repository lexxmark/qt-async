#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <values/AsyncValue.h>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

using AsyncQString = AsyncValue<QString>;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void OnAsyncValueChanged(ASYNC_VALUE_STATE state);
    void on_errorBttn_clicked();
    void on_valueBttn_clicked();
    void on_startBttn_clicked();

    void on_stopBttn_clicked();

    void on_radioButtonProgressBar_clicked();

    void on_radioButtonSpinnerLines_clicked();

private:
    Ui::MainWindow *ui;

    AsyncQString m_value;

    enum class PROGRESS_MODE
    {
        PROGRESS_BAR,
        SPINNER_LINES,
    } m_progressWidgetMode = PROGRESS_MODE::PROGRESS_BAR;
};

#endif // MAINWINDOW_H
