#include "TaskStatisticsDialog.h"
#include "ui_TaskStatisticsDialog.h"
#include "TaskLogger.h"
#include "TaskHistoryDialog.h"

TaskStatisticsDialog::TaskStatisticsDialog(Task *task, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TaskStatisticsDialog)
    , task(task)
    , totalTime(0)
    , timeToday(0)
    , timeThisWeek(0)
    , timeThisMonth(0)
    , timeThisYear(0)
    , averageDuration(0)
    , longestDuration(0)
{
    ui->setupUi(this);

    connect(ui->showHistoryButton, SIGNAL(clicked()), this, SLOT(showHistoryDialog()));

    int numSessions = 0;

    const QList<TaskSession> *taskSessions = TaskLogger::getInstance()->getTaskSessions();
    TaskSession taskSession;

    foreach(taskSession, *taskSessions) {
        if(taskSession.taskId == task->getId()) {
            QDate sessionDate = QDateTime::fromTime_t(taskSession.time).date();

            if(sessionDate.day() == QDate::currentDate().day()) {
                timeToday += taskSession.duration;
            }

            if(sessionDate.weekNumber() == QDate::currentDate().weekNumber()) {
                timeThisWeek += taskSession.duration;
            }

            if(sessionDate.month() == QDate::currentDate().month()) {
                timeThisMonth += taskSession.duration;
            }

            if(sessionDate.year() == QDate::currentDate().year()) {
                timeThisYear += taskSession.duration;
            }

            totalTime += taskSession.duration;

            longestDuration = qMax(longestDuration, taskSession.duration);
            numSessions++;
        }
    }

    if(numSessions > 0) {
        averageDuration = qRound(float(totalTime) / numSessions);
    }

    ui->totalTime->setText(timeToString(totalTime));
    ui->timeToday->setText(timeToString(timeToday));
    ui->timeThisMonth->setText(timeToString(timeThisMonth));
    ui->timeThisWeek->setText(timeToString(timeThisWeek));
    ui->timeThisYear->setText(timeToString(timeThisYear));
    ui->averageDuration->setText(timeToString(averageDuration));
    ui->longestDuration->setText(timeToString(longestDuration));
}

TaskStatisticsDialog::~TaskStatisticsDialog()
{
    delete ui;
}

QString TaskStatisticsDialog::timeToString(int time) {
    int seconds = time % 60;
    int minutes = (time / 60) % 60;
    int hours = time / 3600;
    QString timeString = QString("%1:%2:%3")
            .arg(QString::number(hours), 2, '0')
            .arg(QString::number(minutes), 2, '0')
            .arg(QString::number(seconds), 2, '0');

    return timeString;
}


void TaskStatisticsDialog::showHistoryDialog() {
    TaskHistoryDialog dialog(this);
    dialog.showTask(task);
    dialog.exec();
}
