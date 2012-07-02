#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include "QTableWidgetWithCopy.h"

void QTableWidgetWithCopy::copy() {
    QItemSelectionModel * selection = selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();

    if(indexes.size() < 1)
        return;

    // QModelIndex::operator < sorts first by row, then by column.
    // this is what we need
    std::sort(indexes.begin(), indexes.end());

    // You need a pair of indexes to find the row changes
    QModelIndex previous = indexes.first();
    indexes.removeFirst();
    QString selected_text;
    QModelIndex current;
    Q_FOREACH(current, indexes) {
        QVariant data = model()->data(previous);
        QString text = data.toString();
        // At this point `text` contains the text in one cell
        selected_text.append(text);
        // If you are at the start of the row the row number of the previous index
        // isn't the same.  Text is followed by a row separator, which is a newline.
        if (current.row() != previous.row()) {
            selected_text.append(QLatin1Char('\n'));
        } else {
            // Otherwise it's the same row, so append a column separator, which is a tab.
            selected_text.append(QLatin1Char('\t'));
        }
        previous = current;
    }

    // add last element
    selected_text.append(model()->data(current).toString());
    selected_text.append(QLatin1Char('\n'));
    QApplication::clipboard()->setText(selected_text);
}


void QTableWidgetWithCopy::keyPressEvent(QKeyEvent * event) {
    if(event->matches(QKeySequence::Copy) ) {
        copy();
    } else {
        QTableWidget::keyPressEvent(event);
    }

}
