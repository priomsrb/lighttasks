#ifndef LIGHTTASKSAPPLICATION_H
#define LIGHTTASKSAPPLICATION_H


#include "include/qtsingleapplication/QtSingleApplication"

class LightTasksApplication : public QtSingleApplication
{
    Q_OBJECT
public:
    LightTasksApplication(const QString &id, int &argc, char **argv);

signals:
    
public slots:

private:
    void commitData(QSessionManager &manager);

};

#endif // LIGHTTASKSAPPLICATION_H
