#ifndef TASKLOGGER_H
#define TASKLOGGER_H

#include <QHash>
#include <QTime>
#include "Task.h"

#define LOG_FILENAME "task_log.dat"

class TaskLogger : public QObject
{
    Q_OBJECT
public:
    TaskLogger();

    void addTask(Task *task);
    void deleteTaskHistory(Task *task);

protected slots:
    void taskToggled(bool active);

private:
    QHash<int, int> initialTaskDuration;
    QHash<int, QDateTime> taskActivationTime;
    QString logDirectory;
};

#endif // TASKLOGGER_H
