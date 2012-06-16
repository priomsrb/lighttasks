#include <QtGui/QApplication>
#include "include/qtsingleapplication/QtSingleApplication"
#include "MainWindow.h"

int main(int argc, char *argv[])
{

    QtSingleApplication instance("lighttasks", argc, argv);

    if(instance.isRunning()) {
        return !instance.sendMessage("Restore previous instance");
    }

    MainWindow w;

    QObject::connect(&instance, SIGNAL(messageReceived(const QString&)),
                 &w, SLOT(restore()));

    w.show();

    return instance.exec();
}
