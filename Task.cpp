#include "Task.h"

Task::Task(QObject *parent)
    : QObject(parent)
    , name("")
    , time(0)
    , active(false)
{
}

void Task::toggle() {
    setActive(!isActive());
}

void Task::setActive(bool active) {
    bool toggled = (this->active != active);

    this->active = active;

    if(toggled) {
        emit this->toggled(active);
    }
}

void Task::setTime(const int time) {
    this->time = time;
}

void Task::setName(const QString name) {
    this->name = name;
}

void Task::tick() {
    if(active) {
        time++;
    }
}

int Task::getTime() const {
    return time;
}

QString Task::getName() const {
    return name;
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

bool Task::isActive() const {
    return active;
}
