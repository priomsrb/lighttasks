#include <QtGui/QApplication>
#include "LightTasksApplication.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{

    LightTasksApplication instance("lighttasks", argc, argv);

    if(instance.isRunning()) {
        return !instance.sendMessage("Restore previous instance");
    }

    MainWindow w;

    QObject::connect(&instance, SIGNAL(messageReceived(const QString&)),
                 &w, SLOT(restore()));

    w.show();

    return instance.exec();
}
