/********************************************************************************
** Form generated from reading UI file 'splash.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SPLASH_H
#define UI_SPLASH_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Splash
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout_2;
    QLabel *labelApp;
    QLabel *labelVersion;
    QTextBrowser *textBrowser;
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkHideIt;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Splash)
    {
        if (Splash->objectName().isEmpty())
            Splash->setObjectName(QString::fromUtf8("Splash"));
        Splash->setEnabled(true);
        Splash->resize(346, 257);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/vstboard32_8.png"), QSize(), QIcon::Normal, QIcon::Off);
        Splash->setWindowIcon(icon);
        Splash->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        verticalLayout = new QVBoxLayout(Splash);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(Splash);
        label->setObjectName(QString::fromUtf8("label"));
        label->setPixmap(QPixmap(QString::fromUtf8(":/vstboard logo2.png")));
        label->setTextInteractionFlags(Qt::NoTextInteraction);

        verticalLayout->addWidget(label);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(5);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        labelApp = new QLabel(Splash);
        labelApp->setObjectName(QString::fromUtf8("labelApp"));
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        font.setWeight(75);
        labelApp->setFont(font);
        labelApp->setText(QString::fromUtf8("TextLabel"));
        labelApp->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        horizontalLayout_2->addWidget(labelApp);

        labelVersion = new QLabel(Splash);
        labelVersion->setObjectName(QString::fromUtf8("labelVersion"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(labelVersion->sizePolicy().hasHeightForWidth());
        labelVersion->setSizePolicy(sizePolicy);
        QFont font1;
        font1.setPointSize(10);
        labelVersion->setFont(font1);
        labelVersion->setText(QString::fromUtf8("TextLabel"));
        labelVersion->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        horizontalLayout_2->addWidget(labelVersion);


        verticalLayout->addLayout(horizontalLayout_2);

        textBrowser = new QTextBrowser(Splash);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        textBrowser->setEnabled(false);
        QPalette palette;
        QBrush brush(QColor(0, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Active, QPalette::Dark, brush);
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Dark, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Dark, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush);
        textBrowser->setPalette(palette);
        textBrowser->setAcceptDrops(false);
        textBrowser->setHtml(QString::fromUtf8("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"justify\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\"> Copyright (C) 2010-2012 Rapha\303\203\302\253l Fran\303\203\302\247ois</span></p>\n"
"<p align=\"justify\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"></p>\n"
"<p align=\"justify\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">This program comes with ABSOLUTELY NO WARRANTY; This is free sof"
                        "tware, and you are welcome to redistribute it under certain conditions; for details see &quot;About...&quot; in the Help menu.</span></p></body></html>"));
        textBrowser->setTextInteractionFlags(Qt::NoTextInteraction);

        verticalLayout->addWidget(textBrowser);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        checkHideIt = new QCheckBox(Splash);
        checkHideIt->setObjectName(QString::fromUtf8("checkHideIt"));
        checkHideIt->setChecked(true);

        horizontalLayout->addWidget(checkHideIt);

        buttonBox = new QDialogButtonBox(Splash);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok);

        horizontalLayout->addWidget(buttonBox);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(Splash);
        QObject::connect(buttonBox, SIGNAL(accepted()), Splash, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Splash, SLOT(reject()));

        QMetaObject::connectSlotsByName(Splash);
    } // setupUi

    void retranslateUi(QDialog *Splash)
    {
        checkHideIt->setText(QCoreApplication::translate("Splash", "Don't show this again", nullptr));
        (void)Splash;
    } // retranslateUi

};

namespace Ui {
    class Splash: public Ui_Splash {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SPLASH_H
