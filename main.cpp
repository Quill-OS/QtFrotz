
#include "qtfrotzwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char* argv[] ){
    QApplication a(argc,argv);

    QFile stylesheetFile("/etc/eink.qss");
    stylesheetFile.open(QFile::ReadOnly);
    a.setStyleSheet(stylesheetFile.readAll());
    stylesheetFile.close();

    QtFrotzWindow w;
    w.showFullScreen();
    return a.exec();
}
