#include "Task.h"

Task::Task(QObject *parent)
    : QObject(parent)
    , name("")
    , id(-1)
    , time(0)
    , timeOfToggle(0)
    , active(false)
{
}

Task::~Task() {
}


void Task::setId(const int id) {
    this->id = id;
}

void Task::setName(const QString name) {
    this->name = name;
}

void Task::setTime(const int time) {
    // We temporarily stop the timer when setting its time.
    // Otherwise the difference between the timer's initial value and its
    // stopping time would not equal its duration

    bool wasActive = isActive();

    setActive(false);

    this->time = time;
    timeOfToggle = time;

    setActive(wasActive);
}

void Task::setActive(bool active) {
    bool isToggled = (this->active != active);

    this->active = active;

    if(isToggled) {
        timeOfToggle = time;

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

int Task::getId() const {
    return id;
}

QString Task::getName() const {
    return name;
}

int Task::getTotalTime() const {
    return time;
}

int Task::getCurrentDuration() const {
    return time - timeOfToggle;
}

bool Task::isActive() const {
    return active;
}

QString Task::toText() const {
    int time = getTotalTime();
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
