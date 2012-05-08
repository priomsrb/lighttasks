#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QString>

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QObject *parent = 0);

    void toggle();
    void setActive(bool active);
    void setName(const QString name);
    void setTime(int time);

    bool isActive() const;
    QString getName() const;
    int getTime() const;
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
