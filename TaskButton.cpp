#include <QDebug>
#include <QBoxLayout>
#include <QTimer>
#include <QEvent>
#include <QKeyEvent>
#include <QMetaMethod>
#include <QMessageBox>
#include <QStyle>
#include "TaskButton.h"

TaskButton::TaskButton(QWidget *parent)
    : QWidget(parent)
{
}

void TaskButton::init(Task task) {
    this->task = task;
    setValid(false);

    createWidgets();
    createActions();
}

void TaskButton::createActions() {
    moveUpAction = new QAction("Move up", this);
    moveUpAction->setIcon(QIcon(":icons/up.png"));
    connect(moveUpAction, SIGNAL(triggered()), this, SLOT(doMoveUpAction()));
    addAction(moveUpAction);

    moveDownAction = new QAction("Move down", this);
    moveDownAction->setIcon(QIcon(":icons/down.png"));
    connect(moveDownAction, SIGNAL(triggered()), this, SLOT(doMoveDownAction()));
    addAction(moveDownAction);

    renameAction = new QAction("Rename", this);
    renameAction->setIcon(QIcon(":icons/rename.png"));
    connect(renameAction, SIGNAL(triggered()), this, SLOT(doRenameAction()));
    addAction(renameAction);

    setTimeAction = new QAction("Set time", this);
    setTimeAction->setIcon(QIcon(":icons/set_time.png"));
    connect(setTimeAction, SIGNAL(triggered()), this, SLOT(doSetTimeAction()));
    addAction(setTimeAction);

    resetAction = new QAction("Reset", this);
    resetAction->setIcon(QIcon(QIcon(":icons/reset.png")));
    connect(resetAction, SIGNAL(triggered()), this, SLOT(doResetAction()));
    addAction(resetAction);

    deleteAction = new QAction("Delete", this);
    deleteAction->setIcon(QIcon(QIcon(":icons/delete.png")));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(doDeleteAction()));
    addAction(deleteAction);

    setContextMenuPolicy(Qt::ActionsContextMenu);
}

void TaskButton::createWidgets() {
    changeState(NORMAL);

    layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setMargin(0);

    button = new QPushButton(this);
    button->setMinimumHeight(50);
    button->installEventFilter(this);
    updateButton();
    layout->addWidget(button);

    lineEdit = new QLineEdit(this);
    lineEdit->setMinimumHeight(50);
    lineEdit->setAlignment(Qt::AlignCenter);
    lineEdit->hide();
    lineEdit->installEventFilter(this);
    updateLineEdit();
    layout->addWidget(lineEdit);

    timeEdit = new TimeEditWidget(this);
    timeEdit->hide();
    timeEdit->setMinimumHeight(50);
    timeEdit->installEventFilter(this);
    updateTimeEdit();
    layout->addWidget(timeEdit);

    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(finishRenaming()));
    connect(timeEdit, SIGNAL(returnPressed()), this, SLOT(finishSettingTime()));
    connect(button, SIGNAL(clicked()), this, SLOT(onButtonClick()));

    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(button);
}

void TaskButton::startRenaming() {
    changeState(EDITING);
    button->hide();
    updateLineEdit();
    lineEdit->show();
    // Set focus right after the event system is free
    QTimer::singleShot(0, lineEdit, SLOT(setFocus()));

    emit startedEditing();
}

void TaskButton::startSettingTime() {
    changeState(EDITING);
    button->hide();
    updateTimeEdit();
    timeEdit->show();
    // Set focus right after the event system is free
    QTimer::singleShot(0, timeEdit, SLOT(setFocus()));

    emit startedEditing();
}


void TaskButton::finishRenaming() {
    changeState(NORMAL);
    task.setName(lineEdit->text().toStdString());
    lineEdit->hide();
    updateButton();
    button->show();

    setValid(true);
    emit finishedEditing();
}


void TaskButton::finishSettingTime() {
    changeState(NORMAL);
    task.setTime(timeEdit->getTime());
    timeEdit->hide();
    updateButton();
    button->show();

    emit finishedEditing();
}

void TaskButton::doMoveUpAction() {
    emit movedUp();
}

void TaskButton::doMoveDownAction() {
    emit movedDown();
}


void TaskButton::doRenameAction() {
    startRenaming();
}

void TaskButton::doSetTimeAction() {
    startSettingTime();
}

void TaskButton::doResetAction() {
    task.setTime(0);
    updateButton();
}

void TaskButton::doDeleteAction() {
    QMessageBox messageBox;
    messageBox.setText("Are you sure you want to delete this task?");
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    messageBox.setDefaultButton(QMessageBox::Yes);
    int answer = messageBox.exec();

    if(answer == QMessageBox::Yes) {
        emit deleted();
    }

}

void TaskButton::setValid(bool valid) {
    this->valid = valid;
}

bool TaskButton::isValid() const {
    return valid;
}

bool TaskButton::isEditing() const {
    if(state == EDITING) {
        return true;
    } else {
        return false;
    }
}

void TaskButton::cancelEditing() {
    changeState(NORMAL);
    lineEdit->hide();
    timeEdit->hide();
    updateButton();
    button->show();

    emit cancelledEditing();
}

void TaskButton::tick() {
    task.tick();
    updateButton();
}

const Task TaskButton::getTask() {
    return task;
}

QString TaskButton::getButtonText() {
    int time = task.getTime();
    int seconds = time % 60;
    int minutes = (time / 60) % 60;
    int hours = time / 3600;
    QString timeString = QString("%1:%2:%3")
            .arg(QString::number(hours), 2, '0')
            .arg(QString::number(minutes), 2, '0')
            .arg(QString::number(seconds), 2, '0');

    return QString("%1 (%2)")
            .arg(task.getName().c_str())
            .arg(timeString);
}

void TaskButton::onButtonClick() {
    if(task.isActive()) {
        setActive(false);
    } else {
        setActive(true);
    }
}

void TaskButton::setActive(bool active) {
    if(active) {
        task.start();
        emit activated(true);
    } else {
        task.stop();
        emit activated(false);
    }

    updateButton();
}

void TaskButton::updateButton() {
    button->setText(getButtonText());

    if(task.isActive()) {
        button->setStyleSheet(
                    "QPushButton {"
                    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #BBFFBB, stop: 1 #77DD77);"
                    "color:#000000;"
                    "border: 1px solid #558855;"
                    "border-radius: 5px;"
                    "}"
                    "QPushButton:hover {"
                    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #CCFFCC, stop: 1 #88EE88);"
                    "}"
                    "QPushButton:pressed {"
                    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #66CC66, stop: 1 #77DD77);"
                    "}"
                    "");
    } else {
        button->setStyleSheet(
                    "QPushButton {"
                    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFDDDD, stop: 1 #FF9999);"
                    "color:#000000;"
                    "border: 1px solid #887777;"
                    "border-radius: 5px;"
                    "}"
                    "QPushButton:hover {"
                    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFEEEE, stop: 1 #FFAAAA);"
                    "}"
                    "QPushButton:pressed {"
                    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FF9999, stop: 1 #FFAAAA);"
                    "}"
                    "");
    }
}

void TaskButton::updateLineEdit() {
    lineEdit->setText(task.getName().c_str());
    lineEdit->selectAll();
}

void TaskButton::updateTimeEdit() {
    timeEdit->setTime(task.getTime());
}

bool TaskButton::eventFilter(QObject *obj, QEvent *event) {
    if(obj == lineEdit) {
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Escape) {
                cancelEditing();
                return true;
            }
        }
    } else if(obj == button) {
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Delete) {
                deleteAction->trigger();
                return true;
            } else if(keyEvent->key() == Qt::Key_F2) {
                renameAction->trigger();
                return true;
            }
        }
    } else if(obj == timeEdit) {
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Escape) {
                cancelEditing();
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}


void TaskButton::changeState(State newState) {
    state = newState;
}
