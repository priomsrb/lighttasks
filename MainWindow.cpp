#include <QDebug>
#include <QMenu>
#include <QSettings>
#include <QWindowStateChangeEvent>
#include "qticonloader.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "TaskButton.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , state(NORMAL)
    , reallyQuit(false)
{
    ui->setupUi(this);
    setupTrayIcon();
    loadSettings();

    connect(ui->mainOperationButton, SIGNAL(clicked()), this, SLOT(mainOperationButtonClicked()));
    connect(&saveTimer, SIGNAL(timeout()), this, SLOT(saveSettings()));
    connect(&tickTimer, SIGNAL(timeout()), this, SLOT(updateSystemTrayToolTip()));
    saveTimer.start(60000);
    tickTimer.start(1000);
}


MainWindow::~MainWindow() {
    saveSettings();
    delete ui;
}


void MainWindow::setupTrayIcon() {
    quitAction = new QAction("Quit", this);
    quitAction->setIcon(QtIconLoader::icon("application-exit"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));

    hideRestoreAction = new QAction("Hide", this);
    connect(hideRestoreAction, SIGNAL(triggered()), this, SLOT(doHideRestoreAction()));

    systemTrayMenu = new QMenu(this);
    systemTrayMenu->addSeparator();
    systemTrayMenu->addAction(hideRestoreAction);
    systemTrayMenu->addAction(quitAction);

    systemTrayIcon = new QSystemTrayIcon(QIcon(":icons/lighttasks.png"), this);
    systemTrayIcon->setContextMenu(systemTrayMenu);
    updateSystemTrayToolTip();
    systemTrayIcon->show();
    connect(systemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(systemTrayActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::systemTrayActivated(QSystemTrayIcon::ActivationReason activationReason) {
    if(activationReason == QSystemTrayIcon::Trigger) {
        doHideRestoreAction();
    }
}

void MainWindow::mainOperationButtonClicked() {

    if(state == NORMAL) {
        Task* task = new Task();
        task->setName("New Task");
        TaskItem *taskItem = addTaskItem(task);
        taskItem->valid = false;
        taskItem->taskButton->startRenaming();


    } else if (state == EDITING) {
        for(int i=0; i < taskItems.size(); i++) {
            taskItems[i]->taskButton->cancelEditing();
        }

        changeState(NORMAL);
    }

}

MainWindow::TaskItem* MainWindow::addTaskItem(Task *task) {
    TaskButton *taskButton = new TaskButton(this);
    taskButton->setTask(task);

    connect(&tickTimer, SIGNAL(timeout()), taskButton, SLOT(tick()));
    connect(taskButton, SIGNAL(startedEditing()), this, SLOT(taskStartedEditing()));
    connect(taskButton, SIGNAL(cancelledEditing()), this, SLOT(taskCancelledEditing()));
    connect(taskButton, SIGNAL(finishedEditing()), this, SLOT(taskFinishedEditing()));
    connect(taskButton, SIGNAL(deleted()), this, SLOT(taskDeleted()));
    connect(taskButton, SIGNAL(movedUp()), this, SLOT(taskMovedUp()));
    connect(taskButton, SIGNAL(movedDown()), this, SLOT(taskMovedDown()));


    ui->taskListLayout->insertWidget(0, taskButton);
    ui->taskListScrollArea->ensureWidgetVisible(taskButton);
    setTabOrder(ui->mainOperationButton, taskButton); // We insert to top rather than bottom, so we need to manually set tab order

    QAction *trayAction = new QAction(task->getName(), this);
    trayAction->setCheckable(true);
    connect(taskButton, SIGNAL(activated(bool)), trayAction, SLOT(setChecked(bool)));
    connect(trayAction, SIGNAL(toggled(bool)), taskButton, SLOT(setActive(bool)));
    systemTrayMenu->insertAction(systemTrayMenu->actions().first(), trayAction);

    TaskItem *taskItem = new TaskItem();
    taskItem->task = task;
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
    TaskItem *taskItem = getTaskItemFromButton(taskButton);

    if(taskItem->valid == false) {
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

    taskItem->valid = true;

    QString newTaskName = taskItem->task->getName();
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

        Task* task = new Task();
        task->setName(settings.value("name").toString());
        task->setTime(settings.value("time").toInt());
        TaskItem *taskItem = addTaskItem(task);
        taskItem->valid = true;
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

        Task* task = taskItems[i]->task;
        settings.setValue("name", task->getName());
        settings.setValue("time", (qulonglong)task->getTime());
    }
    settings.endArray();

    settings.sync();
}


void MainWindow::updateSystemTrayToolTip() {
    QList<TaskItem*> activeTaskItems;

    foreach(TaskItem *taskItem, taskItems) {
        if(taskItem->task->isActive()) {
            activeTaskItems.append(taskItem);
        }
    }

    QString tooltip;

    if(activeTaskItems.count() == 0) {
        tooltip = "No tasks active";
    } else {
        for(int i=0; i < activeTaskItems.size(); i++) {
            QString taskText = activeTaskItems[i]->task->toText();
            tooltip += taskText;
            if(i < activeTaskItems.size() - 1) {
                tooltip += '\n';
            }
        }
    }

    systemTrayIcon->setToolTip(tooltip);
}


void MainWindow::doHideRestoreAction() {
    if(this->isVisible()) {
        this->hide();
    } else {
        this->restore();
    }

}

bool MainWindow::event(QEvent *event) {
    if(event->type() == QEvent::Close) {
        if(!reallyQuit) {
            QCloseEvent *closeEvent = static_cast<QCloseEvent*>(event);
            this->hide();
            closeEvent->ignore();
            return true;
        }
    } else if(event->type() == QEvent::Resize || event->type() == QEvent::Move) {
        // Windows will sometimes resize a window to 0x0 when it is hidden
        if(this->geometry().width() > 0 && this->geometry().height() > 0) {
            oldGeometry = this->geometry();
        }

    } else if(event->type() == QEvent::Hide && !event->spontaneous()) {
        // We need the !event->spontaneous() condition because QEvent::Hide also happens when minimizing
        hideRestoreAction->setText("Show");

    } else if(event->type() == QEvent::Show && !event->spontaneous()) {
        // We need the !event->spontaneous() condition because QEvent::Show also happens when restoring
        hideRestoreAction->setText("Hide");

    } else if(event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->modifiers() & Qt::ControlModifier) {
            int key = keyEvent->key();
            switch(key) {
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
                int num = key - Qt::Key_1;
                if(taskItems.size() > num) {
                    taskItems[num]->task->toggle();
                }
            }
        }
    }

    return QMainWindow::event(event);
}

void MainWindow::restore() {
    this->show();
    this->activateWindow();
    this->setGeometry(oldGeometry);
}

void MainWindow::quit() {
    reallyQuit = true;
    this->close();
}


