#ifndef TASK_H
#define TASK_H

#include <string>

class Task
{
public:
    Task();

    void start();
    void stop();

    void setName(const std::string name);
    void setTime(int time);

    void tick();

    std::string getName() const;
    int getTime() const;

    bool isActive() const;

private:
    std::string name;
    int time;
    bool active;
};

#endif // TASK_H
