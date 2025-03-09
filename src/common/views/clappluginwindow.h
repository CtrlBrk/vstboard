#ifndef CLAPPLUGINWINDOW_H
#define CLAPPLUGINWINDOW_H

namespace Ui {
class VstPluginWindow;
}

namespace Connectables {
class Object;
class ClapPlugin;

}

class ObjectContainerAttribs;

namespace View {

class ClapPluginWindow : public QFrame
{
    Q_OBJECT

public:
    ClapPluginWindow(QWidget *parent = 0);
    ~ClapPluginWindow();
    WId GetWinId();

    void LoadAttribs(const ObjectContainerAttribs &attr);
    void SaveAttribs(ObjectContainerAttribs &attr);


private:
    void resizeEvent ( QResizeEvent * event ) override;
    void closeEvent ( QCloseEvent * event ) override;
    Ui::VstPluginWindow *ui;
    bool canResize;

signals:
    void Hide();

public slots:
    void SetWindowSize(int newWidth, int newHeight);

};

}
#endif // CLAPPLUGINWINDOW_H
