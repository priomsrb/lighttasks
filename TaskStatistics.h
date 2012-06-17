#ifndef TASKSTATISTICS_H
#define TASKSTATISTICS_H

#include "TaskLogger.h"

struct TaskItem;

class TaskStatistics
{
public:
    TaskStatistics(QList<TaskItem*> *taskItems);


    bool exportToCsv(QString filename);

private:
    QString logDirectory;
    QList<TaskItem*> *taskItems;

    QString getCsvLine(QString name, int time, int duration);
};

#endif // TASKSTATISTICS_H
