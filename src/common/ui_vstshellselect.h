/********************************************************************************
** Form generated from reading UI file 'vstshellselect.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VSTSHELLSELECT_H
#define UI_VSTSHELLSELECT_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QListWidget>

QT_BEGIN_NAMESPACE

class Ui_VstShellSelect
{
public:
    QGridLayout *gridLayout;
    QListWidget *listPlugins;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *VstShellSelect)
    {
        if (VstShellSelect->objectName().isEmpty())
            VstShellSelect->setObjectName(QString::fromUtf8("VstShellSelect"));
        VstShellSelect->setWindowModality(Qt::ApplicationModal);
        VstShellSelect->resize(234, 297);
        VstShellSelect->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        gridLayout = new QGridLayout(VstShellSelect);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        listPlugins = new QListWidget(VstShellSelect);
        listPlugins->setObjectName(QString::fromUtf8("listPlugins"));
        listPlugins->setUniformItemSizes(true);

        gridLayout->addWidget(listPlugins, 0, 0, 1, 2);

        buttonBox = new QDialogButtonBox(VstShellSelect);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 1, 1, 1, 1);


        retranslateUi(VstShellSelect);
        QObject::connect(buttonBox, SIGNAL(accepted()), VstShellSelect, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), VstShellSelect, SLOT(reject()));
        QObject::connect(listPlugins, SIGNAL(itemDoubleClicked(QListWidgetItem*)), VstShellSelect, SLOT(accept()));

        QMetaObject::connectSlotsByName(VstShellSelect);
    } // setupUi

    void retranslateUi(QDialog *VstShellSelect)
    {
        VstShellSelect->setWindowTitle(QCoreApplication::translate("VstShellSelect", "VstShell", nullptr));
    } // retranslateUi

};

namespace Ui {
    class VstShellSelect: public Ui_VstShellSelect {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VSTSHELLSELECT_H
