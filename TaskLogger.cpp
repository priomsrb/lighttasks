#include "TaskLogger.h"
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>

TaskLogger::TaskLogger()
{
    logDirectory = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/";
}


void TaskLogger::addTask(Task *task) {
    QObject::connect(task, SIGNAL(toggled(bool)), this, SLOT(taskToggled(bool)));
}


void TaskLogger::deleteTaskHistory(Task *task) {
    QFile oldFile(logDirectory + LOG_FILENAME);

    if(oldFile.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QTextStream oldFileStream(&oldFile);

        QTemporaryFile newFile;
        newFile.open();
        QTextStream newFileStream(&newFile);

        QString line = oldFileStream.readLine();

        while(!line.isNull()) {
            QTextStream lineStream(&line);
            int taskId;
            lineStream >> taskId;
            if(taskId != task->getId()) {
                newFileStream << line << "\n";
            }

            line = oldFileStream.readLine();
        }

        oldFile.close();
        newFile.close();

        oldFile.remove();
        newFile.copy(oldFile.fileName()); // newFile will automatically be deleted
    }
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

        if(dir.mkpath(logDirectory) && file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&file);
            int taskDuration = task->getTotalTime() - initialTaskDuration[taskId];
            stream << task->getId() << " " << taskActivationTime[taskId].toTime_t() << " " << taskDuration << "\n";
            file.close();
        }
    }
}

