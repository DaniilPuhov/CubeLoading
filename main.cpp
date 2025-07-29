#include <QApplication>
#include "mainwindow.h"
#include <QTextCodec>

#if __cplusplus != 201703L
#error "C++17 is not enabled!"
#endif

int main(int argc, char *argv[]) {
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QApplication app(argc, argv);

    MainWindow window;
    window.show();    
    return app.exec();
}