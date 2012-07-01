#ifndef TASKLOGGER_H
#define TASKLOGGER_H

#include <QHash>
#include <QList>
#include <QTextStream>
#include <QTime>
#include "Task.h"
#include "Singleton.h"

#define LOG_FILENAME "task_log.dat"


struct TaskSession {
    int taskId;
    unsigned int time;
    int duration;
};

class TaskLogger : public QObject, public Singleton<TaskLogger>
{
    Q_OBJECT
public:
    TaskLogger();

    void addTask(Task *task);
    void deleteTaskHistory(Task *task);
    const QList<TaskSession>* getTaskSessions();

protected slots:
    void taskToggled(bool active);

private:
    QHash<int, int> initialTaskDuration;
    QHash<int, QDateTime> taskActivationTime;
    QList<TaskSession> taskSessions;
    bool taskHistoriesLoaded;

    void loadTaskHistories();
    void saveTaskHistories();
    void writeSessionToStream(const TaskSession taskSession, QTextStream &stream);

    QString logDirectory;
};

#endif // TASKLOGGER_H
