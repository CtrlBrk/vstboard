/********************************************************************************
** Form generated from reading UI file 'aboutdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout;
    QLabel *labelApp;
    QLabel *labelVersion;
    QTextBrowser *textBrowser_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *lgpl3;
    QLabel *vst3;
    QLabel *asio;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
        AboutDialog->resize(608, 304);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AboutDialog->sizePolicy().hasHeightForWidth());
        AboutDialog->setSizePolicy(sizePolicy);
        AboutDialog->setContextMenuPolicy(Qt::NoContextMenu);
        AboutDialog->setWindowTitle(QString::fromUtf8("About the app"));
        AboutDialog->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        verticalLayout = new QVBoxLayout(AboutDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_4 = new QLabel(AboutDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setPixmap(QPixmap(QString::fromUtf8(":/vstboard logo2.png")));
        label_4->setTextInteractionFlags(Qt::NoTextInteraction);

        verticalLayout->addWidget(label_4);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(5);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        labelApp = new QLabel(AboutDialog);
        labelApp->setObjectName(QString::fromUtf8("labelApp"));
        QFont font;
        font.setPointSize(10);
        font.setBold(true);
        font.setWeight(75);
        labelApp->setFont(font);
        labelApp->setText(QString::fromUtf8("appname"));
        labelApp->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        horizontalLayout->addWidget(labelApp);

        labelVersion = new QLabel(AboutDialog);
        labelVersion->setObjectName(QString::fromUtf8("labelVersion"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(labelVersion->sizePolicy().hasHeightForWidth());
        labelVersion->setSizePolicy(sizePolicy1);
        QFont font1;
        font1.setPointSize(10);
        labelVersion->setFont(font1);
        labelVersion->setText(QString::fromUtf8("version"));
        labelVersion->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        horizontalLayout->addWidget(labelVersion);


        verticalLayout->addLayout(horizontalLayout);

        textBrowser_2 = new QTextBrowser(AboutDialog);
        textBrowser_2->setObjectName(QString::fromUtf8("textBrowser_2"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(textBrowser_2->sizePolicy().hasHeightForWidth());
        textBrowser_2->setSizePolicy(sizePolicy2);
        QPalette palette;
        QBrush brush(QColor(240, 240, 240, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
        textBrowser_2->setPalette(palette);
        textBrowser_2->setContextMenuPolicy(Qt::NoContextMenu);
        textBrowser_2->setAcceptDrops(false);
        textBrowser_2->setHtml(QString::fromUtf8("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"http://vstboard.blogspot.com/\"><span style=\" font-size:8pt; text-decoration: underline; color:#0000ff;\">VstBoard</span></a><span style=\" font-size:8pt; color:#000000;\"> Copyright \303\202\302\251 2010-2012 Rapha\303\203\302\253l Fran\303\203\302\247ois. </span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt; color:#000000;\">released under the terms of the </span><a href=\"http://www.gnu.org/licenses/lgpl.html\"><span style=\" fon"
                        "t-size:8pt; text-decoration: underline; color:#0000ff;\">GNU Lesser General Public License version 3.0</span></a><span style=\" font-size:8pt; color:#000000;\"> as published by the Free Software Foundation and appearing in the file LGPL.txt</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"http://www.hermannseib.com/\"><span style=\" font-size:8pt; text-decoration: underline; color:#0000ff;\">VstHost</span></a><span style=\" font-size:8pt; color:#000000;\"> Copyright \303\202\302\251 2006 Hermann Seib</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"http://qt.nokia.com\"><span style=\" font-size:8pt; text-decoration: underline; color:#0000ff;\">Qt</span>"
                        "</a><span style=\" font-size:8pt; color:#000000;\"> Copyright \303\202\302\251 2011 Nokia Corporation and/or its subsidiary(-ies).</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"http://www.portaudio.com\"><span style=\" font-size:8pt; text-decoration: underline; color:#0000ff;\">PortAudio</span></a><span style=\" font-size:8pt; color:#000000;\"> Portable Real-Time Audio Library Copyright \303\202\302\251 1999-2006 Ross Bencina and Phil Burk</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"http://sourceforge.net/projects/portmedia\"><span style=\" font-size:8pt; text-decoration: underline; color:#0000ff;\">PortMidi</span></a><span style=\" font-size:8pt; color:#000000;\"> Copyright \303\202\302\251 1999-2000 Ross Bencina and Phil Burk, Copyright \303\202\302\251 2001-2009 Roger B. Dannenberg.</span></p>\n"
"<p style=\""
                        " margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"http://www.everaldo.com/crystal\"><span style=\" font-size:8pt; text-decoration: underline; color:#0000ff;\">The Crystal Project</span></a><span style=\" font-size:8pt; color:#000000;\"> Copyright \303\202\302\251 2006-2007 Everaldo Coelho</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Geneva,Arial,Helvetica,sans-serif'; font-size:8pt; color:#000000;\">VST Plug-In Technology by </span><a href=\"http://www.steinberg.net\"><span style=\" font-size:8pt; text-decoration: underline; color:#0000ff;\">Steinberg</span></a><span style=\" font-family:'Geneva,Arial,Helvetica,sans-serif'; font-size:8pt; color:#000000;\">.<"
                        "/span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Geneva,Arial,Helvetica,sans-serif'; font-size:8pt; color:#000000;\">ASIO Interface Technology by Steinberg Media Technologies GmbH.</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Geneva,Arial,Helvetica,sans-serif'; font-size:8pt; color:#000000;\">VST and ASIO are trademarks of Steinberg Media Technologies GmbH</span></p></body></html>"));
        textBrowser_2->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);
        textBrowser_2->setOpenExternalLinks(true);

        verticalLayout->addWidget(textBrowser_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        lgpl3 = new QLabel(AboutDialog);
        lgpl3->setObjectName(QString::fromUtf8("lgpl3"));
        lgpl3->setPixmap(QPixmap(QString::fromUtf8(":/lgplv3-88x31.png")));

        horizontalLayout_2->addWidget(lgpl3);

        vst3 = new QLabel(AboutDialog);
        vst3->setObjectName(QString::fromUtf8("vst3"));
        vst3->setPixmap(QPixmap(QString::fromUtf8(":/vst3.png")));

        horizontalLayout_2->addWidget(vst3);

        asio = new QLabel(AboutDialog);
        asio->setObjectName(QString::fromUtf8("asio"));
        asio->setPixmap(QPixmap(QString::fromUtf8(":/asio.png")));

        horizontalLayout_2->addWidget(asio);

        buttonBox = new QDialogButtonBox(AboutDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close);

        horizontalLayout_2->addWidget(buttonBox);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(AboutDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AboutDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AboutDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        lgpl3->setText(QString());
        vst3->setText(QString());
        asio->setText(QString());
        (void)AboutDialog;
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG_H
