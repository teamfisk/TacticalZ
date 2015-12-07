/********************************************************************************
** Form generated from reading UI file 'MayaExporter.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAYAEXPORTER_H
#define UI_MAYAEXPORTER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_leeeeelClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *leeeeelClass)
    {
        if (leeeeelClass->objectName().isEmpty())
            leeeeelClass->setObjectName(QString::fromUtf8("leeeeelClass"));
        leeeeelClass->resize(600, 400);
        menuBar = new QMenuBar(leeeeelClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        leeeeelClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(leeeeelClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        leeeeelClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(leeeeelClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        leeeeelClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(leeeeelClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        leeeeelClass->setStatusBar(statusBar);

        retranslateUi(leeeeelClass);

        QMetaObject::connectSlotsByName(leeeeelClass);
    } // setupUi

    void retranslateUi(QMainWindow *leeeeelClass)
    {
        leeeeelClass->setWindowTitle(QApplication::translate("leeeeelClass", "leeeeel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class leeeeelClass: public Ui_leeeeelClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAYAEXPORTER_H
