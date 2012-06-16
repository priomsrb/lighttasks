#include "Task.h"

Task::Task(QObject *parent)
    : QObject(parent)
    , name("")
    , time(0)
    , active(false)
{
}

void Task::setName(const QString name) {
    this->name = name;
}

void Task::setTime(const int time) {
    this->time = time;
}

void Task::setActive(bool active) {
    bool isToggled = (this->active != active);

    this->active = active;

    if(isToggled) {
        emit toggled(active);
    }
}

void Task::toggle() {
    setActive(!isActive());
}

void Task::tick() {
    if(active) {
        time++;
    }
}

QString Task::getName() const {
    return name;
}

int Task::getTime() const {
    return time;
}

bool Task::isActive() const {
    return active;
}

QString Task::toText() const {
    int time = getTime();
    int seconds = time % 60;
    int minutes = (time / 60) % 60;
    int hours = time / 3600;
    QString timeString = QString("%1:%2:%3")
            .arg(QString::number(hours), 2, '0')
            .arg(QString::number(minutes), 2, '0')
            .arg(QString::number(seconds), 2, '0');

    return QString("%1 (%2)")
            .arg(getName())
            .arg(timeString);
}
