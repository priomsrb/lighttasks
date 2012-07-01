#include "TaskLogger.h"
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>

TaskLogger::TaskLogger()
    : taskHistoriesLoaded(false)
{
    logDirectory = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/";
}


void TaskLogger::addTask(Task *task) {
    QObject::connect(task, SIGNAL(toggled(bool)), this, SLOT(taskToggled(bool)));
}


void TaskLogger::deleteTaskHistory(Task *task) {
    loadTaskHistories();

    for(int i=0; i < taskSessions.size(); i++) {
        if(taskSessions[i].taskId == task->getId()) {
            taskSessions.removeAt(i);
            i--;
        }
    }

    saveTaskHistories();
}

const QList<TaskSession>* TaskLogger::getTaskSessions() {
    loadTaskHistories();
    return &taskSessions;
}

void TaskLogger::taskToggled(bool active) {
    Task *task = static_cast<Task*>(sender());
    int taskId = task->getId();

    if(active) {
        initialTaskDuration[taskId] = task->getTotalTime();
        taskActivationTime[taskId] = QDateTime::currentDateTime();
    } else {

        QDir dir;
        QFile file(logDirectory + LOG_FILENAME);

        TaskSession taskSession;
        taskSession.taskId = taskId;
        taskSession.duration = task->getTotalTime() - initialTaskDuration[taskId];
        taskSession.time = taskActivationTime[taskId].toTime_t();
        taskSessions.append(taskSession);

        if(dir.mkpath(logDirectory) && file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&file);
            writeSessionToStream(taskSession, stream);
            file.close();
        }

    }
}

void TaskLogger::writeSessionToStream(const TaskSession taskSession, QTextStream &stream) {
    stream << taskSession.taskId << " " << taskSession.time << " " << taskSession.duration << "\n";
}

void TaskLogger::loadTaskHistories() {
    if(taskHistoriesLoaded == false) {
        QFile logFile(logDirectory + LOG_FILENAME);

        if(!logFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        taskSessions.clear();

        QTextStream logFileStream(&logFile);

        QString line = logFileStream.readLine();

        while(!line.isNull()) {
            QTextStream lineStream(&line);
            TaskSession taskSession;
            lineStream >> taskSession.taskId >> taskSession.time >> taskSession.duration;

            taskSessions.append(taskSession);

            line = logFileStream.readLine();
        }

        taskHistoriesLoaded = true;
    }
}

void TaskLogger::saveTaskHistories() {
    QDir dir;
    QFile file(logDirectory + LOG_FILENAME);

    if(dir.mkpath(logDirectory) && file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);

        TaskSession taskSession;
        foreach(taskSession, taskSessions) {
            writeSessionToStream(taskSession, stream);
        }

        file.close();
    }

}
