#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include "TaskHistoryDialog.h"
#include "ui_TaskHistoryDialog.h"
#include "MainWindow.h"

const QList<TaskItem*> *TaskHistoryDialog::taskItems = 0;

TaskHistoryDialog::TaskHistoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskHistoryDialog)
{
    ui->setupUi(this);
    ui->historyTable->setColumnWidth(2, 180);

    connect(ui->taskComboBox, SIGNAL(activated(int)), this, SLOT(taskChanged(int)));
    connect(ui->saveToFileButton, SIGNAL(clicked()), this, SLOT(showSaveToFileDialog()));

    for(int i=0; i < taskItems->size(); i++) {
        Task *task = (*taskItems)[i]->task;
        ui->taskComboBox->addItem(task->getName());
    }

    showTask(NULL);
}

TaskHistoryDialog::~TaskHistoryDialog()
{
    delete ui;
}


// If task is NULL then it will show all tasks
void TaskHistoryDialog::showTask(Task *task) {

    if(task != NULL) {
        int taskIndex = ui->taskComboBox->findText(task->getName());
        if(taskIndex != -1) {
            ui->taskComboBox->setCurrentIndex(taskIndex);
        }
    }


    const QList<TaskSession> *taskSessions = TaskLogger::getInstance()->getTaskSessions();

    relevantSessions.clear();
    for(int i = 0; i < taskSessions->size(); i++) {
        TaskSession session = (*taskSessions)[i];
        if(task == NULL || session.taskId == task->getId()) {
            relevantSessions.append(session);
        }
    }

    // We pre-size the table to make the rendering faster
    ui->historyTable->setRowCount(relevantSessions.size());
    ui->historyTable->clearContents();

    for(int i = 0; i < relevantSessions.size(); i++) {
        TaskSession session = relevantSessions[i];

        QDateTime sessionDateTime = QDateTime::fromTime_t(session.time);
        QString date = sessionDateTime.toString("dd/MM/yyyy");
        QString time = sessionDateTime.toString("hh:mm:ss");
        QString name = getTaskName(session.taskId);
        QString duration = timeToString(session.duration);

        ui->historyTable->setItem(i, 0, newReadOnlyItem(date));
        ui->historyTable->setItem(i, 1, newReadOnlyItem(time));
        ui->historyTable->setItem(i, 2, newReadOnlyItem(name));
        ui->historyTable->setItem(i, 3, newReadOnlyItem(duration));
    }
}

void TaskHistoryDialog::setTaskItems(const QList<TaskItem*>* taskItems_) {
    taskItems = taskItems_;
}


QString TaskHistoryDialog::timeToString(int time) {
    int seconds = time % 60;
    int minutes = (time / 60) % 60;
    int hours = time / 3600;
    QString timeString = QString("%1:%2:%3")
            .arg(QString::number(hours), 2, '0')
            .arg(QString::number(minutes), 2, '0')
            .arg(QString::number(seconds), 2, '0');

    return timeString;
}

QString TaskHistoryDialog::getTaskName(int taskId) {

    for(int i=0; i < taskItems->size(); i++) {
        Task *task = (*taskItems)[i]->task;
        if(task->getId() == taskId) {
            return task->getName();
        }
    }

    return "Unknown Task";
}

void TaskHistoryDialog::taskChanged(int taskIndex) {
    if(taskIndex != 0) {
        QString taskName = ui->taskComboBox->itemText(taskIndex);

        for(int i=0; i < taskItems->size(); i++) {
            Task *task = (*taskItems)[i]->task;
            if(task->getName() == taskName) {
                showTask(task);
                return;
            }
        }
    }

    // If taskIndex is 0 (<All Items> was selected) or the task was not found
    // Then show all tasks
    showTask(NULL);
}


void TaskHistoryDialog::showSaveToFileDialog() {
    // TODO: remember the last used directory

    QString filename = QFileDialog::getSaveFileName(this, "Save history to file", "", "Comma Separated Values (*.csv)");

    if(filename.isNull())
        return;

    if(!saveToCsv(filename)) {
        QMessageBox::warning(this, "Error", "Unable to save history to file");
    }
}

bool TaskHistoryDialog::saveToCsv(QString filename) {
    QFile csvFile(filename);

    if(!csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream csvFileStream(&csvFile);

    csvFileStream << "Date, Time, Task, Duration\n";

    for(int i = 0; i < relevantSessions.size(); i++) {
        csvFileStream << getCsvLine(relevantSessions[i]);
    }

    csvFile.close();

    return true;
}

QString TaskHistoryDialog::getCsvLine(TaskSession session) {
    QString line;
    QTextStream lineStream(&line);

    QDateTime dateTime = QDateTime::fromTime_t(session.time);

    lineStream << dateTime.toString("dd/MM/yyyy") << ", ";
    lineStream << dateTime.toString("hh:mm:ss") << ", ";

    // TODO: handle ill-formatted task names (e.g. task names with commas and quotes)
    QString name = getTaskName(session.taskId);
    lineStream << name << ", ";

    lineStream << timeToString(session.duration);

    lineStream << "\n";

    return line;
}


QTableWidgetItem *TaskHistoryDialog::newReadOnlyItem(QString &text) {
    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    return item;
}
