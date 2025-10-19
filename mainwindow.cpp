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
#include "tracerdialog.h"

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

    connect(wellsWidget, &IconListWidget::itemPropertiesRequested,
            this, &MainWindow::onEditWell);
    connect(wellsWidget, &IconListWidget::itemRenamed,
            this, &MainWindow::onWellRenamed);
    connect(tracersWidget, &IconListWidget::itemPropertiesRequested,
            this, &MainWindow::onEditTracer);
    connect(tracersWidget, &IconListWidget::itemRenamed,
            this, &MainWindow::onTracerRenamed);
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

void MainWindow::onEditWell(const QString& name, const QVariant& data)
{
    int wellIndex = data.toInt();
    if (wellIndex < 0 || wellIndex >= static_cast<int>(gwaModel.getWellCount())) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid well index."));
        return;
    }

    // Get mutable reference to well
    CWell& well = gwaModel.getWellMutable(wellIndex);
    std::string originalWellName = well.getName();

    // Show dialog
    WellDialog dialog(&gwaModel, &well, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get updated well data
        CWell updatedWell = dialog.getWell();
        std::string newWellName = updatedWell.getName();

        // Get parameter linkages from dialog
        QMap<QString, QString> linkages = dialog.getParameterLinkages();

        // Convert to std::map
        std::map<std::string, std::string> linkagesStd;
        for (auto it = linkages.begin(); it != linkages.end(); ++it) {
            linkagesStd[it.key().toStdString()] = it.value().toStdString();
        }

        // If well name changed, clear linkages under old name
        if (originalWellName != newWellName) {
            gwaModel.clearParameterLinkages(originalWellName, "well");
        }

        // Copy updated data back to model
        well = updatedWell;

        // Update parameter linkages using CGWA's method
        gwaModel.updateWellParameterLinkages(newWellName, linkagesStd);

        // Refresh the widget to show any changes
        wellsWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Well);

        statusBar()->showMessage(
            tr("Well '%1' updated").arg(QString::fromStdString(newWellName)),
            3000
            );
    }
}

void MainWindow::onWellRenamed(const QString& oldName, const QString& newName, const QVariant& data)
{
    int wellIndex = data.toInt();

    if (wellIndex < 0 || wellIndex >= static_cast<int>(gwaModel.getWellCount())) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid well index."));
        return;
    }

    // Validate new name
    if (newName.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Name"),
                             tr("Well name cannot be empty."));
        // Revert the name in the widget
        wellsWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Well);
        return;
    }

    // Check for duplicate names
    for (size_t i = 0; i < gwaModel.getWellCount(); ++i) {
        if (i != static_cast<size_t>(wellIndex) &&
            gwaModel.getWell(i).getName() == newName.toStdString()) {
            QMessageBox::warning(this, tr("Duplicate Name"),
                                 tr("A well with name '%1' already exists.").arg(newName));
            // Revert the name
            wellsWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Well);
            return;
        }
    }

    // Get mutable reference to well
    CWell& well = gwaModel.getWellMutable(wellIndex);
    std::string oldNameStd = oldName.toStdString();
    std::string newNameStd = newName.toStdString();

    // Update parameter linkages to use new name
    // First, find all current linkages for this well
    std::map<std::string, std::string> currentLinkages;

    // Collect all possible quantities
    std::vector<std::string> quantities = {"f", "age_old", "fm", "vz_delay"};
    for (int i = 0; i < 10; ++i) {
        quantities.push_back("param[" + std::to_string(i) + "]");
    }

    // Find which parameters are linked to each quantity
    for (const auto& quantity : quantities) {
        std::string linkedParam = gwaModel.findLinkedParameter(oldNameStd, quantity, "well");
        if (!linkedParam.empty()) {
            currentLinkages[quantity] = linkedParam;
        }
    }

    // Clear old linkages under old name
    gwaModel.clearParameterLinkages(oldNameStd, "well");

    // Update the well name
    well.setName(newNameStd);

    // Re-add linkages under new name
    gwaModel.updateWellParameterLinkages(newNameStd, currentLinkages);

    statusBar()->showMessage(tr("Well renamed from '%1' to '%2'").arg(oldName, newName), 3000);
}

void MainWindow::onEditTracer(const QString& name, const QVariant& data)
{
    int tracerIndex = data.toInt();
    if (tracerIndex < 0 || tracerIndex >= static_cast<int>(gwaModel.getTracerCount())) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid tracer index."));
        return;
    }

    // Get mutable reference to tracer
    CTracer& tracer = gwaModel.getTracerMutable(tracerIndex);
    std::string originalTracerName = tracer.getName();

    // Show dialog
    TracerDialog dialog(&gwaModel, &tracer, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get updated tracer data
        CTracer updatedTracer = dialog.getTracer();
        std::string newTracerName = updatedTracer.getName();

        // Get parameter linkages from dialog
        QMap<QString, QString> linkages = dialog.getParameterLinkages();

        // Convert to std::map
        std::map<std::string, std::string> linkagesStd;
        for (auto it = linkages.begin(); it != linkages.end(); ++it) {
            linkagesStd[it.key().toStdString()] = it.value().toStdString();
        }

        // If tracer name changed, clear linkages under old name
        if (originalTracerName != newTracerName) {
            gwaModel.clearParameterLinkages(originalTracerName, "tracer");
        }

        // Copy updated data back to model
        tracer = updatedTracer;

        // Update parameter linkages using CGWA's method
        gwaModel.updateTracerParameterLinkages(newTracerName, linkagesStd);

        // Refresh the widget to show any changes
        tracersWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Tracer);

        statusBar()->showMessage(
            tr("Tracer '%1' updated").arg(QString::fromStdString(newTracerName)),
            3000
            );
    }
}

void MainWindow::onTracerRenamed(const QString& oldName, const QString& newName, const QVariant& data)
{
    int tracerIndex = data.toInt();

    if (tracerIndex < 0 || tracerIndex >= static_cast<int>(gwaModel.getTracerCount())) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid tracer index."));
        return;
    }

    // Validate new name
    if (newName.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Name"),
                             tr("Tracer name cannot be empty."));
        // Revert the name in the widget
        tracersWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Tracer);
        return;
    }

    // Check for duplicate names
    for (size_t i = 0; i < gwaModel.getTracerCount(); ++i) {
        if (i != static_cast<size_t>(tracerIndex) &&
            gwaModel.getTracer(i).getName() == newName.toStdString()) {
            QMessageBox::warning(this, tr("Duplicate Name"),
                                 tr("A tracer with name '%1' already exists.").arg(newName));
            // Revert the name
            tracersWidget->populateFromGWA(&gwaModel, IconListWidget::ItemType::Tracer);
            return;
        }
    }

    // Get mutable reference to tracer
    CTracer& tracer = gwaModel.getTracerMutable(tracerIndex);
    std::string oldNameStd = oldName.toStdString();
    std::string newNameStd = newName.toStdString();

    // Update parameter linkages to use new name
    // First, find all current linkages for this tracer
    std::map<std::string, std::string> currentLinkages;

    // Collect all possible quantities for tracers
    std::vector<std::string> quantities = {
        "input_multiplier", "decay", "retard",
        "co", "cm", "fm"
    };

    // Find which parameters are linked to each quantity
    for (const auto& quantity : quantities) {
        std::string linkedParam = gwaModel.findLinkedParameter(oldNameStd, quantity, "tracer");
        if (!linkedParam.empty()) {
            currentLinkages[quantity] = linkedParam;
        }
    }

    // Clear old linkages under old name
    gwaModel.clearParameterLinkages(oldNameStd, "tracer");

    // Update the tracer name
    tracer.setName(newNameStd);

    // Re-add linkages under new name
    gwaModel.updateTracerParameterLinkages(newNameStd, currentLinkages);

    statusBar()->showMessage(tr("Tracer renamed from '%1' to '%2'").arg(oldName, newName), 3000);
}
