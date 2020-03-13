/********************************************************************************
** Form generated from reading UI file 'buttonswidget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BUTTONSWIDGET_H
#define UI_BUTTONSWIDGET_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ButtonsWidget
{
public:
    QVBoxLayout *verticalLayout;
    QCheckBox *lmb;
    QCheckBox *mmb;
    QCheckBox *rmb;
    QCheckBox *mb4;
    QCheckBox *mb5;

    void setupUi(QWidget *ButtonsWidget)
    {
        if (ButtonsWidget->objectName().isEmpty())
            ButtonsWidget->setObjectName(QString::fromUtf8("ButtonsWidget"));
        ButtonsWidget->resize(48, 90);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ButtonsWidget->sizePolicy().hasHeightForWidth());
        ButtonsWidget->setSizePolicy(sizePolicy);
        ButtonsWidget->setMinimumSize(QSize(0, 90));
        ButtonsWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 127);"));
        ButtonsWidget->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        verticalLayout = new QVBoxLayout(ButtonsWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        lmb = new QCheckBox(ButtonsWidget);
        lmb->setObjectName(QString::fromUtf8("lmb"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lmb->sizePolicy().hasHeightForWidth());
        lmb->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(lmb);

        mmb = new QCheckBox(ButtonsWidget);
        mmb->setObjectName(QString::fromUtf8("mmb"));
        sizePolicy1.setHeightForWidth(mmb->sizePolicy().hasHeightForWidth());
        mmb->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(mmb);

        rmb = new QCheckBox(ButtonsWidget);
        rmb->setObjectName(QString::fromUtf8("rmb"));
        sizePolicy1.setHeightForWidth(rmb->sizePolicy().hasHeightForWidth());
        rmb->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(rmb);

        mb4 = new QCheckBox(ButtonsWidget);
        mb4->setObjectName(QString::fromUtf8("mb4"));
        sizePolicy1.setHeightForWidth(mb4->sizePolicy().hasHeightForWidth());
        mb4->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(mb4);

        mb5 = new QCheckBox(ButtonsWidget);
        mb5->setObjectName(QString::fromUtf8("mb5"));
        sizePolicy1.setHeightForWidth(mb5->sizePolicy().hasHeightForWidth());
        mb5->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(mb5);


        retranslateUi(ButtonsWidget);

        QMetaObject::connectSlotsByName(ButtonsWidget);
    } // setupUi

    void retranslateUi(QWidget *ButtonsWidget)
    {
        lmb->setText(QCoreApplication::translate("ButtonsWidget", "Left", nullptr));
        mmb->setText(QCoreApplication::translate("ButtonsWidget", "Mid", nullptr));
        rmb->setText(QCoreApplication::translate("ButtonsWidget", "Right", nullptr));
        mb4->setText(QCoreApplication::translate("ButtonsWidget", "Bt 4", nullptr));
        mb5->setText(QCoreApplication::translate("ButtonsWidget", "Bt 5", nullptr));
        (void)ButtonsWidget;
    } // retranslateUi

};

namespace Ui {
    class ButtonsWidget: public Ui_ButtonsWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BUTTONSWIDGET_H
