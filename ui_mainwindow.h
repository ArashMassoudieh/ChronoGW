/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionWells;
    QAction *actionTracers;
    QAction *actionParameters;
    QAction *actionObservations;
    QAction *actionDeterministic_GA;
    QAction *actionBayesian_MCMC;
    QAction *actionDeterministinc_LM;
    QAction *actionAbout;
    QAction *actionSave_As;
    QAction *actionRecent_Projects;
    QAction *actionGenetic_Algorithm_Settings;
    QAction *actionMCMC_Settings;
    QAction *actionLevenberg_Marquardt_Settings;
    QWidget *centralwidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuParameter_Estimation;
    QMenu *menuAbout;
    QMenu *menuSettings;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName("actionOpen");
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName("actionSave");
        actionWells = new QAction(MainWindow);
        actionWells->setObjectName("actionWells");
        actionTracers = new QAction(MainWindow);
        actionTracers->setObjectName("actionTracers");
        actionParameters = new QAction(MainWindow);
        actionParameters->setObjectName("actionParameters");
        actionObservations = new QAction(MainWindow);
        actionObservations->setObjectName("actionObservations");
        actionDeterministic_GA = new QAction(MainWindow);
        actionDeterministic_GA->setObjectName("actionDeterministic_GA");
        actionBayesian_MCMC = new QAction(MainWindow);
        actionBayesian_MCMC->setObjectName("actionBayesian_MCMC");
        actionDeterministinc_LM = new QAction(MainWindow);
        actionDeterministinc_LM->setObjectName("actionDeterministinc_LM");
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName("actionAbout");
        actionSave_As = new QAction(MainWindow);
        actionSave_As->setObjectName("actionSave_As");
        actionRecent_Projects = new QAction(MainWindow);
        actionRecent_Projects->setObjectName("actionRecent_Projects");
        actionGenetic_Algorithm_Settings = new QAction(MainWindow);
        actionGenetic_Algorithm_Settings->setObjectName("actionGenetic_Algorithm_Settings");
        actionMCMC_Settings = new QAction(MainWindow);
        actionMCMC_Settings->setObjectName("actionMCMC_Settings");
        actionLevenberg_Marquardt_Settings = new QAction(MainWindow);
        actionLevenberg_Marquardt_Settings->setObjectName("actionLevenberg_Marquardt_Settings");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 23));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuParameter_Estimation = new QMenu(menubar);
        menuParameter_Estimation->setObjectName("menuParameter_Estimation");
        menuAbout = new QMenu(menubar);
        menuAbout->setObjectName("menuAbout");
        menuSettings = new QMenu(menubar);
        menuSettings->setObjectName("menuSettings");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuSettings->menuAction());
        menubar->addAction(menuParameter_Estimation->menuAction());
        menubar->addAction(menuAbout->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionRecent_Projects);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_As);
        menuParameter_Estimation->addAction(actionDeterministic_GA);
        menuParameter_Estimation->addAction(actionBayesian_MCMC);
        menuParameter_Estimation->addAction(actionDeterministinc_LM);
        menuAbout->addAction(actionAbout);
        menuSettings->addAction(actionGenetic_Algorithm_Settings);
        menuSettings->addAction(actionMCMC_Settings);
        menuSettings->addAction(actionLevenberg_Marquardt_Settings);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "Open", nullptr));
        actionSave->setText(QCoreApplication::translate("MainWindow", "Save", nullptr));
        actionWells->setText(QCoreApplication::translate("MainWindow", "Wells", nullptr));
        actionTracers->setText(QCoreApplication::translate("MainWindow", "Tracers", nullptr));
        actionParameters->setText(QCoreApplication::translate("MainWindow", "Parameters", nullptr));
        actionObservations->setText(QCoreApplication::translate("MainWindow", "Observations", nullptr));
        actionDeterministic_GA->setText(QCoreApplication::translate("MainWindow", "Deterministic (GA)", nullptr));
        actionBayesian_MCMC->setText(QCoreApplication::translate("MainWindow", "Bayesian (MCMC)", nullptr));
        actionDeterministinc_LM->setText(QCoreApplication::translate("MainWindow", "Deterministinc (LM)", nullptr));
        actionAbout->setText(QCoreApplication::translate("MainWindow", "About", nullptr));
        actionSave_As->setText(QCoreApplication::translate("MainWindow", "Save As", nullptr));
        actionRecent_Projects->setText(QCoreApplication::translate("MainWindow", "Recent Projects", nullptr));
        actionGenetic_Algorithm_Settings->setText(QCoreApplication::translate("MainWindow", "Genetic Algorithm Settings", nullptr));
        actionMCMC_Settings->setText(QCoreApplication::translate("MainWindow", "MCMC Settings", nullptr));
        actionLevenberg_Marquardt_Settings->setText(QCoreApplication::translate("MainWindow", "Levenberg-Marquardt Settings", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuParameter_Estimation->setTitle(QCoreApplication::translate("MainWindow", "Parameter Estimation", nullptr));
        menuAbout->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
        menuSettings->setTitle(QCoreApplication::translate("MainWindow", "Settings", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
