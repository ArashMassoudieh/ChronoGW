#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "IconListWidget.h"
#include "GWA.h"

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

private:
    Ui::MainWindow *ui;
    QString currentFilePath;
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
};

#endif // MAINWINDOW_H
