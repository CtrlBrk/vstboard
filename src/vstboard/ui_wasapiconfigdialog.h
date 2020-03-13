/********************************************************************************
** Form generated from reading UI file 'wasapiconfigdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WASAPICONFIGDIALOG_H
#define UI_WASAPICONFIGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_WasapiConfigDialog
{
public:
    QFormLayout *formLayout;
    QLabel *label;
    QSpinBox *inputLatency;
    QLabel *label_2;
    QSpinBox *outputLatency;
    QDialogButtonBox *buttonBox;
    QCheckBox *exclusiveMode;

    void setupUi(QDialog *WasapiConfigDialog)
    {
        if (WasapiConfigDialog->objectName().isEmpty())
            WasapiConfigDialog->setObjectName(QString::fromUtf8("WasapiConfigDialog"));
        WasapiConfigDialog->resize(225, 116);
        formLayout = new QFormLayout(WasapiConfigDialog);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label = new QLabel(WasapiConfigDialog);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label);

        inputLatency = new QSpinBox(WasapiConfigDialog);
        inputLatency->setObjectName(QString::fromUtf8("inputLatency"));
        inputLatency->setMaximum(5000);

        formLayout->setWidget(1, QFormLayout::FieldRole, inputLatency);

        label_2 = new QLabel(WasapiConfigDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_2);

        outputLatency = new QSpinBox(WasapiConfigDialog);
        outputLatency->setObjectName(QString::fromUtf8("outputLatency"));
        outputLatency->setMaximum(5000);

        formLayout->setWidget(2, QFormLayout::FieldRole, outputLatency);

        buttonBox = new QDialogButtonBox(WasapiConfigDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        formLayout->setWidget(3, QFormLayout::SpanningRole, buttonBox);

        exclusiveMode = new QCheckBox(WasapiConfigDialog);
        exclusiveMode->setObjectName(QString::fromUtf8("exclusiveMode"));

        formLayout->setWidget(0, QFormLayout::SpanningRole, exclusiveMode);


        retranslateUi(WasapiConfigDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), WasapiConfigDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), WasapiConfigDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(WasapiConfigDialog);
    } // setupUi

    void retranslateUi(QDialog *WasapiConfigDialog)
    {
        WasapiConfigDialog->setWindowTitle(QCoreApplication::translate("WasapiConfigDialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("WasapiConfigDialog", "Input latency (0=auto)", nullptr));
        label_2->setText(QCoreApplication::translate("WasapiConfigDialog", "Output latency (0=auto)", nullptr));
        exclusiveMode->setText(QCoreApplication::translate("WasapiConfigDialog", "Exclusive mode", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WasapiConfigDialog: public Ui_WasapiConfigDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WASAPICONFIGDIALOG_H
