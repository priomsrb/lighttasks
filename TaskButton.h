#ifndef TASKBUTTON_H
#define TASKBUTTON_H

#include <QAction>
#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include "Task.h"
#include "TimeEditWidget.h"

class TaskButton : public QWidget
{
    Q_OBJECT
public:
    TaskButton(QWidget *parent = 0);
    void init(Task task);

    void setValid(bool valid);
    bool isValid() const;
    bool isEditing() const;
    void startRenaming();
    void startSettingTime();

    const Task getTask();

public slots:
    void tick();
    void cancelEditing();
    void setActive(bool active);


signals:
//    void startedRenaming() const;
//    void startedInitialNaming() const;
//    void cancelledRenaming() const;
//    void cancelledInitialNaming() const;
//    void finishedRenaming() const;
//    void finishedInitialNaming() const;
    void startedEditing();
    void finishedEditing();
    void cancelledEditing();
    void movedUp();
    void movedDown();
    void deleted();
    void activated(bool active);

private slots:
    void onButtonClick();
    void finishRenaming();
    void finishSettingTime();

    void doMoveUpAction();
    void doMoveDownAction();
    void doRenameAction();
    void doSetTimeAction();
    void doResetAction();
    void doDeleteAction();



private:

    enum State {
        NORMAL,
        EDITING
    };

    Task task;
    State state;
    bool valid;

    QBoxLayout *layout;
    QPushButton *button;
    QLineEdit *lineEdit;
    TimeEditWidget *timeEdit;

    QAction *moveUpAction;
    QAction *moveDownAction;
    QAction *renameAction;
    QAction *resetAction;
    QAction *setTimeAction;
    QAction *deleteAction;

    void changeState(State newState);
    void createWidgets();
    void createActions();
    QString getButtonText();
    void updateButton();
    void updateLineEdit();
    void updateTimeEdit();

    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // TASKBUTTON_H
