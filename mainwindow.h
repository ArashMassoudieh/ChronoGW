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
    void onEditWell(const QString& name, const QVariant& data);

private:
    Ui::MainWindow *ui;
    QString currentFilePath;
    CGWA gwaModel;

    IconListWidget* wellsWidget;
    IconListWidget* tracersWidget;
    IconListWidget* parametersWidget;
    IconListWidget* observationsWidget;

    void setupCentralWidget();
    void updateAllWidgets();  // Helper to refresh all widgets from model
};

#endif // MAINWINDOW_H
