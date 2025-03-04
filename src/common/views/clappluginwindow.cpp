#include "clappluginwindow.h"
#include "ui_vstpluginwindow.h"
#include "connectables/clapplugin.h"

using namespace View;


ClapPluginWindow::ClapPluginWindow(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::VstPluginWindow),
    canResize(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::Tool);
    ui->setupUi(this);
}

ClapPluginWindow::~ClapPluginWindow()
{
    delete ui;
}

WId ClapPluginWindow::GetWinId()
{
    return ui->scrollAreaWidgetContents->winId();
}


void ClapPluginWindow::SetWindowSize(int newWidth, int newHeight)
{
    if(!canResize) {
        ui->scrollAreaWidgetContents->setFixedSize(newWidth,newHeight);
        setMaximumSize(newWidth,newHeight);
    }
    resize(newWidth,newHeight);
}
