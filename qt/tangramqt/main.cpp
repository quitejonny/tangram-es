#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::locale::global( std::locale( "C" ) );
    MainWindow w;
    w.show();

    return a.exec();
}
