/********************************************************************************
** Form generated from reading UI file 'configdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIGDIALOG_H
#define UI_CONFIGDIALOG_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_ConfigDialog
{
public:
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_3;
    QComboBox *defaultSetup;
    QPushButton *browseSetup;
    QLabel *label_2;
    QComboBox *defaultProject;
    QPushButton *browseProject;
    QLabel *label;
    QComboBox *defaultVstPath;
    QPushButton *browseVst;
    QLabel *label_4;
    QComboBox *defaultBankPath;
    QPushButton *browseBank;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBox_2;
    QFormLayout *formLayout_3;
    QLabel *label_5;
    QSpinBox *nbThreads;
    QCheckBox *fastEditorOpenClose;
    QGroupBox *groupBox_3;
    QFormLayout *formLayout;
    QLabel *label_7;
    QComboBox *onUnsavedSetup;
    QLabel *label_8;
    QComboBox *onUnsavedProject;
    QGroupBox *groupSampleFormat;
    QFormLayout *formLayout_2;
    QLabel *labelsampleRate;
    QComboBox *sampleRate;
    QComboBox *bufferSize;
    QLabel *labelsamplePrecision;
    QComboBox *samplePrecision;
    QLabel *labelbufferSize;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ConfigDialog)
    {
        if (ConfigDialog->objectName().isEmpty())
            ConfigDialog->setObjectName(QString::fromUtf8("ConfigDialog"));
        ConfigDialog->resize(525, 370);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ConfigDialog->sizePolicy().hasHeightForWidth());
        ConfigDialog->setSizePolicy(sizePolicy);
        ConfigDialog->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        gridLayout_2 = new QGridLayout(ConfigDialog);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        groupBox = new QGroupBox(ConfigDialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_3, 0, 0, 1, 1);

        defaultSetup = new QComboBox(groupBox);
        defaultSetup->setObjectName(QString::fromUtf8("defaultSetup"));
        QSizePolicy sizePolicy2(QSizePolicy::Ignored, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(defaultSetup->sizePolicy().hasHeightForWidth());
        defaultSetup->setSizePolicy(sizePolicy2);
        defaultSetup->setEditable(true);

        gridLayout->addWidget(defaultSetup, 0, 1, 1, 1);

        browseSetup = new QPushButton(groupBox);
        browseSetup->setObjectName(QString::fromUtf8("browseSetup"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(browseSetup->sizePolicy().hasHeightForWidth());
        browseSetup->setSizePolicy(sizePolicy3);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/img16x16/folder.png"), QSize(), QIcon::Normal, QIcon::Off);
        browseSetup->setIcon(icon);

        gridLayout->addWidget(browseSetup, 0, 2, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        defaultProject = new QComboBox(groupBox);
        defaultProject->setObjectName(QString::fromUtf8("defaultProject"));
        sizePolicy2.setHeightForWidth(defaultProject->sizePolicy().hasHeightForWidth());
        defaultProject->setSizePolicy(sizePolicy2);
        defaultProject->setEditable(true);

        gridLayout->addWidget(defaultProject, 1, 1, 1, 1);

        browseProject = new QPushButton(groupBox);
        browseProject->setObjectName(QString::fromUtf8("browseProject"));
        sizePolicy3.setHeightForWidth(browseProject->sizePolicy().hasHeightForWidth());
        browseProject->setSizePolicy(sizePolicy3);
        browseProject->setIcon(icon);

        gridLayout->addWidget(browseProject, 1, 2, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label, 2, 0, 1, 1);

        defaultVstPath = new QComboBox(groupBox);
        defaultVstPath->setObjectName(QString::fromUtf8("defaultVstPath"));
        sizePolicy2.setHeightForWidth(defaultVstPath->sizePolicy().hasHeightForWidth());
        defaultVstPath->setSizePolicy(sizePolicy2);
        defaultVstPath->setEditable(true);

        gridLayout->addWidget(defaultVstPath, 2, 1, 1, 1);

        browseVst = new QPushButton(groupBox);
        browseVst->setObjectName(QString::fromUtf8("browseVst"));
        sizePolicy3.setHeightForWidth(browseVst->sizePolicy().hasHeightForWidth());
        browseVst->setSizePolicy(sizePolicy3);
        browseVst->setIcon(icon);

        gridLayout->addWidget(browseVst, 2, 2, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        defaultBankPath = new QComboBox(groupBox);
        defaultBankPath->setObjectName(QString::fromUtf8("defaultBankPath"));
        sizePolicy2.setHeightForWidth(defaultBankPath->sizePolicy().hasHeightForWidth());
        defaultBankPath->setSizePolicy(sizePolicy2);
        defaultBankPath->setEditable(true);

        gridLayout->addWidget(defaultBankPath, 3, 1, 1, 1);

        browseBank = new QPushButton(groupBox);
        browseBank->setObjectName(QString::fromUtf8("browseBank"));
        sizePolicy3.setHeightForWidth(browseBank->sizePolicy().hasHeightForWidth());
        browseBank->setSizePolicy(sizePolicy3);
        browseBank->setIcon(icon);

        gridLayout->addWidget(browseBank, 3, 2, 1, 1);


        gridLayout_2->addWidget(groupBox, 1, 3, 1, 6);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 2, 5, 3, 1);

        groupBox_2 = new QGroupBox(ConfigDialog);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        formLayout_3 = new QFormLayout(groupBox_2);
        formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout_3->setWidget(0, QFormLayout::LabelRole, label_5);

        nbThreads = new QSpinBox(groupBox_2);
        nbThreads->setObjectName(QString::fromUtf8("nbThreads"));
        sizePolicy3.setHeightForWidth(nbThreads->sizePolicy().hasHeightForWidth());
        nbThreads->setSizePolicy(sizePolicy3);
        nbThreads->setMaximum(500);

        formLayout_3->setWidget(0, QFormLayout::FieldRole, nbThreads);

        fastEditorOpenClose = new QCheckBox(groupBox_2);
        fastEditorOpenClose->setObjectName(QString::fromUtf8("fastEditorOpenClose"));

        formLayout_3->setWidget(1, QFormLayout::SpanningRole, fastEditorOpenClose);


        gridLayout_2->addWidget(groupBox_2, 3, 3, 2, 2);

        groupBox_3 = new QGroupBox(ConfigDialog);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        QSizePolicy sizePolicy4(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy4);
        formLayout = new QFormLayout(groupBox_3);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label_7 = new QLabel(groupBox_3);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_7);

        onUnsavedSetup = new QComboBox(groupBox_3);
        onUnsavedSetup->setObjectName(QString::fromUtf8("onUnsavedSetup"));
        onUnsavedSetup->setFrame(true);

        formLayout->setWidget(0, QFormLayout::FieldRole, onUnsavedSetup);

        label_8 = new QLabel(groupBox_3);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_8);

        onUnsavedProject = new QComboBox(groupBox_3);
        onUnsavedProject->setObjectName(QString::fromUtf8("onUnsavedProject"));

        formLayout->setWidget(1, QFormLayout::FieldRole, onUnsavedProject);


        gridLayout_2->addWidget(groupBox_3, 2, 4, 1, 1);

        groupSampleFormat = new QGroupBox(ConfigDialog);
        groupSampleFormat->setObjectName(QString::fromUtf8("groupSampleFormat"));
        sizePolicy4.setHeightForWidth(groupSampleFormat->sizePolicy().hasHeightForWidth());
        groupSampleFormat->setSizePolicy(sizePolicy4);
        formLayout_2 = new QFormLayout(groupSampleFormat);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        labelsampleRate = new QLabel(groupSampleFormat);
        labelsampleRate->setObjectName(QString::fromUtf8("labelsampleRate"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, labelsampleRate);

        sampleRate = new QComboBox(groupSampleFormat);
        sampleRate->setObjectName(QString::fromUtf8("sampleRate"));

        formLayout_2->setWidget(0, QFormLayout::FieldRole, sampleRate);

        bufferSize = new QComboBox(groupSampleFormat);
        bufferSize->setObjectName(QString::fromUtf8("bufferSize"));
        bufferSize->setEnabled(false);

        formLayout_2->setWidget(2, QFormLayout::FieldRole, bufferSize);

        labelsamplePrecision = new QLabel(groupSampleFormat);
        labelsamplePrecision->setObjectName(QString::fromUtf8("labelsamplePrecision"));

        formLayout_2->setWidget(3, QFormLayout::LabelRole, labelsamplePrecision);

        samplePrecision = new QComboBox(groupSampleFormat);
        samplePrecision->setObjectName(QString::fromUtf8("samplePrecision"));

        formLayout_2->setWidget(3, QFormLayout::FieldRole, samplePrecision);

        labelbufferSize = new QLabel(groupSampleFormat);
        labelbufferSize->setObjectName(QString::fromUtf8("labelbufferSize"));
        labelbufferSize->setEnabled(false);

        formLayout_2->setWidget(2, QFormLayout::LabelRole, labelbufferSize);


        gridLayout_2->addWidget(groupSampleFormat, 2, 3, 1, 1);

        buttonBox = new QDialogButtonBox(ConfigDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        sizePolicy3.setHeightForWidth(buttonBox->sizePolicy().hasHeightForWidth());
        buttonBox->setSizePolicy(sizePolicy3);
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(buttonBox, 4, 7, 1, 1);


        retranslateUi(ConfigDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ConfigDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ConfigDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ConfigDialog);
    } // setupUi

    void retranslateUi(QDialog *ConfigDialog)
    {
        ConfigDialog->setWindowTitle(QCoreApplication::translate("ConfigDialog", "Config", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ConfigDialog", "Default files and directories", nullptr));
        label_3->setText(QCoreApplication::translate("ConfigDialog", "Setup file", nullptr));
#if QT_CONFIG(tooltip)
        browseSetup->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        browseSetup->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        label_2->setText(QCoreApplication::translate("ConfigDialog", "Project file", nullptr));
#if QT_CONFIG(tooltip)
        browseProject->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        browseProject->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        label->setText(QCoreApplication::translate("ConfigDialog", "VST directory", nullptr));
#if QT_CONFIG(tooltip)
        browseVst->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        browseVst->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        label_4->setText(QCoreApplication::translate("ConfigDialog", "Bank directory", nullptr));
#if QT_CONFIG(tooltip)
        browseBank->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        browseBank->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        groupBox_2->setTitle(QCoreApplication::translate("ConfigDialog", "Engine", nullptr));
        label_5->setText(QCoreApplication::translate("ConfigDialog", "Number of threads (0=auto) :", nullptr));
#if QT_CONFIG(tooltip)
        fastEditorOpenClose->setToolTip(QCoreApplication::translate("ConfigDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Allows faster program change by hiding windows instead of closing them. May increase cpu and memory usage</span></p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        fastEditorOpenClose->setText(QCoreApplication::translate("ConfigDialog", "Fast plugins windows open/close", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("ConfigDialog", "On unsaved changes", nullptr));
        label_7->setText(QCoreApplication::translate("ConfigDialog", "Setup", nullptr));
        label_8->setText(QCoreApplication::translate("ConfigDialog", "Project", nullptr));
        groupSampleFormat->setTitle(QCoreApplication::translate("ConfigDialog", "Sample format", nullptr));
        labelsampleRate->setText(QCoreApplication::translate("ConfigDialog", "Sample rate", nullptr));
        labelsamplePrecision->setText(QCoreApplication::translate("ConfigDialog", "Precision", nullptr));
        labelbufferSize->setText(QCoreApplication::translate("ConfigDialog", "Buffer size", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ConfigDialog: public Ui_ConfigDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIGDIALOG_H
