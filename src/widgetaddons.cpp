#include "widgetaddons.h"


void applyExpandingTabsStyle(QTabWidget* target) {

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
}



// I did not write any of the code (whose purpose is to enable rich text in QTreeWidget) below myself.
// look in header to see the source
void HTMLDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    painter->save();

    QTextDocument doc;
    doc.setHtml(options.text);

    options.text = "";
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    // shift text right to make icon visible
    QSize iconSize = options.icon.actualSize(options.rect.size());
    painter->translate(options.rect.left()+iconSize.width(), options.rect.top());
    QRect clip(0, 0, options.rect.width()+iconSize.width(), options.rect.height());

    painter->setClipRect(clip);
    QAbstractTextDocumentLayout::PaintContext ctx;

    // set text color to red for selected item
    if (option.state & QStyle::State_Selected)
        ctx.palette.setColor(QPalette::Text, QColor("red"));
    ctx.clip = clip;
    doc.documentLayout()->draw(painter, ctx);

    painter->restore();
}

QSize HTMLDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}

