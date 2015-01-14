#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include "TaskItem.h"
#include "TaskLogger.h"

TaskLogger::TaskLogger()
    : taskItems(NULL)
    , taskHistoriesLoaded(false)
{
//    logDirectory = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/";
    // Check if this is the same directory as used before
    logDirectory = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/";
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


// This will return all saved and active sessions
const QList<TaskSession> TaskLogger::getTaskSessions() {
    loadTaskHistories();

    QList<TaskSession> sessions = taskSessions;

    if(taskItems != NULL) {
        for(int i = 0; i < taskItems->size(); i++) {
            Task *task = (*taskItems)[i]->task;
            if(task->isActive()) {
                TaskSession activeSession;
                activeSession.taskId = task->getId();
                activeSession.duration = task->getCurrentDuration();
                activeSession.time = QDateTime::currentDateTime().toTime_t() - task->getCurrentDuration();
                sessions.append(activeSession);
            }
        }
    }

    return sessions;
}


void TaskLogger::setTaskItems(const QList<TaskItem*>* taskItems) {
    this->taskItems = taskItems;
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
