#ifndef TIMEEDITWIDGET_H
#define TIMEEDITWIDGET_H

#include <QWidget>

namespace Ui {
class TimeEditWidget;
}

class TimeEditWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TimeEditWidget(QWidget *parent = 0);
    ~TimeEditWidget();

    void setTime(int time);
    int getTime();

public slots:
    void setFocus();

signals:
    void returnPressed();
    
private:
    Ui::TimeEditWidget *ui;

    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // TIMEEDITWIDGET_H
