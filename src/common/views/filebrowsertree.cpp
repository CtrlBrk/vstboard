#include "filebrowsertree.h"

FileBrowserTree::FileBrowserTree(QWidget *parent) :
    QTreeView(parent)
{

}


void FileBrowserTree::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::BackButton) {
        emit historyBack();
        event->setAccepted(true);
        return;
    }
    if (event->button() == Qt::ForwardButton) {
        emit historyForward();
        event->setAccepted(true);
        return;
    }

    QTreeView::mouseReleaseEvent(event);
}
