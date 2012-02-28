#include <QKeyEvent>
#include <QTimer>
#include "TimeEditWidget.h"
#include "ui_TimeEditWidget.h"

TimeEditWidget::TimeEditWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeEditWidget)
{
    ui->setupUi(this);
    ui->secondsSpinBox->installEventFilter(this);
    ui->minutesSpinBox->installEventFilter(this);
    ui->hoursSpinBox->installEventFilter(this);
}

TimeEditWidget::~TimeEditWidget()
{
    delete ui;
}


void TimeEditWidget::setTime(int time) {
    int seconds = time % 60;
    int minutes = (time / 60) % 60;
    int hours = time / 3600;

    ui->secondsSpinBox->setValue(seconds);
    ui->minutesSpinBox->setValue(minutes);
    ui->hoursSpinBox->setValue(hours);


}

void TimeEditWidget::setFocus() {
    ui->hoursSpinBox->setFocus();
    ui->hoursSpinBox->selectAll();
}

int TimeEditWidget::getTime() {
    int seconds = ui->secondsSpinBox->value();
    int minutes = ui->minutesSpinBox->value();
    int hours = ui->hoursSpinBox->value();

    int time = seconds + 60 * minutes + 3600 * hours;

    return time;
}

bool TimeEditWidget::eventFilter(QObject *obj, QEvent *event) {
    if(obj == ui->secondsSpinBox || obj == ui->minutesSpinBox || obj == ui->hoursSpinBox) {
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if(keyEvent->key() == Qt::Key_Return) {
                emit returnPressed();
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}
