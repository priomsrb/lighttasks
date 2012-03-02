#ifndef TASK_H
#define TASK_H

#include <QString>

class Task
{
public:
    Task();

    void start();
    void stop();

    void setName(const QString name);
    void setTime(int time);

    void tick();

    QString getName() const;
    int getTime() const;

    QString toText() const;

    bool isActive() const;

private:
    QString name;
    int time;
    bool active;
};

#endif // TASK_H
