#ifndef ECHECKBOX_H
#define ECHECKBOX_H

#include <QCheckBox>
#include <QMouseEvent>
#include <QStyle>
#include <QRect>
#include <QStyleOptionButton>
#include <QDebug>

class ECheckBox : public QCheckBox
{
    Q_OBJECT

    public:
        ECheckBox(QWidget* parent = nullptr) : QCheckBox(parent)
        {
            this->setMouseTracking(true);
            this->setAttribute(Qt::WA_Hover);
            //this->setCursor(Qt::PointingHandCursor);
            this->installEventFilter(this);

            labelClicked = false;
            checkBoxClicked = false;
        }

        bool IsLabelClicked() { return labelClicked; }
        bool IsCheckBoxClicked() { return checkBoxClicked; }

    signals:
        void Click_signal(bool labelClick, bool checkBoxClick);

    private:
        virtual bool eventFilter(QObject* obj, QEvent* event)
        {
            //qDebug() << "My event filter has been hit (1)";

            QMouseEvent* mouseEvent = static_cast<QMouseEvent *> (event);

            if (event->type() == QEvent::MouseButtonPress && mouseEvent->button()==Qt::LeftButton)
            {
                qDebug() << "My event filter has been hit (2)";

                QStyleOptionButton option;

                option.initFrom(this);

                // Define a rect for the label
                QRect rect = this->style()->subElementRect(QStyle::SE_CheckBoxContents, &option, this);

                // Check if we hit the label
                if (rect.contains(mouseEvent->pos()))
                {
                    qDebug() << "My event filter has been hit (3)";

                    checkBoxClicked = false;
                    labelClicked = true;
                }
                else
                {
                    checkBoxClicked = true;
                    labelClicked = false;
                }

                emit Click_signal(labelClicked, checkBoxClicked);

                return true;
            }

            return QWidget::eventFilter(obj, event);
        }

        bool labelClicked;
        bool checkBoxClicked;
};

#endif // ECHECKBOX_H

// End of file
