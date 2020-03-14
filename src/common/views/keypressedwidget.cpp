#include "keypressedwidget.h"

KeypressedWidget::KeypressedWidget(QWidget *parent) :
    QLineEdit(parent)
{
    grabKeyboard();
}

KeypressedWidget::~KeypressedWidget() {
    releaseKeyboard();
}

void KeypressedWidget::keyPressEvent(QKeyEvent *ev)
{
//    ev->key()
//    setText("You Pressed Key " + ev->text());
    QString keyName;

    keyName = QKeySequence(ev->modifiers()).toString();
    int k = ev->key();
    if(k!=0
            && k!=Qt::Key_unknown
            && k!=Qt::Key_Shift
            && k!=Qt::Key_Control
            && k!=Qt::Key_Meta
            && k!=Qt::Key_Alt
            && k!=Qt::Key_CapsLock
            ) {
        keyName += QKeySequence(ev->key()).toString();
    }

    setText(keyName);
}
