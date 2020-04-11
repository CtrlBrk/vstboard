#ifndef FILEBROWSERTREE_H
#define FILEBROWSERTREE_H

//#include "precomp.h"

class FileBrowserTree : public QTreeView
{
    Q_OBJECT
public:
    FileBrowserTree(QWidget *parent = 0);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void historyBack();
    void historyForward();
};

#endif // FILEBROWSERTREE_H
