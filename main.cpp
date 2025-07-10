#include "bcai.h"
#include "QCamera"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BCAI w;
    w.show();
    return a.exec();

}
