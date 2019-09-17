#ifndef ELISTWIDGET_H
#define ELISTWIDGET_H

#include <QStyledItemDelegate>
#include <QListWidget>
#include <QPainter>
#include <QDebug>

class EListWidget : public QListWidget
{
    public:
        EListWidget(QWidget* parent) : QListWidget(parent) { }

    private:
        virtual void wheelEvent(QWheelEvent* event);
        virtual void viewportEntered();
};

void EListWidget::wheelEvent(QWheelEvent* event)
{
    // to do any processing here if you want to do something before scrolling
    //qDebug() << "Mouse scroll captured";

    // call base implementation
    QListWidget::wheelEvent(event);

    // to do some processing after scrolling.
}

void EListWidget::viewportEntered()
{
    qDebug() << "ViewPort entered";
}

class Delegate : public QStyledItemDelegate
{
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

void Delegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    //qDebug() << "Painter";

    if ((option.state & QStyle::State_Selected) || (option.state & QStyle::State_MouseOver))
    {
        QStyledItemDelegate::paint(painter, option, index);

        // get the color to paint with
        //QVariant var = index.model()->data(index, Qt::BackgroundRole);

        // draw the row and its content
        //painter->fillRect(option.rect, var.value<QColor>());
        //painter->drawText(option.rect, index.model()->data(index, Qt::DisplayRole).toString());
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }

    // ...
}

#endif // ELISTWIDGET_H

// End of file
