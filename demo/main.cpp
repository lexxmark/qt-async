#include "MainWindow.h"
#include <QApplication>
#include <widgets/AsyncWidgetError.h>

static QString asyncStyleSheet = R"(
    AsyncWidgetError,
    AsyncWidgetProgressBar,
    AsyncWidgetProgressSpinner,
    AsyncWidgetProgressCircle,
    QLabel#AsyncLabel {
        background: white;
        border: 1px solid black;
    }
)";

int main(int argc, char *argv[])
{
    //QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    a.setStyleSheet(asyncStyleSheet);
    MainWindow w;
    w.show();
    
    return a.exec();
}
