#ifndef TASKHISTORYDIALOG_H
#define TASKHISTORYDIALOG_H

#include <QDialog>
#include <QList>
#include "TaskLogger.h"

namespace Ui {
class TaskHistoryDialog;
}

class QTableWidgetItem;
class Task;
class TaskItem;

class TaskHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskHistoryDialog(QWidget *parent = 0);
    ~TaskHistoryDialog();
    void showTask(Task *task);

    static void setTaskItems(const QList<TaskItem *> *taskItems);

private:
    Ui::TaskHistoryDialog *ui;
    static const QList<TaskItem*> *taskItems;
    QList<TaskSession> relevantSessions;


    QString timeToString(int time);
    QString getTaskName(int taskId);

private slots:
    void taskChanged(int taskIndex);
    void showSaveToFileDialog();
    bool saveToCsv(QString filename);
    QString getCsvLine(TaskSession session);
    QTableWidgetItem *newReadOnlyItem(QString &text);
};


#endif // TASKHISTORYDIALOG_H
