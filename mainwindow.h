#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "IconListWidget.h"
#include "GWA.h"
#include "GA.h"
#include "MCMC.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    void onEditWell(const QString& wellName, int index);
    void onEditTracer(const QString& tracerName, int index);
    void onEditParameter(const QString& paramName, int index);
    void onEditObservation(const QString& obsName, int index);
    void onWellRenamed(const QString& oldName, const QString& newName, const QVariant& data);
    void onTracerRenamed(const QString& oldName, const QString& newName, const QVariant& data);
    void onAddWell();
    void onAddTracer();
    void onAddParameter();
    void onAddObservation();
    void onModelModified();
    void onPlotObservation(const QString& obsName, int index);
    void onObservationContextAction(const QString& obsName, int index, const QString& actionType);
    void onRecentFileTriggered();
    void onGASettingsTriggered();

private:
    Ui::MainWindow *ui;
    CGWA gwaModel;

    IconListWidget* wellsWidget;
    IconListWidget* tracersWidget;
    IconListWidget* parametersWidget;
    IconListWidget* observationsWidget;

    void setupCentralWidget();
    void updateAllWidgets();
    bool isModified_;  // Track if document has unsaved changes
    QString currentFilePath_;  // Track current file path

    void setModified(bool modified);
    void updateWindowTitle();

    static const int MaxRecentFiles = 10;
    QMenu* recentFilesMenu;
    QList<QAction*> recentFileActions;

    void updateRecentFilesMenu();
    void addToRecentFiles(const QString& filePath);
    QString getRecentFilesPath() const;
    QStringList loadRecentFiles() const;
    void saveRecentFiles(const QStringList& files) const;
    CGA<CGWA> ga;
    CMCMC<CGWA> mcmc;
};

#endif // MAINWINDOW_H
