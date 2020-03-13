/********************************************************************************
** Form generated from reading UI file 'keybindingdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KEYBINDINGDIALOG_H
#define UI_KEYBINDINGDIALOG_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListView>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_KeyBindingDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QTabWidget *tabWidget;
    QWidget *keys_tab;
    QHBoxLayout *horizontalLayout_2;
    QTableView *tableMain;
    QWidget *modes_tab;
    QHBoxLayout *horizontalLayout;
    QListView *listModes;
    QTableView *tableMode;
    QHBoxLayout *horizontalLayout_3;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *KeyBindingDialog)
    {
        if (KeyBindingDialog->objectName().isEmpty())
            KeyBindingDialog->setObjectName(QString::fromUtf8("KeyBindingDialog"));
        KeyBindingDialog->resize(495, 327);
        KeyBindingDialog->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        verticalLayout_2 = new QVBoxLayout(KeyBindingDialog);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        tabWidget = new QTabWidget(KeyBindingDialog);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        keys_tab = new QWidget();
        keys_tab->setObjectName(QString::fromUtf8("keys_tab"));
        horizontalLayout_2 = new QHBoxLayout(keys_tab);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        tableMain = new QTableView(keys_tab);
        tableMain->setObjectName(QString::fromUtf8("tableMain"));
        tableMain->setContextMenuPolicy(Qt::NoContextMenu);
        tableMain->setFrameShape(QFrame::NoFrame);
        tableMain->setEditTriggers(QAbstractItemView::AllEditTriggers);
        tableMain->setSelectionMode(QAbstractItemView::NoSelection);
        tableMain->horizontalHeader()->setVisible(false);
        tableMain->verticalHeader()->setVisible(false);

        horizontalLayout_2->addWidget(tableMain);

        QIcon icon;
        icon.addFile(QString::fromUtf8(":/img16x16/key_bindings.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(keys_tab, icon, QString());
        modes_tab = new QWidget();
        modes_tab->setObjectName(QString::fromUtf8("modes_tab"));
        horizontalLayout = new QHBoxLayout(modes_tab);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        listModes = new QListView(modes_tab);
        listModes->setObjectName(QString::fromUtf8("listModes"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(listModes->sizePolicy().hasHeightForWidth());
        listModes->setSizePolicy(sizePolicy);
        listModes->setMaximumSize(QSize(70, 16777215));
        listModes->setContextMenuPolicy(Qt::NoContextMenu);
        listModes->setSelectionBehavior(QAbstractItemView::SelectRows);

        horizontalLayout->addWidget(listModes);

        tableMode = new QTableView(modes_tab);
        tableMode->setObjectName(QString::fromUtf8("tableMode"));
        tableMode->setContextMenuPolicy(Qt::NoContextMenu);
        tableMode->setFrameShape(QFrame::NoFrame);
        tableMode->setEditTriggers(QAbstractItemView::AllEditTriggers);
        tableMode->setSelectionMode(QAbstractItemView::NoSelection);
        tableMode->verticalHeader()->setVisible(false);

        horizontalLayout->addWidget(tableMode);

        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/img16x16/move.png"), QSize(), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(modes_tab, icon1, QString());

        verticalLayout_2->addWidget(tabWidget);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        buttonBox = new QDialogButtonBox(KeyBindingDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok|QDialogButtonBox::RestoreDefaults);

        horizontalLayout_3->addWidget(buttonBox);


        verticalLayout_2->addLayout(horizontalLayout_3);


        retranslateUi(KeyBindingDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), KeyBindingDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), KeyBindingDialog, SLOT(reject()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(KeyBindingDialog);
    } // setupUi

    void retranslateUi(QDialog *KeyBindingDialog)
    {
        KeyBindingDialog->setWindowTitle(QCoreApplication::translate("KeyBindingDialog", "Keyboard mapping", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(keys_tab), QCoreApplication::translate("KeyBindingDialog", "Keys", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(modes_tab), QCoreApplication::translate("KeyBindingDialog", "Modes", nullptr));
    } // retranslateUi

};

namespace Ui {
    class KeyBindingDialog: public Ui_KeyBindingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KEYBINDINGDIALOG_H
