#include "MainWindow.h"
#include <QApplication>
#include <widgets/AsyncWidgetError.h>

static QString asyncStyleSheet = R"(
    AsyncWidgetError,
    AsyncWidgetProgressBar,
    AsyncWidgetProgressSpinner,
    QLabel#AsyncLabel {
        background: white;
        border: 1px solid black;
    }
)";

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyleSheet(asyncStyleSheet);
    MainWindow w;
    w.show();
    
    return a.exec();
}
