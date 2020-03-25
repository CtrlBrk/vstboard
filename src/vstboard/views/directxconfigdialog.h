#ifndef DIRECTXCONFIGDIALOG_H
#define DIRECTXCONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class DirectxConfigDialog;
}

class MainHost;
class DirectxConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DirectxConfigDialog( MainHost *myHost, QWidget *parent = nullptr);
    ~DirectxConfigDialog();

private:
    Ui::DirectxConfigDialog *ui;
    MainHost *myHost;

public slots:
    void accept();


};

#endif // DIRECTXCONFIGDIALOG_H
