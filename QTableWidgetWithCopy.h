#ifndef QTABLEWIDGETWITHCOPY_H
#define QTABLEWIDGETWITHCOPY_H


// Adapted from http://stackoverflow.com/a/2180913/311792

#include <QTableWidget>

// QTableWidget with support for copying multiple cells
class QTableWidgetWithCopy: public QTableWidget
{
public:
    QTableWidgetWithCopy(QWidget *parent)
        : QTableWidget(parent)
    {
    }

private:
    void copy();

protected:
    virtual void keyPressEvent(QKeyEvent * event);
};


#endif // QTABLEWIDGETWITHCOPY_H
