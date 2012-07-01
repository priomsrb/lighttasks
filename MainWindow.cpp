#include <QDebug>
#include <QMenu>
#include <QSettings>
#include <QWindowStateChangeEvent>
#include "qticonloader.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "TaskButton.h"
#include "TaskLogger.h"
#include "TaskHistoryDialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , state(NORMAL)
    , reallyQuit(false)
    , alwaysOnTop(false)
{
    ui->setupUi(this);

    connect(ui->mainOperationButton, SIGNAL(clicked()), this, SLOT(mainOperationButtonClicked()));
    connect(ui->actionAlwaysOnTop, SIGNAL(toggled(bool)), this, SLOT(alwaysOnTopToggled(bool)));
    connect(ui->actionShowHistory, SIGNAL(triggered()), this, SLOT(showHistory()));
    connect(&saveTimer, SIGNAL(timeout()), this, SLOT(saveSettings()));
    connect(&tickTimer, SIGNAL(timeout()), this, SLOT(updateSystemTrayToolTip()));

    TaskHistoryDialog::setTaskItems(&taskItems);
    taskLogger = TaskLogger::getInstance();

    setupTrayIcon();
    loadSettings();

    saveTimer.start(60000);
    tickTimer.start(1000);
}


MainWindow::~MainWindow() {
    for(int i=0; i < taskItems.size(); i++) {
        taskItems[i]->task->setActive(false);
    }

    saveSettings();
    taskLogger->destroyInstance();
    delete ui;
}


void MainWindow::setupTrayIcon() {
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quit()));

    hideRestoreAction = new QAction("Hide", this);
    connect(hideRestoreAction, SIGNAL(triggered()), this, SLOT(doHideRestoreAction()));

    systemTrayMenu = new QMenu(this);
    systemTrayMenu->addSeparator();
    systemTrayMenu->addAction(hideRestoreAction);
    systemTrayMenu->addAction(ui->actionQuit);

    normalIcon.addFile(":icons/lighttasks.png");
    taskActiveIcon.addFile(":icons/lighttasks_task_active.png");

    systemTrayIcon = new QSystemTrayIcon(normalIcon, this);
    systemTrayIcon->setContextMenu(systemTrayMenu);
    updateSystemTrayToolTip();
    systemTrayIcon->show();
    connect(systemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(systemTrayActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::systemTrayActivated(QSystemTrayIcon::ActivationReason activationReason) {
    if(activationReason == QSystemTrayIcon::Trigger ||
            activationReason == QSystemTrayIcon::DoubleClick) {
        doHideRestoreAction();
    }
}

void MainWindow::mainOperationButtonClicked() {
    if(state == NORMAL) {
        addNewTask();
    } else if (state == EDITING) {
        for(int i=0; i < taskItems.size(); i++) {
            taskItems[i]->taskButton->cancelEditing();
        }
    }
}

void MainWindow::alwaysOnTopToggled(bool alwaysOnTop) {
    this->alwaysOnTop = alwaysOnTop;

    Qt::WindowFlags flags = this->windowFlags();

    if (alwaysOnTop) {
        this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    } else {
        this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
    }

    this->show();
}

void MainWindow::addNewTask() {
    Task* task = new Task();
    task->setName("New Task");
    TaskItem *taskItem = createTaskItem(task);
    taskItem->valid = false;
    taskItem->taskButton->startRenaming();
}

TaskItem* MainWindow::createTaskItem(Task *task) {
    if(task->getId() == -1) {
        task->setId(generateNewTaskId());
    }

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

    // We insert to top rather than bottom, so we need to manually set tab order
    setTabOrder(ui->mainOperationButton, taskButton);

    QAction *trayAction = new QAction(task->getName(), this);
    trayAction->setCheckable(true);
    connect(task, SIGNAL(toggled(bool)), trayAction, SLOT(setChecked(bool)));
    connect(trayAction, SIGNAL(toggled(bool)), taskButton, SLOT(setActive(bool)));
    connect(task, SIGNAL(toggled(bool)), this, SLOT(updateIcon()));
    systemTrayMenu->insertAction(systemTrayMenu->actions().first(), trayAction);

    taskLogger->addTask(task);

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
    TaskButton *taskButton = static_cast<TaskButton*>(sender());
    TaskItem *taskItem = getTaskItemFromButton(taskButton);

    if(taskItem->valid == false) {
        removeTaskItem(taskItem);
    }

    changeState(NORMAL);
}


void MainWindow::removeTaskItem(TaskItem* taskItem) {
    taskItem->taskButton->deleteLater();
    taskItem->trayAction->deleteLater();

    taskItem->task->setActive(false); // Make sure all slots fire for task toggling
    taskLogger->deleteTaskHistory(taskItem->task);
    delete taskItem->task;

    taskItems.removeOne(taskItem);
    delete taskItem;
}


TaskItem* MainWindow::getTaskItemFromButton(TaskButton* taskButton) {
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
    QSettings settings;

    QSize windowSize = settings.value("windowSize", QSize(234, 330)).toSize();
    resize(windowSize);

    alwaysOnTop = settings.value("alwaysOnTop", false).toBool();
    ui->actionAlwaysOnTop->setChecked(alwaysOnTop);

    int numTasks = settings.beginReadArray("tasks");

    // We load the tasks in reverse order because the tasks are added like a stack
    for(int i=numTasks - 1; i >= 0; i--) {
        settings.setArrayIndex(i);

        Task* task = new Task();
        task->setName(settings.value("name", "New Task").toString());
        task->setId(settings.value("id", -1).toInt());
        task->setTime(settings.value("time", 0).toInt());
        TaskItem *taskItem = createTaskItem(task);
        taskItem->valid = true;
    }
    settings.endArray();
}


void MainWindow::saveSettings() {
    QSettings settings;
    settings.clear();

    settings.setValue("windowSize", size());
    settings.setValue("alwaysOnTop", alwaysOnTop);

    int numTasks = taskItems.size();
    settings.beginWriteArray("tasks", numTasks);

    for(int i=0; i < numTasks; i++) {
        settings.setArrayIndex(i);

        Task* task = taskItems[i]->task;
        settings.setValue("name", task->getName());
        settings.setValue("id", task->getId());
        settings.setValue("time", task->getTotalTime());
    }
    settings.endArray();

    settings.sync();
}


void MainWindow::updateIcon() {
    static bool prevTasksActive = false;
    bool tasksActive = false;

    foreach(TaskItem *taskItem, taskItems) {
        if(taskItem->task->isActive()) {
            tasksActive = true;
            break;
        }
    }

    if(tasksActive != prevTasksActive) {
        if(tasksActive) {
            setWindowIcon(taskActiveIcon);
            systemTrayIcon->setIcon(taskActiveIcon);
        } else {
            setWindowIcon(normalIcon);
            systemTrayIcon->setIcon(normalIcon);
        }
    }

    prevTasksActive = tasksActive;

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

void MainWindow::showHistory() {
    TaskHistoryDialog dialog(this);
    dialog.exec();
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
            case Qt::Key_9: {
                int num = key - Qt::Key_1;
                if(taskItems.size() > num) {
                    taskItems[num]->task->toggle();
                }
                return true;
            }

            case Qt::Key_T:
            {
                if(state == NORMAL) {
                    this->addNewTask();
                }
                return true;
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


int MainWindow::generateNewTaskId() {
   // Task IDs are generated by finding the lowest available ID

    QVector<bool> idsTaken(taskItems.size() + 1, false);

    for(int i=0; i < taskItems.size(); i++) {
        int taskId = taskItems[i]->task->getId();
        if(taskId < taskItems.size()) {
            idsTaken[taskId] = true;
        }
    }

    for(int i=0; i < idsTaken.size(); i++) {
        if(idsTaken[i] == false) {
            return i;
        }
    }

    // Should never reach here
    return -1;
}

