#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QWidget>
#include "welldialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , wellsWidget(nullptr)
    , tracersWidget(nullptr)
    , parametersWidget(nullptr)
    , observationsWidget(nullptr)
{
    ui->setupUi(this);

    setWindowTitle("ChronoGW - Groundwater Age Modeling");

    setupCentralWidget();

    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::onSaveAsFile);
    connect(wellsWidget, &IconListWidget::itemPropertiesRequested, this, &MainWindow::onEditWell);
}

void MainWindow::setupCentralWidget()
{
    // Create central widget with layout
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Create a grid layout for the four icon widgets
    QGridLayout* gridLayout = new QGridLayout();

    // Wells section (top-left)
    QLabel* wellsLabel = new QLabel("Wells", this);
    wellsLabel->setStyleSheet("font-size: 14pt;");
    wellsWidget = new IconListWidget(this);

    QVBoxLayout* wellsLayout = new QVBoxLayout();
    wellsLayout->addWidget(wellsLabel);
    wellsLayout->addWidget(wellsWidget);

    // Tracers section (top-right)
    QLabel* tracersLabel = new QLabel("Tracers", this);
    tracersLabel->setStyleSheet("font-size: 14pt;");
    tracersWidget = new IconListWidget(this);

    QVBoxLayout* tracersLayout = new QVBoxLayout();
    tracersLayout->addWidget(tracersLabel);
    tracersLayout->addWidget(tracersWidget);

    // Parameters section (bottom-left)
    QLabel* parametersLabel = new QLabel("Parameters", this);
    parametersLabel->setStyleSheet("font-size: 14pt;");
    parametersWidget = new IconListWidget(this);

    QVBoxLayout* parametersLayout = new QVBoxLayout();
    parametersLayout->addWidget(parametersLabel);
    parametersLayout->addWidget(parametersWidget);

    // Observations section (bottom-right)
    QLabel* observationsLabel = new QLabel("Observations", this);
    observationsLabel->setStyleSheet("font-size: 14pt;");
    observationsWidget = new IconListWidget(this);

    QVBoxLayout* observationsLayout = new QVBoxLayout();
    observationsLayout->addWidget(observationsLabel);
    observationsLayout->addWidget(observationsWidget);

    // Add to grid: row, column, rowspan, colspan
    QWidget* wellsContainer = new QWidget();
    wellsContainer->setLayout(wellsLayout);

    QWidget* tracersContainer = new QWidget();
    tracersContainer->setLayout(tracersLayout);

    QWidget* parametersContainer = new QWidget();
    parametersContainer->setLayout(parametersLayout);

    QWidget* observationsContainer = new QWidget();
    observationsContainer->setLayout(observationsLayout);

    gridLayout->addWidget(wellsContainer, 0, 0);
    gridLayout->addWidget(tracersContainer, 0, 1);
    gridLayout->addWidget(parametersContainer, 1, 0);
    gridLayout->addWidget(observationsContainer, 1, 1);

    mainLayout->addLayout(gridLayout);
    setCentralWidget(centralWidget);
}

void MainWindow::updateAllWidgets()
{
    wellsWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Well);
    tracersWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Tracer);
    parametersWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Parameter);
    observationsWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Observation);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOpenFile()
{
    // Open file dialog to select GWA configuration file
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open GWA Configuration File"),
        QString(),  // Start in current directory
        tr("GWA Files (*.gwa);;Text Files (*.txt);;All Files (*)")
        );

    // Check if user cancelled
    if (fileName.isEmpty()) {
        return;
    }

    // Try to load the file using GWA class
    bool success = gwaModel.loadFromFile(fileName.toStdString());

    if (success) {
        currentFilePath = fileName;
        statusBar()->showMessage(tr("Loaded: %1").arg(fileName), 3000);

        updateAllWidgets();

    } else {
        QMessageBox::critical(
            this,
            tr("Error Loading File"),
            tr("Failed to load configuration file:\n%1").arg(fileName)
            );
    }
}

void MainWindow::onSaveFile()
{
    // If we don't have a current file, use Save As
    if (currentFilePath.isEmpty()) {
        onSaveAsFile();
        return;
    }

    // TODO: Update gwaModel with data from UI widgets before saving
    // - Wells
    // - Tracers
    // - Parameters
    // - Observations

    // Save to file
    bool success = gwaModel.exportToFile(currentFilePath.toStdString());

    if (success) {
        statusBar()->showMessage(tr("Saved: %1").arg(currentFilePath), 3000);
    } else {
        QMessageBox::critical(
            this,
            tr("Error Saving File"),
            tr("Failed to save configuration file:\n%1").arg(currentFilePath)
            );
    }
}

void MainWindow::onSaveAsFile()
{
    // Always prompt for new filename
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save GWA Configuration File As"),
        currentFilePath.isEmpty() ? QString() : currentFilePath,
        tr("GWA Files (*.gwa);;Text Files (*.txt);;All Files (*)")
        );

    if (fileName.isEmpty()) {
        return;  // User cancelled
    }

    // TODO: Update gwaModel with data from UI widgets before saving
    // - Wells
    // - Tracers
    // - Parameters
    // - Observations

    // Save to file
    bool success = gwaModel.exportToFile(fileName.toStdString());

    if (success) {
        currentFilePath = fileName;  // Update current file path
        statusBar()->showMessage(tr("Saved: %1").arg(currentFilePath), 3000);
    } else {
        QMessageBox::critical(
            this,
            tr("Error Saving File"),
            tr("Failed to save configuration file:\n%1").arg(fileName)
            );
    }
}

#include "welldialog.h"

void MainWindow::onEditWell(const QString& name, const QVariant& data)
{
    int wellIndex = data.toInt();

    if (wellIndex < 0 || wellIndex >= static_cast<int>(gwaModel.getWellCount())) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid well index."));
        return;
    }

    // Get mutable reference to well
    CWell& well = gwaModel.getWellMutable(wellIndex);

    // Show dialog
    WellDialog dialog(&well, &gwaModel, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get updated well data
        CWell updatedWell = dialog.getWell();

        // Copy updated data back to model
        well = updatedWell;

        // Refresh the widget to show any name changes
        wellsWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Well);

        statusBar()->showMessage(tr("Well '%1' updated").arg(name), 3000);
    }
}

