#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    if(w.needShow)
        w.show();
    else
        w.hide();

    return a.exec();
}
