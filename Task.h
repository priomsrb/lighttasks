#ifndef TASK_H
#define TASK_H

#include <QString>

class Task
{
public:
    Task();

    void setActive(bool active);
    void setName(const QString name);
    void setTime(int time);


    bool isActive() const;
    QString getName() const;
    int getTime() const;
    QString toText() const;

    void tick();

private:
    QString name;
    int time;
    bool active;
};

#endif // TASK_H
