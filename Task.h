#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QString>

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QObject *parent = 0);

    void setName(const QString name);
    void setTime(int time);
    void setActive(bool active);
    void toggle();

    QString getName() const;
    int getTime() const;
    bool isActive() const;
    QString toText() const;

    void tick();

signals:
    void toggled(bool newState);

private:
    QString name;
    int time;
    bool active;
};

#endif // TASK_H
