#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QList>
#include <QSystemTrayIcon>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class Task;
class TaskButton;
class TaskLogger;

struct TaskItem {
    Task *task;
    TaskButton *taskButton;
    QAction *trayAction;
    bool valid;
};



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void mainOperationButtonClicked();
    void alwaysOnTopToggled(bool alwaysOnTop);
    void taskStartedEditing();
    void taskCancelledEditing();
    void taskFinishedEditing();
    void taskDeleted();
    void taskMovedUp();
    void taskMovedDown();
    void saveSettings();
    void systemTrayActivated(QSystemTrayIcon::ActivationReason activationReason);
    void updateSystemTrayToolTip();
    void updateIcon();
    void doHideRestoreAction();
    void showHistory();
    void restore();
    void quit();

private:

    enum State {
        NORMAL,
        EDITING
    };


    Ui::MainWindow *ui;
    QList<TaskItem*> taskItems;
    TaskLogger *taskLogger;
    QTimer tickTimer;
    QTimer saveTimer;
    State state;
    QSystemTrayIcon *systemTrayIcon;
    QIcon normalIcon;
    QIcon taskActiveIcon;
    QMenu *systemTrayMenu;
    QAction *hideRestoreAction;
    QRect oldGeometry;
    bool reallyQuit;
    bool alwaysOnTop;

    void addNewTask();
    TaskItem *createTaskItem(Task *task);
    void changeState(State newState);
    void loadSettings();
    void setupTrayIcon();
    void removeTaskItem(TaskItem* taskItem);
    TaskItem* getTaskItemFromButton(TaskButton* taskButton);
    void moveTaskItem(int fromIndex, int toIndex);
    bool event(QEvent *event);
    int generateNewTaskId();

};

#endif // MAINWINDOW_H
