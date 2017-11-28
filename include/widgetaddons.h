#ifndef WIDGETADDONS_H
#define WIDGETADDONS_H

#include <QLineEdit>
#include <QEvent>
#include <QTabWidget>
#include <QLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QStyledItemDelegate>
#include <QTabBar>
#include <QString>
#include <iostream>
#include <QStackedLayout>
#include "math.h"

using namespace std;

void applyExpandingTabsStyle(QTabWidget* target);

/*
 * contains any subclasses/modifications/add-ons to widgets that are necessary
 * in particular, our own implementation of lineedits
*/

// a quick subclass of QLineEdit
class LineEdit: public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(QWidget* parent = 0) : QLineEdit(parent) {;}
    void saveCurrentEntry() { currentEntry = text();}   // managed/called by parent widget
    QString getCurrentEntry() { return currentEntry;}
    void setError(bool errorOccurred) { recentError = errorOccurred;}

signals:
    void receivedFocus();

private:
    QString currentEntry;
    bool recentError = false;

    // I'd love to use PressEvent, but that function's existing implementation is needed
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE { if (recentError)
                                                                    setText(currentEntry);
                                                                 recentError = false;
                                                                }
protected:
    void focusInEvent(QFocusEvent* event) Q_DECL_OVERRIDE { emit receivedFocus();
                                                            QLineEdit::focusInEvent(event);}
};

/*
inline void applyExpandingTabsStyle(QTabWidget* target) {

    int desiredWidth = rint(double(target->size().width())/double(target->count()));
    int desiredHeight = desiredWidth/2.6;
    int reducedHeight = desiredHeight/1.7;
    QString qssCode("QTabBar::tab:!only-one"
                    "{ width: %1px;  "
                    "  height: %2px;}"
                    "QTabBar::tab:only-one"
                    "{ width: %1px;"
                    "  height: %3px;}"
                    );
    target->setStyleSheet(qssCode.arg(desiredWidth).arg(desiredHeight).arg(reducedHeight));
}*/


class TabResizeFilter: public QObject {
    QTabWidget* target;

public:
    TabResizeFilter(QTabWidget* parent) : QObject(parent), target(parent) {;}
    bool eventFilter(QObject *watched, QEvent *event)
    {
        if (event->type() == QEvent::Resize)
            applyExpandingTabsStyle(target);

        return false;  // doesn't hurt to let this trickle through anyways
    }
};


// not currently used.
// copied and pasted from
// http://stackoverflow.com/questions/1956542/how-to-make-item-view-render-rich-html-text-in-qt
// just a tad annoying how QTreeView doesn't automatically support rich text ...
class HTMLDelegate : public QStyledItemDelegate
{
protected:
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};


#endif // WIDGETADDONS_H
