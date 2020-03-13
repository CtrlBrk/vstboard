/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QUndoView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "views/filebrowser.h"
#include "views/listaudiodevicesview.h"
#include "views/maingraphicsview.h"
#include "views/programlist.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionSave;
    QAction *actionLoad;
    QAction *actionNew;
    QAction *actionConfig;
    QAction *actionDelete;
    QAction *actionSave_Setup;
    QAction *actionNew_Setup;
    QAction *actionHost_panel;
    QAction *actionAbout;
    QAction *actionSave_Project_As;
    QAction *actionSave_Setup_As;
    QAction *actionGroup_panel;
    QAction *actionProgram_panel;
    QAction *actionRefresh_Audio_devices;
    QAction *actionRefresh_Midi_devices;
    QAction *actionRestore_default_layout;
    QAction *actionTool_bar;
    QAction *actionProject_panel;
    QAction *actionLoad_Setup;
    QAction *actionAppearance;
    QAction *actionCable;
    QAction *actionValue;
    QAction *actionKeyBinding;
    QAction *actionHide_all_editors;
    QAction *actionAutoShowGui;
    QAction *actionUndo;
    QAction *actionRedo;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_9;
    QSplitter *splitterPanels;
    QGroupBox *HostBox;
    QHBoxLayout *horizontalLayout_2;
    MainGraphicsView *hostView;
    QGroupBox *ProjectBox;
    QHBoxLayout *horizontalLayout_5;
    MainGraphicsView *projectView;
    QGroupBox *ProgramBox;
    QHBoxLayout *horizontalLayout;
    QSplitter *splitterProg;
    MainGraphicsView *programView;
    QListView *programParkList;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_3;
    QSplitter *splitterGroup;
    MainGraphicsView *groupView;
    QListView *groupParkList;
    QMenuBar *menuBar;
    QMenu *menuView;
    QMenu *menuEdit;
    QMenu *menuTool;
    QMenu *menuFile;
    QMenu *menuRecent_Setups;
    QMenu *menuRecent_Projects;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dockPrograms;
    QWidget *dockWidgetContents_2;
    QVBoxLayout *verticalLayout_10;
    ProgramList *Programs;
    QDockWidget *dockMidiDevices;
    QWidget *dockWidgetContents_6;
    QVBoxLayout *verticalLayout_8;
    QTreeView *treeMidiInterfaces;
    QDockWidget *dockVstBrowser;
    QWidget *dockWidgetContents_5;
    QVBoxLayout *verticalLayout_7;
    FileBrowser *VstBrowser;
    QDockWidget *dockAudioDevices;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout_6;
    ListAudioDevicesView *treeAudioInterfaces;
    QDockWidget *dockSolver;
    QWidget *dockWidgetContents_4;
    QVBoxLayout *verticalLayout_5;
    QTableWidget *solverView;
    QDockWidget *dockTools;
    QWidget *dockWidgetContents_7;
    QVBoxLayout *verticalLayout_4;
    QTreeView *treeTools;
    QDockWidget *dockHostModel;
    QWidget *dockWidgetContents_8;
    QVBoxLayout *verticalLayout_3;
    QTreeView *treeHostModel;
    QDockWidget *dockBankBrowser;
    QWidget *dockWidgetContents_3;
    QVBoxLayout *verticalLayout;
    FileBrowser *BankBrowser;
    QDockWidget *dockUndo;
    QWidget *dockWidgetContents_9;
    QVBoxLayout *verticalLayout_2;
    QUndoView *listUndo;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(846, 570);
        MainWindow->setFocusPolicy(Qt::StrongFocus);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/vstboard32_8.png"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        MainWindow->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        MainWindow->setToolButtonStyle(Qt::ToolButtonFollowStyle);
        MainWindow->setDockNestingEnabled(true);
        MainWindow->setDockOptions(QMainWindow::AllowNestedDocks|QMainWindow::AllowTabbedDocks|QMainWindow::AnimatedDocks|QMainWindow::VerticalTabs);
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/img16x16/filesave.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon1);
        actionSave->setAutoRepeat(false);
        actionLoad = new QAction(MainWindow);
        actionLoad->setObjectName(QString::fromUtf8("actionLoad"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/img16x16/fileopen.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionLoad->setIcon(icon2);
        actionLoad->setAutoRepeat(false);
        actionNew = new QAction(MainWindow);
        actionNew->setObjectName(QString::fromUtf8("actionNew"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/img16x16/filenew.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionNew->setIcon(icon3);
        actionNew->setAutoRepeat(false);
        actionConfig = new QAction(MainWindow);
        actionConfig->setObjectName(QString::fromUtf8("actionConfig"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/img16x16/configure.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionConfig->setIcon(icon4);
        actionConfig->setAutoRepeat(false);
        actionDelete = new QAction(MainWindow);
        actionDelete->setObjectName(QString::fromUtf8("actionDelete"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/img16x16/delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDelete->setIcon(icon5);
        actionDelete->setAutoRepeat(false);
        actionSave_Setup = new QAction(MainWindow);
        actionSave_Setup->setObjectName(QString::fromUtf8("actionSave_Setup"));
        actionSave_Setup->setIcon(icon1);
        actionSave_Setup->setAutoRepeat(false);
        actionNew_Setup = new QAction(MainWindow);
        actionNew_Setup->setObjectName(QString::fromUtf8("actionNew_Setup"));
        actionNew_Setup->setIcon(icon3);
        actionNew_Setup->setAutoRepeat(false);
        actionHost_panel = new QAction(MainWindow);
        actionHost_panel->setObjectName(QString::fromUtf8("actionHost_panel"));
        actionHost_panel->setCheckable(true);
        actionHost_panel->setChecked(true);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/img16x16/file_setup.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHost_panel->setIcon(icon6);
        actionHost_panel->setAutoRepeat(false);
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/img16x16/documentinfo.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAbout->setIcon(icon7);
        actionAbout->setAutoRepeat(false);
        actionSave_Project_As = new QAction(MainWindow);
        actionSave_Project_As->setObjectName(QString::fromUtf8("actionSave_Project_As"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/img16x16/filesaveas.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave_Project_As->setIcon(icon8);
        actionSave_Project_As->setAutoRepeat(false);
        actionSave_Setup_As = new QAction(MainWindow);
        actionSave_Setup_As->setObjectName(QString::fromUtf8("actionSave_Setup_As"));
        actionSave_Setup_As->setIcon(icon8);
        actionSave_Setup_As->setAutoRepeat(false);
        actionGroup_panel = new QAction(MainWindow);
        actionGroup_panel->setObjectName(QString::fromUtf8("actionGroup_panel"));
        actionGroup_panel->setCheckable(true);
        actionGroup_panel->setChecked(true);
        actionGroup_panel->setIcon(icon6);
        actionGroup_panel->setAutoRepeat(false);
        actionProgram_panel = new QAction(MainWindow);
        actionProgram_panel->setObjectName(QString::fromUtf8("actionProgram_panel"));
        actionProgram_panel->setCheckable(true);
        actionProgram_panel->setChecked(true);
        actionProgram_panel->setIcon(icon6);
        actionProgram_panel->setAutoRepeat(false);
        actionRefresh_Audio_devices = new QAction(MainWindow);
        actionRefresh_Audio_devices->setObjectName(QString::fromUtf8("actionRefresh_Audio_devices"));
        actionRefresh_Audio_devices->setAutoRepeat(false);
        actionRefresh_Midi_devices = new QAction(MainWindow);
        actionRefresh_Midi_devices->setObjectName(QString::fromUtf8("actionRefresh_Midi_devices"));
        actionRefresh_Midi_devices->setAutoRepeat(false);
        actionRestore_default_layout = new QAction(MainWindow);
        actionRestore_default_layout->setObjectName(QString::fromUtf8("actionRestore_default_layout"));
        actionRestore_default_layout->setAutoRepeat(false);
        actionTool_bar = new QAction(MainWindow);
        actionTool_bar->setObjectName(QString::fromUtf8("actionTool_bar"));
        actionTool_bar->setCheckable(true);
        actionTool_bar->setAutoRepeat(false);
        actionProject_panel = new QAction(MainWindow);
        actionProject_panel->setObjectName(QString::fromUtf8("actionProject_panel"));
        actionProject_panel->setCheckable(true);
        actionProject_panel->setChecked(true);
        actionProject_panel->setIcon(icon6);
        actionProject_panel->setAutoRepeat(false);
        actionLoad_Setup = new QAction(MainWindow);
        actionLoad_Setup->setObjectName(QString::fromUtf8("actionLoad_Setup"));
        actionLoad_Setup->setIcon(icon2);
        actionLoad_Setup->setAutoRepeat(false);
        actionAppearance = new QAction(MainWindow);
        actionAppearance->setObjectName(QString::fromUtf8("actionAppearance"));
        actionAppearance->setCheckable(true);
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/img16x16/appearance.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAppearance->setIcon(icon9);
        actionAppearance->setAutoRepeat(false);
        actionCable = new QAction(MainWindow);
        actionCable->setObjectName(QString::fromUtf8("actionCable"));
        actionCable->setCheckable(true);
        actionCable->setChecked(true);
        actionCable->setAutoRepeat(false);
        actionValue = new QAction(MainWindow);
        actionValue->setObjectName(QString::fromUtf8("actionValue"));
        actionValue->setCheckable(true);
        actionValue->setAutoRepeat(false);
        actionKeyBinding = new QAction(MainWindow);
        actionKeyBinding->setObjectName(QString::fromUtf8("actionKeyBinding"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/img16x16/key_bindings.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionKeyBinding->setIcon(icon10);
        actionKeyBinding->setAutoRepeat(false);
        actionHide_all_editors = new QAction(MainWindow);
        actionHide_all_editors->setObjectName(QString::fromUtf8("actionHide_all_editors"));
        actionHide_all_editors->setCheckable(true);
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/img16x16/14_layer_lowerlayer.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHide_all_editors->setIcon(icon11);
        actionHide_all_editors->setAutoRepeat(false);
        actionAutoShowGui = new QAction(MainWindow);
        actionAutoShowGui->setObjectName(QString::fromUtf8("actionAutoShowGui"));
        actionAutoShowGui->setCheckable(true);
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/img16x16/14_layer_raiselayer.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAutoShowGui->setIcon(icon12);
        actionAutoShowGui->setAutoRepeat(false);
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/img16x16/undo.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionUndo->setIcon(icon13);
        actionRedo = new QAction(MainWindow);
        actionRedo->setObjectName(QString::fromUtf8("actionRedo"));
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/img16x16/redo.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRedo->setIcon(icon14);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        verticalLayout_9 = new QVBoxLayout(centralWidget);
        verticalLayout_9->setSpacing(0);
        verticalLayout_9->setContentsMargins(0, 0, 0, 0);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        splitterPanels = new QSplitter(centralWidget);
        splitterPanels->setObjectName(QString::fromUtf8("splitterPanels"));
        splitterPanels->setOrientation(Qt::Vertical);
        splitterPanels->setChildrenCollapsible(false);
        HostBox = new QGroupBox(splitterPanels);
        HostBox->setObjectName(QString::fromUtf8("HostBox"));
        HostBox->setAlignment(Qt::AlignCenter);
        HostBox->setFlat(true);
        horizontalLayout_2 = new QHBoxLayout(HostBox);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        hostView = new MainGraphicsView(HostBox);
        hostView->setObjectName(QString::fromUtf8("hostView"));
        hostView->setFrameShape(QFrame::NoFrame);
        hostView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        hostView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        hostView->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
        hostView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        hostView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

        horizontalLayout_2->addWidget(hostView);

        splitterPanels->addWidget(HostBox);
        ProjectBox = new QGroupBox(splitterPanels);
        ProjectBox->setObjectName(QString::fromUtf8("ProjectBox"));
        ProjectBox->setAlignment(Qt::AlignCenter);
        ProjectBox->setFlat(true);
        horizontalLayout_5 = new QHBoxLayout(ProjectBox);
        horizontalLayout_5->setSpacing(0);
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        projectView = new MainGraphicsView(ProjectBox);
        projectView->setObjectName(QString::fromUtf8("projectView"));
        projectView->setFrameShape(QFrame::NoFrame);
        projectView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        projectView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        projectView->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
        projectView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        projectView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

        horizontalLayout_5->addWidget(projectView);

        splitterPanels->addWidget(ProjectBox);
        ProgramBox = new QGroupBox(splitterPanels);
        ProgramBox->setObjectName(QString::fromUtf8("ProgramBox"));
        ProgramBox->setAlignment(Qt::AlignCenter);
        ProgramBox->setFlat(true);
        horizontalLayout = new QHBoxLayout(ProgramBox);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        splitterProg = new QSplitter(ProgramBox);
        splitterProg->setObjectName(QString::fromUtf8("splitterProg"));
        splitterProg->setOrientation(Qt::Horizontal);
        splitterProg->setHandleWidth(2);
        programView = new MainGraphicsView(splitterProg);
        programView->setObjectName(QString::fromUtf8("programView"));
        programView->setFrameShape(QFrame::NoFrame);
        programView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        programView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        programView->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
        programView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        programView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
        splitterProg->addWidget(programView);
        programParkList = new QListView(splitterProg);
        programParkList->setObjectName(QString::fromUtf8("programParkList"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(programParkList->sizePolicy().hasHeightForWidth());
        programParkList->setSizePolicy(sizePolicy1);
        programParkList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        programParkList->setDragEnabled(true);
        programParkList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        programParkList->setUniformItemSizes(true);
        splitterProg->addWidget(programParkList);

        horizontalLayout->addWidget(splitterProg);

        splitterPanels->addWidget(ProgramBox);
        groupBox = new QGroupBox(splitterPanels);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setAlignment(Qt::AlignCenter);
        groupBox->setFlat(true);
        horizontalLayout_3 = new QHBoxLayout(groupBox);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        splitterGroup = new QSplitter(groupBox);
        splitterGroup->setObjectName(QString::fromUtf8("splitterGroup"));
        splitterGroup->setOrientation(Qt::Horizontal);
        splitterGroup->setHandleWidth(2);
        groupView = new MainGraphicsView(splitterGroup);
        groupView->setObjectName(QString::fromUtf8("groupView"));
        groupView->setFrameShape(QFrame::NoFrame);
        groupView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        groupView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        groupView->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
        groupView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        groupView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
        splitterGroup->addWidget(groupView);
        groupParkList = new QListView(splitterGroup);
        groupParkList->setObjectName(QString::fromUtf8("groupParkList"));
        sizePolicy1.setHeightForWidth(groupParkList->sizePolicy().hasHeightForWidth());
        groupParkList->setSizePolicy(sizePolicy1);
        groupParkList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        groupParkList->setDragEnabled(true);
        groupParkList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        groupParkList->setUniformItemSizes(true);
        splitterGroup->addWidget(groupParkList);

        horizontalLayout_3->addWidget(splitterGroup);

        splitterPanels->addWidget(groupBox);

        verticalLayout_9->addWidget(splitterPanels);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 846, 21));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuTool = new QMenu(menuEdit);
        menuTool->setObjectName(QString::fromUtf8("menuTool"));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuRecent_Setups = new QMenu(menuFile);
        menuRecent_Setups->setObjectName(QString::fromUtf8("menuRecent_Setups"));
        menuRecent_Projects = new QMenu(menuFile);
        menuRecent_Projects->setObjectName(QString::fromUtf8("menuRecent_Projects"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        dockPrograms = new QDockWidget(MainWindow);
        dockPrograms->setObjectName(QString::fromUtf8("dockPrograms"));
        QSizePolicy sizePolicy2(QSizePolicy::Ignored, QSizePolicy::Ignored);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(dockPrograms->sizePolicy().hasHeightForWidth());
        dockPrograms->setSizePolicy(sizePolicy2);
        dockPrograms->setMinimumSize(QSize(80, 50));
        dockPrograms->setFloating(false);
        dockPrograms->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockPrograms->setAllowedAreas(Qt::AllDockWidgetAreas);
        dockWidgetContents_2 = new QWidget();
        dockWidgetContents_2->setObjectName(QString::fromUtf8("dockWidgetContents_2"));
        verticalLayout_10 = new QVBoxLayout(dockWidgetContents_2);
        verticalLayout_10->setSpacing(0);
        verticalLayout_10->setContentsMargins(0, 0, 0, 0);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        Programs = new ProgramList(dockWidgetContents_2);
        Programs->setObjectName(QString::fromUtf8("Programs"));
        QSizePolicy sizePolicy3(QSizePolicy::Ignored, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(Programs->sizePolicy().hasHeightForWidth());
        Programs->setSizePolicy(sizePolicy3);

        verticalLayout_10->addWidget(Programs);

        dockPrograms->setWidget(dockWidgetContents_2);
        MainWindow->addDockWidget(Qt::RightDockWidgetArea, dockPrograms);
        dockMidiDevices = new QDockWidget(MainWindow);
        dockMidiDevices->setObjectName(QString::fromUtf8("dockMidiDevices"));
        dockMidiDevices->setMinimumSize(QSize(80, 91));
        dockMidiDevices->setAcceptDrops(false);
        dockMidiDevices->setFloating(false);
        dockMidiDevices->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockMidiDevices->setAllowedAreas(Qt::AllDockWidgetAreas);
        dockWidgetContents_6 = new QWidget();
        dockWidgetContents_6->setObjectName(QString::fromUtf8("dockWidgetContents_6"));
        verticalLayout_8 = new QVBoxLayout(dockWidgetContents_6);
        verticalLayout_8->setSpacing(0);
        verticalLayout_8->setContentsMargins(0, 0, 0, 0);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        treeMidiInterfaces = new QTreeView(dockWidgetContents_6);
        treeMidiInterfaces->setObjectName(QString::fromUtf8("treeMidiInterfaces"));
        QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(treeMidiInterfaces->sizePolicy().hasHeightForWidth());
        treeMidiInterfaces->setSizePolicy(sizePolicy4);
        treeMidiInterfaces->setContextMenuPolicy(Qt::ActionsContextMenu);
        treeMidiInterfaces->setFrameShape(QFrame::NoFrame);
        treeMidiInterfaces->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        treeMidiInterfaces->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeMidiInterfaces->setProperty("showDropIndicator", QVariant(false));
        treeMidiInterfaces->setDragEnabled(true);
        treeMidiInterfaces->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeMidiInterfaces->setIndentation(10);
        treeMidiInterfaces->setUniformRowHeights(true);
        treeMidiInterfaces->setSortingEnabled(true);
        treeMidiInterfaces->setAnimated(true);
        treeMidiInterfaces->header()->setStretchLastSection(false);

        verticalLayout_8->addWidget(treeMidiInterfaces);

        dockMidiDevices->setWidget(dockWidgetContents_6);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockMidiDevices);
        dockVstBrowser = new QDockWidget(MainWindow);
        dockVstBrowser->setObjectName(QString::fromUtf8("dockVstBrowser"));
        dockVstBrowser->setMinimumSize(QSize(80, 50));
        dockWidgetContents_5 = new QWidget();
        dockWidgetContents_5->setObjectName(QString::fromUtf8("dockWidgetContents_5"));
        verticalLayout_7 = new QVBoxLayout(dockWidgetContents_5);
        verticalLayout_7->setSpacing(0);
        verticalLayout_7->setContentsMargins(0, 0, 0, 0);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        VstBrowser = new FileBrowser(dockWidgetContents_5);
        VstBrowser->setObjectName(QString::fromUtf8("VstBrowser"));
        sizePolicy4.setHeightForWidth(VstBrowser->sizePolicy().hasHeightForWidth());
        VstBrowser->setSizePolicy(sizePolicy4);

        verticalLayout_7->addWidget(VstBrowser);

        dockVstBrowser->setWidget(dockWidgetContents_5);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockVstBrowser);
        dockAudioDevices = new QDockWidget(MainWindow);
        dockAudioDevices->setObjectName(QString::fromUtf8("dockAudioDevices"));
        dockAudioDevices->setMinimumSize(QSize(80, 91));
        dockAudioDevices->setFloating(false);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        verticalLayout_6 = new QVBoxLayout(dockWidgetContents);
        verticalLayout_6->setSpacing(0);
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        treeAudioInterfaces = new ListAudioDevicesView(dockWidgetContents);
        treeAudioInterfaces->setObjectName(QString::fromUtf8("treeAudioInterfaces"));
        sizePolicy4.setHeightForWidth(treeAudioInterfaces->sizePolicy().hasHeightForWidth());
        treeAudioInterfaces->setSizePolicy(sizePolicy4);
        treeAudioInterfaces->setContextMenuPolicy(Qt::CustomContextMenu);
        treeAudioInterfaces->setFrameShape(QFrame::NoFrame);
        treeAudioInterfaces->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        treeAudioInterfaces->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeAudioInterfaces->setProperty("showDropIndicator", QVariant(false));
        treeAudioInterfaces->setDragEnabled(true);
        treeAudioInterfaces->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeAudioInterfaces->setIndentation(10);
        treeAudioInterfaces->setUniformRowHeights(true);
        treeAudioInterfaces->setSortingEnabled(true);
        treeAudioInterfaces->setAnimated(true);
        treeAudioInterfaces->header()->setStretchLastSection(false);

        verticalLayout_6->addWidget(treeAudioInterfaces);

        dockAudioDevices->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockAudioDevices);
        dockSolver = new QDockWidget(MainWindow);
        dockSolver->setObjectName(QString::fromUtf8("dockSolver"));
        sizePolicy2.setHeightForWidth(dockSolver->sizePolicy().hasHeightForWidth());
        dockSolver->setSizePolicy(sizePolicy2);
        dockSolver->setMinimumSize(QSize(80, 162));
        dockWidgetContents_4 = new QWidget();
        dockWidgetContents_4->setObjectName(QString::fromUtf8("dockWidgetContents_4"));
        verticalLayout_5 = new QVBoxLayout(dockWidgetContents_4);
        verticalLayout_5->setSpacing(0);
        verticalLayout_5->setContentsMargins(0, 0, 0, 0);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        solverView = new QTableWidget(dockWidgetContents_4);
        solverView->setObjectName(QString::fromUtf8("solverView"));
        solverView->setAcceptDrops(false);
        solverView->setFrameShape(QFrame::NoFrame);
        solverView->setSelectionMode(QAbstractItemView::NoSelection);
        solverView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        solverView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        solverView->verticalHeader()->setVisible(false);

        verticalLayout_5->addWidget(solverView);

        dockSolver->setWidget(dockWidgetContents_4);
        MainWindow->addDockWidget(Qt::RightDockWidgetArea, dockSolver);
        dockTools = new QDockWidget(MainWindow);
        dockTools->setObjectName(QString::fromUtf8("dockTools"));
        dockTools->setMinimumSize(QSize(80, 91));
        dockWidgetContents_7 = new QWidget();
        dockWidgetContents_7->setObjectName(QString::fromUtf8("dockWidgetContents_7"));
        verticalLayout_4 = new QVBoxLayout(dockWidgetContents_7);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        treeTools = new QTreeView(dockWidgetContents_7);
        treeTools->setObjectName(QString::fromUtf8("treeTools"));
        sizePolicy4.setHeightForWidth(treeTools->sizePolicy().hasHeightForWidth());
        treeTools->setSizePolicy(sizePolicy4);
        treeTools->setFrameShape(QFrame::NoFrame);
        treeTools->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        treeTools->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeTools->setProperty("showDropIndicator", QVariant(false));
        treeTools->setDragDropMode(QAbstractItemView::DragOnly);
        treeTools->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeTools->setIndentation(10);
        treeTools->setUniformRowHeights(true);
        treeTools->setSortingEnabled(false);
        treeTools->setAnimated(true);
        treeTools->setHeaderHidden(true);
        treeTools->header()->setStretchLastSection(false);

        verticalLayout_4->addWidget(treeTools);

        dockTools->setWidget(dockWidgetContents_7);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockTools);
        dockHostModel = new QDockWidget(MainWindow);
        dockHostModel->setObjectName(QString::fromUtf8("dockHostModel"));
        sizePolicy2.setHeightForWidth(dockHostModel->sizePolicy().hasHeightForWidth());
        dockHostModel->setSizePolicy(sizePolicy2);
        dockHostModel->setMinimumSize(QSize(80, 91));
        dockWidgetContents_8 = new QWidget();
        dockWidgetContents_8->setObjectName(QString::fromUtf8("dockWidgetContents_8"));
        verticalLayout_3 = new QVBoxLayout(dockWidgetContents_8);
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        treeHostModel = new QTreeView(dockWidgetContents_8);
        treeHostModel->setObjectName(QString::fromUtf8("treeHostModel"));
        sizePolicy3.setHeightForWidth(treeHostModel->sizePolicy().hasHeightForWidth());
        treeHostModel->setSizePolicy(sizePolicy3);
        treeHostModel->setFrameShape(QFrame::NoFrame);
        treeHostModel->setEditTriggers(QAbstractItemView::EditKeyPressed|QAbstractItemView::SelectedClicked);
        treeHostModel->setDragEnabled(true);
        treeHostModel->setDragDropMode(QAbstractItemView::DragDrop);
        treeHostModel->setDefaultDropAction(Qt::IgnoreAction);
        treeHostModel->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeHostModel->setIndentation(10);
        treeHostModel->setUniformRowHeights(true);
        treeHostModel->setSortingEnabled(true);
        treeHostModel->setAnimated(true);

        verticalLayout_3->addWidget(treeHostModel);

        dockHostModel->setWidget(dockWidgetContents_8);
        MainWindow->addDockWidget(Qt::RightDockWidgetArea, dockHostModel);
        dockBankBrowser = new QDockWidget(MainWindow);
        dockBankBrowser->setObjectName(QString::fromUtf8("dockBankBrowser"));
        dockBankBrowser->setMinimumSize(QSize(80, 50));
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName(QString::fromUtf8("dockWidgetContents_3"));
        verticalLayout = new QVBoxLayout(dockWidgetContents_3);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        BankBrowser = new FileBrowser(dockWidgetContents_3);
        BankBrowser->setObjectName(QString::fromUtf8("BankBrowser"));

        verticalLayout->addWidget(BankBrowser);

        dockBankBrowser->setWidget(dockWidgetContents_3);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, dockBankBrowser);
        dockUndo = new QDockWidget(MainWindow);
        dockUndo->setObjectName(QString::fromUtf8("dockUndo"));
        dockUndo->setMinimumSize(QSize(80, 91));
        dockWidgetContents_9 = new QWidget();
        dockWidgetContents_9->setObjectName(QString::fromUtf8("dockWidgetContents_9"));
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents_9);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        listUndo = new QUndoView(dockWidgetContents_9);
        listUndo->setObjectName(QString::fromUtf8("listUndo"));
        listUndo->setFrameShape(QFrame::NoFrame);
        listUndo->setUniformItemSizes(true);

        verticalLayout_2->addWidget(listUndo);

        dockUndo->setWidget(dockWidgetContents_9);
        MainWindow->addDockWidget(Qt::RightDockWidgetArea, dockUndo);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuView->addAction(actionHost_panel);
        menuView->addAction(actionProject_panel);
        menuView->addAction(actionProgram_panel);
        menuView->addAction(actionGroup_panel);
        menuView->addSeparator();
        menuView->addAction(actionRestore_default_layout);
        menuView->addAction(actionTool_bar);
        menuView->addAction(actionAppearance);
        menuView->addAction(actionAutoShowGui);
        menuView->addAction(actionHide_all_editors);
        menuView->addSeparator();
        menuEdit->addAction(actionConfig);
        menuEdit->addAction(actionKeyBinding);
        menuEdit->addAction(actionRefresh_Audio_devices);
        menuEdit->addAction(actionRefresh_Midi_devices);
        menuEdit->addAction(menuTool->menuAction());
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuTool->addAction(actionCable);
        menuTool->addAction(actionValue);
        menuFile->addAction(actionLoad);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_Project_As);
        menuFile->addAction(actionNew);
        menuFile->addAction(menuRecent_Projects->menuAction());
        menuFile->addSeparator();
        menuFile->addAction(actionLoad_Setup);
        menuFile->addAction(actionSave_Setup);
        menuFile->addAction(actionSave_Setup_As);
        menuFile->addAction(actionNew_Setup);
        menuFile->addAction(menuRecent_Setups->menuAction());
        menuHelp->addAction(actionAbout);
        mainToolBar->addAction(actionLoad);
        mainToolBar->addAction(actionNew);
        mainToolBar->addAction(actionSave);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionHost_panel);
        mainToolBar->addAction(actionProject_panel);
        mainToolBar->addAction(actionProgram_panel);
        mainToolBar->addAction(actionGroup_panel);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionConfig);
        mainToolBar->addAction(actionAppearance);
        mainToolBar->addAction(actionKeyBinding);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionCable);
        mainToolBar->addAction(actionValue);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionAutoShowGui);
        mainToolBar->addAction(actionHide_all_editors);
        mainToolBar->addAction(actionUndo);
        mainToolBar->addAction(actionRedo);

        retranslateUi(MainWindow);
        QObject::connect(actionHost_panel, SIGNAL(toggled(bool)), HostBox, SLOT(setVisible(bool)));
        QObject::connect(actionGroup_panel, SIGNAL(toggled(bool)), groupBox, SLOT(setVisible(bool)));
        QObject::connect(actionProgram_panel, SIGNAL(toggled(bool)), ProgramBox, SLOT(setVisible(bool)));
        QObject::connect(actionTool_bar, SIGNAL(toggled(bool)), mainToolBar, SLOT(setVisible(bool)));
        QObject::connect(actionProject_panel, SIGNAL(toggled(bool)), ProjectBox, SLOT(setVisible(bool)));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "VstBoard", nullptr));
        actionSave->setText(QCoreApplication::translate("MainWindow", "Save Project", nullptr));
#if QT_CONFIG(statustip)
        actionSave->setStatusTip(QCoreApplication::translate("MainWindow", "Save the Program and Group panels in a project file", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionSave->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionLoad->setText(QCoreApplication::translate("MainWindow", "Open Project", nullptr));
#if QT_CONFIG(tooltip)
        actionLoad->setToolTip(QCoreApplication::translate("MainWindow", "Open a Project file", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        actionLoad->setStatusTip(QCoreApplication::translate("MainWindow", "Open a Project file", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionLoad->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionNew->setText(QCoreApplication::translate("MainWindow", "New Project", nullptr));
#if QT_CONFIG(statustip)
        actionNew->setStatusTip(QCoreApplication::translate("MainWindow", "Clear the Program and Group panels", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionNew->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionConfig->setText(QCoreApplication::translate("MainWindow", "Configuration", nullptr));
#if QT_CONFIG(statustip)
        actionConfig->setStatusTip(QCoreApplication::translate("MainWindow", "Show the configuration dialog", nullptr));
#endif // QT_CONFIG(statustip)
        actionDelete->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
#if QT_CONFIG(statustip)
        actionDelete->setStatusTip(QCoreApplication::translate("MainWindow", "Delete", nullptr));
#endif // QT_CONFIG(statustip)
        actionSave_Setup->setText(QCoreApplication::translate("MainWindow", "Save Setup", nullptr));
#if QT_CONFIG(statustip)
        actionSave_Setup->setStatusTip(QCoreApplication::translate("MainWindow", "Save the Host panel in a setup file", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionSave_Setup->setShortcut(QCoreApplication::translate("MainWindow", "Alt+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionNew_Setup->setText(QCoreApplication::translate("MainWindow", "New Setup", nullptr));
#if QT_CONFIG(statustip)
        actionNew_Setup->setStatusTip(QCoreApplication::translate("MainWindow", "Clear the Host panel", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionNew_Setup->setShortcut(QCoreApplication::translate("MainWindow", "Alt+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionHost_panel->setText(QCoreApplication::translate("MainWindow", "Toggle Host panel", nullptr));
#if QT_CONFIG(statustip)
        actionHost_panel->setStatusTip(QCoreApplication::translate("MainWindow", "Show or hide the host panel", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionHost_panel->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+H", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAbout->setText(QCoreApplication::translate("MainWindow", "About...", nullptr));
#if QT_CONFIG(statustip)
        actionAbout->setStatusTip(QCoreApplication::translate("MainWindow", "About", nullptr));
#endif // QT_CONFIG(statustip)
        actionSave_Project_As->setText(QCoreApplication::translate("MainWindow", "Save Project As ...", nullptr));
#if QT_CONFIG(statustip)
        actionSave_Project_As->setStatusTip(QCoreApplication::translate("MainWindow", "Save Project As", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionSave_Project_As->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave_Setup_As->setText(QCoreApplication::translate("MainWindow", "Save Setup As ...", nullptr));
#if QT_CONFIG(statustip)
        actionSave_Setup_As->setStatusTip(QCoreApplication::translate("MainWindow", "Save Setup As", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionSave_Setup_As->setShortcut(QCoreApplication::translate("MainWindow", "Alt+Shift+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionGroup_panel->setText(QCoreApplication::translate("MainWindow", "Toggle Group panel", nullptr));
#if QT_CONFIG(tooltip)
        actionGroup_panel->setToolTip(QCoreApplication::translate("MainWindow", "Toggle Group panel", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        actionGroup_panel->setStatusTip(QCoreApplication::translate("MainWindow", "Show or hide the group panel", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionGroup_panel->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+G", nullptr));
#endif // QT_CONFIG(shortcut)
        actionProgram_panel->setText(QCoreApplication::translate("MainWindow", "Toggle Program panel", nullptr));
#if QT_CONFIG(statustip)
        actionProgram_panel->setStatusTip(QCoreApplication::translate("MainWindow", "Show or hide the program panel", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionProgram_panel->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+P", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRefresh_Audio_devices->setText(QCoreApplication::translate("MainWindow", "Refresh Audio devices", nullptr));
        actionRefresh_Midi_devices->setText(QCoreApplication::translate("MainWindow", "Refresh Midi devices", nullptr));
        actionRestore_default_layout->setText(QCoreApplication::translate("MainWindow", "Restore default layout", nullptr));
        actionTool_bar->setText(QCoreApplication::translate("MainWindow", "Tool bar", nullptr));
        actionProject_panel->setText(QCoreApplication::translate("MainWindow", "Toggle Project panel", nullptr));
        actionLoad_Setup->setText(QCoreApplication::translate("MainWindow", "Open Setup", nullptr));
#if QT_CONFIG(tooltip)
        actionLoad_Setup->setToolTip(QCoreApplication::translate("MainWindow", "Open a Setup file", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        actionLoad_Setup->setStatusTip(QCoreApplication::translate("MainWindow", "Open a Setup file", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
        actionLoad_Setup->setShortcut(QCoreApplication::translate("MainWindow", "Alt+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAppearance->setText(QCoreApplication::translate("MainWindow", "Appearance", nullptr));
        actionCable->setText(QCoreApplication::translate("MainWindow", "Cable", nullptr));
#if QT_CONFIG(shortcut)
        actionCable->setShortcut(QCoreApplication::translate("MainWindow", "C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionValue->setText(QCoreApplication::translate("MainWindow", "Value", nullptr));
#if QT_CONFIG(shortcut)
        actionValue->setShortcut(QCoreApplication::translate("MainWindow", "V", nullptr));
#endif // QT_CONFIG(shortcut)
        actionKeyBinding->setText(QCoreApplication::translate("MainWindow", "Keyboard mapping", nullptr));
#if QT_CONFIG(tooltip)
        actionKeyBinding->setToolTip(QCoreApplication::translate("MainWindow", "Keyboard and Mouse mapping", nullptr));
#endif // QT_CONFIG(tooltip)
        actionHide_all_editors->setText(QCoreApplication::translate("MainWindow", "Hide all plugins editors", nullptr));
#if QT_CONFIG(shortcut)
        actionHide_all_editors->setShortcut(QCoreApplication::translate("MainWindow", "H", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAutoShowGui->setText(QCoreApplication::translate("MainWindow", "Auto-open plugins editors", nullptr));
#if QT_CONFIG(shortcut)
        actionAutoShowGui->setShortcut(QCoreApplication::translate("MainWindow", "G", nullptr));
#endif // QT_CONFIG(shortcut)
        actionUndo->setText(QCoreApplication::translate("MainWindow", "Undo", nullptr));
#if QT_CONFIG(shortcut)
        actionUndo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRedo->setText(QCoreApplication::translate("MainWindow", "Redo", nullptr));
#if QT_CONFIG(shortcut)
        actionRedo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        HostBox->setTitle(QCoreApplication::translate("MainWindow", "Host Panel", nullptr));
        ProjectBox->setTitle(QCoreApplication::translate("MainWindow", "Project Panel", nullptr));
        ProgramBox->setTitle(QCoreApplication::translate("MainWindow", "Program Panel", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Group Panel", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "View", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
        menuTool->setTitle(QCoreApplication::translate("MainWindow", "Tool", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuRecent_Setups->setTitle(QCoreApplication::translate("MainWindow", "Recent Setups", nullptr));
        menuRecent_Projects->setTitle(QCoreApplication::translate("MainWindow", "Recent Projects", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
        mainToolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "Toolbar", nullptr));
        dockPrograms->setWindowTitle(QCoreApplication::translate("MainWindow", "Programs", nullptr));
        dockMidiDevices->setWindowTitle(QCoreApplication::translate("MainWindow", "Midi devices", nullptr));
        dockVstBrowser->setWindowTitle(QCoreApplication::translate("MainWindow", "VST Plugins", nullptr));
        dockAudioDevices->setWindowTitle(QCoreApplication::translate("MainWindow", "Audio devices", nullptr));
        dockSolver->setWindowTitle(QCoreApplication::translate("MainWindow", "Solver", nullptr));
        dockTools->setWindowTitle(QCoreApplication::translate("MainWindow", "Tools", nullptr));
        dockHostModel->setWindowTitle(QCoreApplication::translate("MainWindow", "HostModel", nullptr));
        dockBankBrowser->setWindowTitle(QCoreApplication::translate("MainWindow", "Browser", nullptr));
        dockUndo->setWindowTitle(QCoreApplication::translate("MainWindow", "Undo history", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
