#ifndef TASKSTATISTICSDIALOG_H
#define TASKSTATISTICSDIALOG_H

#include <QDialog>
#include "Task.h"

namespace Ui {
class TaskStatisticsDialog;
}
class TaskLogger;


class TaskStatisticsDialog : public QDialog
{
    Q_OBJECT
    
public:
    TaskStatisticsDialog(Task *task, QWidget *parent);
    ~TaskStatisticsDialog();
    
private:
    Ui::TaskStatisticsDialog *ui;
    Task *task;

    int totalTime;
    int timeToday;
    int timeThisWeek;
    int timeThisMonth;
    int timeThisYear;
    int averageDuration;
    int longestDuration;

private slots:
    void showHistoryDialog();

};

#endif // TASKSTATISTICSDIALOG_H
