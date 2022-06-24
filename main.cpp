
#include "qtfrotzwindow.h"
#include <QApplication>


int main(int argc, char* argv[] ){
    QApplication a(argc,argv);
    QtFrotzWindow w;
    w.showFullScreen();
    //w.show();
    return a.exec();
}
