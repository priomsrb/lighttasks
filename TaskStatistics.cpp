#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include "TaskStatistics.h"
#include "MainWindow.h"

TaskStatistics::TaskStatistics(QList<TaskItem *> *taskItems)
    : taskItems(taskItems)
{
    logDirectory = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/";
}

bool TaskStatistics::exportToCsv(QString filename) {
    QFile logFile(logDirectory + LOG_FILENAME);
    QFile csvFile(filename);

    if(!logFile.open(QIODevice::ReadOnly | QIODevice::Text)
            || !csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream logFileStream(&logFile);
    QTextStream csvFileStream(&csvFile);

    csvFileStream << "Task, Date, Time, Duration\n";

    QHash<int, QString> taskNames;

    for(int i=0; i < taskItems->size(); i++) {
        Task *task = (*taskItems)[i]->task;
        taskNames[task->getId()] = task->getName();
    }

    QString line = logFileStream.readLine();

    while(!line.isNull()) {
        QTextStream lineStream(&line);
        int id, time, duration;
        lineStream >> id >> time >> duration;

        QString taskName = "Unknown Task";
        if(taskNames.contains(id)) {
            taskName = taskNames[id];
        }

        csvFileStream << getCsvLine(taskName, time, duration);

        line = logFileStream.readLine();
    }


    // Export the currently active tasks. This needs to be done explicitly
    // because they have not been written to the log file yet.
    // TODO: Not very happy with this approach but it works for now. Try to make it cleaner
    for(int i=0; i < taskItems->size(); i++) {
        Task *task = (*taskItems)[i]->task;
        if(task->isActive()) {
            int time = QDateTime::currentDateTime().toTime_t() - task->getCurrentDuration();
            csvFileStream << getCsvLine(task->getName(), time, task->getCurrentDuration());
        }
    }

    logFile.close();
    csvFile.close();

    return true;

}


QString TaskStatistics::getCsvLine(QString name, int time, int duration) {
    QString line;
    QTextStream lineStream(&line);

    // TODO: handle ill-formatted task names (e.g. task names with commas and quotes)
    lineStream << name << ", ";

    QDateTime dateTime = QDateTime::fromTime_t(time);

    lineStream << dateTime.toString("dd/MM/yyyy") << ", ";
    lineStream << dateTime.toString("hh:mm:ss") << ", ";

    int seconds = duration % 60;
    int minutes = (duration / 60) % 60;
    int hours = duration / 3600;
    QString durationString = QString("%1:%2:%3")
            .arg(QString::number(hours), 2, '0')
            .arg(QString::number(minutes), 2, '0')
            .arg(QString::number(seconds), 2, '0');

    lineStream << durationString;

    lineStream << "\n";

    return line;
}
