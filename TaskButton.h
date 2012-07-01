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
    ~TaskButton();

    void setTask(Task *task);

    void startRenaming();
    void startSettingTime();
    bool isEditing() const;

public slots:
    void tick();
    void setActive(bool active);
    void cancelEditing();


signals:
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
    void doStatisticsAction();
    void doRenameAction();
    void doSetTimeAction();
    void doResetAction();
    void doDeleteAction();
    void updateButton();



private:

    enum State {
        NORMAL,
        EDITING
    };

    Task *task;
    State state;

    QBoxLayout *layout;
    QPushButton *button;
    QLineEdit *lineEdit;
    TimeEditWidget *timeEdit;

    QAction *moveUpAction;
    QAction *moveDownAction;
    QAction *statisticsAction;
    QAction *renameAction;
    QAction *resetAction;
    QAction *setTimeAction;
    QAction *deleteAction;

    void changeState(State newState);
    void createWidgets();
    void createActions();
    void updateLineEdit();
    void updateTimeEdit();

    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // TASKBUTTON_H
