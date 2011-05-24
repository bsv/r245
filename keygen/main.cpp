#include "keygen.h"

#include <QtGui>
#include <QApplication>

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QTextCodec::setCodecForCStrings(codec);

    QApplication a(argc, argv);
    KeygenWindow w;
    w.show();
    return a.exec();
}
