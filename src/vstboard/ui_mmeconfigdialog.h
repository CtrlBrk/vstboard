/********************************************************************************
** Form generated from reading UI file 'mmeconfigdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MMECONFIGDIALOG_H
#define UI_MMECONFIGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_MmeConfigDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QFormLayout *formLayout;
    QSpinBox *framesPerBuffer;
    QLabel *label_2;
    QSpinBox *bufferCount;
    QLabel *label;
    QCheckBox *UseLowLevelLatencyParameters;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QCheckBox *DontThrottleOverloadedProcessingThread;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *MmeConfigDialog)
    {
        if (MmeConfigDialog->objectName().isEmpty())
            MmeConfigDialog->setObjectName(QString::fromUtf8("MmeConfigDialog"));
        MmeConfigDialog->resize(400, 234);
        verticalLayout_2 = new QVBoxLayout(MmeConfigDialog);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(MmeConfigDialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        formLayout = new QFormLayout(groupBox);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        framesPerBuffer = new QSpinBox(groupBox);
        framesPerBuffer->setObjectName(QString::fromUtf8("framesPerBuffer"));
        framesPerBuffer->setMinimum(1);
        framesPerBuffer->setMaximum(4096);

        formLayout->setWidget(2, QFormLayout::FieldRole, framesPerBuffer);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_2);

        bufferCount = new QSpinBox(groupBox);
        bufferCount->setObjectName(QString::fromUtf8("bufferCount"));
        bufferCount->setMinimum(1);

        formLayout->setWidget(3, QFormLayout::FieldRole, bufferCount);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label);

        UseLowLevelLatencyParameters = new QCheckBox(groupBox);
        UseLowLevelLatencyParameters->setObjectName(QString::fromUtf8("UseLowLevelLatencyParameters"));

        formLayout->setWidget(1, QFormLayout::SpanningRole, UseLowLevelLatencyParameters);


        verticalLayout_2->addWidget(groupBox);

        groupBox_2 = new QGroupBox(MmeConfigDialog);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        DontThrottleOverloadedProcessingThread = new QCheckBox(groupBox_2);
        DontThrottleOverloadedProcessingThread->setObjectName(QString::fromUtf8("DontThrottleOverloadedProcessingThread"));

        verticalLayout->addWidget(DontThrottleOverloadedProcessingThread);


        verticalLayout_2->addWidget(groupBox_2);

        buttonBox = new QDialogButtonBox(MmeConfigDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);


        retranslateUi(MmeConfigDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), MmeConfigDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), MmeConfigDialog, SLOT(reject()));
        QObject::connect(UseLowLevelLatencyParameters, SIGNAL(toggled(bool)), framesPerBuffer, SLOT(setEnabled(bool)));
        QObject::connect(UseLowLevelLatencyParameters, SIGNAL(toggled(bool)), bufferCount, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(MmeConfigDialog);
    } // setupUi

    void retranslateUi(QDialog *MmeConfigDialog)
    {
        MmeConfigDialog->setWindowTitle(QCoreApplication::translate("MmeConfigDialog", "Dialog", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MmeConfigDialog", "Buffers", nullptr));
        label_2->setText(QCoreApplication::translate("MmeConfigDialog", "Buffer Count", nullptr));
        label->setText(QCoreApplication::translate("MmeConfigDialog", "Buffer Size", nullptr));
        UseLowLevelLatencyParameters->setText(QCoreApplication::translate("MmeConfigDialog", "Use Low Level Latency Parameters", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("MmeConfigDialog", "Options", nullptr));
        DontThrottleOverloadedProcessingThread->setText(QCoreApplication::translate("MmeConfigDialog", "Don't Throttle Overloaded Processing Thread", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MmeConfigDialog: public Ui_MmeConfigDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MMECONFIGDIALOG_H
