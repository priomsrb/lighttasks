#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QString>

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QObject *parent = 0);
    ~Task();

    void setName(const QString name);
    void setId(int id);
    void setTime(int time);
    void setActive(bool active);
    void toggle();

    QString getName() const;
    int getId() const;
    int getTotalTime() const;
    int getCurrentDuration() const;
    bool isActive() const;

    QString toText() const;

    void tick();

signals:
    void toggled(bool newState);

private:
    QString name;
    int id;     // IDs are needed because names can be changed. An ID of -1 means unassigned
    int time;
    int timeOfToggle;
    bool active;
};

#endif // TASK_H
