#ifndef TASKITEM_H
#define TASKITEM_H

class Task;
class TaskButton;
class QAction;

struct TaskItem {
    Task *task;
    TaskButton *taskButton;
    QAction *trayAction;
    bool valid;
};


#endif // TASKITEM_H
