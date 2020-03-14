#ifndef KEYPRESSEDWIDGET_H
#define KEYPRESSEDWIDGET_H

#include <QLineEdit>

class KeypressedWidget : public QLineEdit
{
    Q_OBJECT
public:
    explicit KeypressedWidget(QWidget *parent = 0);
    ~KeypressedWidget();
protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // KEYPRESSEDWIDGET_H
