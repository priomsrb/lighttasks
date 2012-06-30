#include "LightTasksApplication.h"
#include <QDebug>

LightTasksApplication::LightTasksApplication(const QString &id, int &argc, char **argv)
    : QtSingleApplication(id, argc, argv)
{
    setApplicationName("lighttasks");
    setOrganizationName("lighttasks");
}

void LightTasksApplication::commitData(QSessionManager &manager) {
    Q_UNUSED(manager);
    // Leaving this empty means all windows will be closed when session manager requests
}
