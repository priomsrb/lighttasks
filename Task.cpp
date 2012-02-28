#include "Task.h"

Task::Task()
    : name("")
    , time(0)
    , active(false)
{
}

void Task::start() {
    active = true;
}

void Task::stop() {
    active = false;
}

void Task::setTime(const int time) {
    this->time = time;
}

void Task::setName(const std::string name) {
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

std::string Task::getName() const {
    return name;
}

bool Task::isActive() const {
    return active;
}
