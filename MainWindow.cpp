#include <QDebug>
#include <QMenu>
#include <QSettings>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "TaskButton.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , state(NORMAL)
{
    ui->setupUi(this);
    setWindowIcon(QIcon("icons/lighttasks.png"));
    setupTrayIcon();
    loadSettings();

    connect(ui->mainOperationButton, SIGNAL(clicked()), this, SLOT(operationButtonClicked()));
    connect(&saveTimer, SIGNAL(timeout()), this, SLOT(saveSettings()));
    saveTimer.start(60000);
    tickTimer.start(1000);
}


MainWindow::~MainWindow() {
    saveSettings();
    delete ui;
}


void MainWindow::setupTrayIcon() {
    quitAction = new QAction("Quit", this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    systemTrayMenu = new QMenu(this);
    systemTrayMenu->addSeparator();
    systemTrayMenu->addAction(quitAction);

    systemTrayIcon = new QSystemTrayIcon(QIcon("icons/lighttasks_small.png"), this);
    systemTrayIcon->setContextMenu(systemTrayMenu);
    updateSystemTrayToolTip();
    systemTrayIcon->show();
    connect(systemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(systemTrayActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::systemTrayActivated(QSystemTrayIcon::ActivationReason activationReason) {
    if(activationReason == QSystemTrayIcon::Trigger) {
        if(this->isActiveWindow()) {
            this->hide();
        } else {
            this->show();
            this->activateWindow();
            this->setGeometry(oldGeometry);
        }
    }
}

void MainWindow::operationButtonClicked() {

    if(state == NORMAL) {
        Task task;
        task.setName("New Task");
        TaskItem *taskItem = addTaskItem(task);
        taskItem->taskButton->startRenaming();


    } else if (state == EDITING) {
        for(int i=0; i < taskItems.size(); i++) {
            taskItems[i]->taskButton->cancelEditing();
        }

        changeState(NORMAL);
    }

}

MainWindow::TaskItem* MainWindow::addTaskItem(Task task) {
    TaskButton *taskButton = new TaskButton(this);

    connect(&tickTimer, SIGNAL(timeout()), taskButton, SLOT(tick()));
    connect(taskButton, SIGNAL(startedEditing()), this, SLOT(taskStartedEditing()));
    connect(taskButton, SIGNAL(cancelledEditing()), this, SLOT(taskCancelledEditing()));
    connect(taskButton, SIGNAL(finishedEditing()), this, SLOT(taskFinishedEditing()));
    connect(taskButton, SIGNAL(deleted()), this, SLOT(taskDeleted()));
    connect(taskButton, SIGNAL(movedUp()), this, SLOT(taskMovedUp()));
    connect(taskButton, SIGNAL(movedDown()), this, SLOT(taskMovedDown()));

    taskButton->init(task);

    ui->taskListLayout->insertWidget(0, taskButton);
    ui->taskListScrollArea->ensureWidgetVisible(taskButton);
    setTabOrder(ui->mainOperationButton, taskButton); // We insert to top rather than bottom, so we need to manually set tab order

    QAction *trayAction = new QAction(task.getName().c_str(), this);
    trayAction->setCheckable(true);
    connect(taskButton, SIGNAL(activated(bool)), trayAction, SLOT(setChecked(bool)));
    connect(trayAction, SIGNAL(toggled(bool)), taskButton, SLOT(setActive(bool)));
    systemTrayMenu->insertAction(systemTrayMenu->actions().first(), trayAction);
    connect(taskButton, SIGNAL(activated(bool)), this, SLOT(updateSystemTrayToolTip()));

    TaskItem *taskItem = new TaskItem();
    taskItem->taskButton = taskButton;
    taskItem->trayAction = trayAction;
    taskItems.insert(0, taskItem);

    return taskItem;
}


void MainWindow::taskStartedEditing() {
    for(int i=0; i < taskItems.size(); i++) {
        TaskButton *taskButton = taskItems[i]->taskButton;
        if(taskButton != sender() && taskButton->isEditing()) {
            taskButton->cancelEditing();
        }
    }

    changeState(EDITING);
}

void MainWindow::taskCancelledEditing() {
    changeState(NORMAL);
    TaskButton *taskButton = static_cast<TaskButton*>(sender());

    if(taskButton->isValid() == false) {
        TaskItem *taskItem = getTaskItemFromButton(taskButton);
        removeTaskItem(taskItem);
    }
}


void MainWindow::removeTaskItem(TaskItem* taskItem) {
    taskItem->taskButton->deleteLater();
    taskItem->trayAction->deleteLater();
    taskItems.removeOne(taskItem);
}

MainWindow::TaskItem* MainWindow::getTaskItemFromButton(TaskButton* taskButton) {
    for(int i=0; i < taskItems.size(); i++) {
        if(taskItems[i]->taskButton == taskButton) {
            return taskItems[i];
        }
    }

    return NULL;
}


void MainWindow::taskFinishedEditing() {
    TaskButton *taskButton = static_cast<TaskButton*>(sender());
    TaskItem *taskItem = getTaskItemFromButton(taskButton);

    QString newTaskName = taskItem->taskButton->getTask().getName().c_str();
    taskItem->trayAction->setText(newTaskName);

    taskItem->taskButton->setFocus();
    changeState(NORMAL);
}


void MainWindow::changeState(State newState) {

    if(newState == EDITING) {
        ui->mainOperationButton->setText("Cancel");
    } else if(newState == NORMAL) {
        ui->mainOperationButton->setText("Add new task");
    }

    state = newState;
}

void MainWindow::taskDeleted() {
    TaskButton *taskButton = static_cast<TaskButton*>(sender());
    TaskItem *taskItem = getTaskItemFromButton(taskButton);
    removeTaskItem(taskItem);
}

void MainWindow::taskMovedUp() {
    TaskButton *taskButton = static_cast<TaskButton*>(sender());
    TaskItem *taskItem = getTaskItemFromButton(taskButton);

    int index = taskItems.indexOf(taskItem);

    if(index > 0) {
        moveTaskItem(index, index - 1);
    }
}

void MainWindow::taskMovedDown() {
    TaskButton *taskButton = static_cast<TaskButton*>(sender());
    TaskItem *taskItem = getTaskItemFromButton(taskButton);

    int index = taskItems.indexOf(taskItem);

    if(index < taskItems.size() - 1) {
        moveTaskItem(index, index + 1);
    }
}

void MainWindow::moveTaskItem(int fromIndex, int toIndex) {
    TaskItem *taskItem = taskItems[fromIndex];
    ui->taskListLayout->removeWidget(taskItem->taskButton);
    ui->taskListLayout->insertWidget(toIndex, taskItem->taskButton);
    systemTrayMenu->removeAction(taskItem->trayAction);

    systemTrayMenu->insertAction(systemTrayMenu->actions()[toIndex], taskItem->trayAction);
    taskItems.move(fromIndex, toIndex);
}

void MainWindow::loadSettings() {
    QSettings settings("lighttasks", "lighttasks");

    int numTasks = settings.beginReadArray("tasks");

    // We load the tasks in reverse order because the tasks are added like a stack
    for(int i=numTasks - 1; i >= 0; i--) {
        settings.setArrayIndex(i);

        Task task;
        task.setName(settings.value("name").toString().toStdString());
        task.setTime(settings.value("time").toInt());
        TaskItem *taskItem = addTaskItem(task);
        taskItem->taskButton->setValid(true);
    }
    settings.endArray();
}


void MainWindow::saveSettings() {
    QSettings settings("lighttasks", "lighttasks");
    settings.clear();

    int numTasks = taskItems.size();
    settings.beginWriteArray("tasks", numTasks);

    for(int i=0; i < numTasks; i++) {
        settings.setArrayIndex(i);

        Task task = taskItems[i]->taskButton->getTask();
        settings.setValue("name", task.getName().c_str());
        settings.setValue("time", (qulonglong)task.getTime());
    }
    settings.endArray();

    settings.sync();
}


void MainWindow::updateSystemTrayToolTip() {
    TaskItem *taskItem;
    QList<TaskItem*> activeTaskItems;

    foreach(taskItem, taskItems) {
        if(taskItem->taskButton->getTask().isActive()) {
            activeTaskItems.append(taskItem);
        }
    }

    QString tooltip;
    if(activeTaskItems.count() == 0) {
        tooltip = "No tasks active";
    } else if(activeTaskItems.count() == 1) {
        QString taskName = activeTaskItems[0]->taskButton->getTask().getName().c_str();
        tooltip = "'" + taskName + "' is active";
    } else {
        tooltip = QString::number(activeTaskItems.count()) + " tasks active";
    }

    systemTrayIcon->setToolTip(tooltip);
}

void MainWindow::moveEvent(QMoveEvent *event) {
    Q_UNUSED(event);
    // We need to remember the old geometry when restoring the window from the system tray
    oldGeometry = this->geometry();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    // We need to remember the old geometry when restoring the window from the system tray
    oldGeometry = this->geometry();
}
