/********************************************************************************
** Form generated from reading UI file 'modifierswidget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MODIFIERSWIDGET_H
#define UI_MODIFIERSWIDGET_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ModifiersWidget
{
public:
    QVBoxLayout *verticalLayout;
    QCheckBox *shift;
    QCheckBox *ctrl;
    QCheckBox *alt;

    void setupUi(QWidget *ModifiersWidget)
    {
        if (ModifiersWidget->objectName().isEmpty())
            ModifiersWidget->setObjectName(QString::fromUtf8("ModifiersWidget"));
        ModifiersWidget->resize(45, 54);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ModifiersWidget->sizePolicy().hasHeightForWidth());
        ModifiersWidget->setSizePolicy(sizePolicy);
        ModifiersWidget->setMinimumSize(QSize(0, 54));
        ModifiersWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 127);"));
        ModifiersWidget->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        verticalLayout = new QVBoxLayout(ModifiersWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        shift = new QCheckBox(ModifiersWidget);
        shift->setObjectName(QString::fromUtf8("shift"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(shift->sizePolicy().hasHeightForWidth());
        shift->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(shift);

        ctrl = new QCheckBox(ModifiersWidget);
        ctrl->setObjectName(QString::fromUtf8("ctrl"));
        sizePolicy1.setHeightForWidth(ctrl->sizePolicy().hasHeightForWidth());
        ctrl->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(ctrl);

        alt = new QCheckBox(ModifiersWidget);
        alt->setObjectName(QString::fromUtf8("alt"));
        sizePolicy1.setHeightForWidth(alt->sizePolicy().hasHeightForWidth());
        alt->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(alt);


        retranslateUi(ModifiersWidget);

        QMetaObject::connectSlotsByName(ModifiersWidget);
    } // setupUi

    void retranslateUi(QWidget *ModifiersWidget)
    {
        ModifiersWidget->setWindowTitle(QCoreApplication::translate("ModifiersWidget", "Form", nullptr));
        shift->setText(QCoreApplication::translate("ModifiersWidget", "Shift", nullptr));
        ctrl->setText(QCoreApplication::translate("ModifiersWidget", "Ctrl", nullptr));
        alt->setText(QCoreApplication::translate("ModifiersWidget", "Alt", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ModifiersWidget: public Ui_ModifiersWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MODIFIERSWIDGET_H
