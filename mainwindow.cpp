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
#include "parameterdialog.h"
#include "observationdialog.h"
#include "chartwindow.h"
#include <QStandardPaths>
#include "GASettingsDialog.h"
#include "MCMCSettingsDialog.h"


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
    connect(ui->actionGenetic_Algorithm_Settings, &QAction::triggered, this, &MainWindow::onGASettingsTriggered);
    connect(ui->actionMCMC_Settings, &QAction::triggered, this, &MainWindow::onMCMCSettingsTriggered);
    connect(ui->actionDeterministic_GA, &QAction::triggered, this, &MainWindow::onRunDeterministicGA);
    connect(ui->actionBayesian_MCMC, &QAction::triggered, this, &MainWindow::onRunMCMC);
    recentFilesMenu = new QMenu("Recent Projects", this);
    ui->actionRecent_Projects->setMenu(recentFilesMenu);
    updateRecentFilesMenu();

    ga = CGA<CGWA>(&gwaModel);

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
    wellsLabel->setStyleSheet("font-size: 14pt; font-weight: bold;");
    wellsWidget = new IconListWidget(this);
    wellsWidget->setGWA(&gwaModel);
    wellsWidget->setItemType(IconListWidget::ItemType::Well);

    QVBoxLayout* wellsLayout = new QVBoxLayout();
    wellsLayout->addWidget(wellsLabel);
    wellsLayout->addWidget(wellsWidget);

    // Tracers section (top-right)
    QLabel* tracersLabel = new QLabel("Tracers", this);
    tracersLabel->setStyleSheet("font-size: 14pt; font-weight: bold;");
    tracersWidget = new IconListWidget(this);
    tracersWidget->setGWA(&gwaModel);
    tracersWidget->setItemType(IconListWidget::ItemType::Tracer);

    QVBoxLayout* tracersLayout = new QVBoxLayout();
    tracersLayout->addWidget(tracersLabel);
    tracersLayout->addWidget(tracersWidget);

    // Parameters section (bottom-left)
    QLabel* parametersLabel = new QLabel("Parameters", this);
    parametersLabel->setStyleSheet("font-size: 14pt; font-weight: bold;");
    parametersWidget = new IconListWidget(this);
    parametersWidget->setGWA(&gwaModel);
    parametersWidget->setItemType(IconListWidget::ItemType::Parameter);

    QVBoxLayout* parametersLayout = new QVBoxLayout();
    parametersLayout->addWidget(parametersLabel);
    parametersLayout->addWidget(parametersWidget);

    // Observations section (bottom-right)
    QLabel* observationsLabel = new QLabel("Observations", this);
    observationsLabel->setStyleSheet("font-size: 14pt; font-weight: bold;");
    observationsWidget = new IconListWidget(this);
    observationsWidget->setGWA(&gwaModel);
    observationsWidget->setItemType(IconListWidget::ItemType::Observation);

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

    // Connect signals
    connect(wellsWidget, &IconListWidget::addItemRequested,
            this, &MainWindow::onAddWell);
    connect(wellsWidget, &IconListWidget::itemPropertiesRequested,
            this, &MainWindow::onEditWell);
    connect(wellsWidget, &IconListWidget::listModified,
            this, &MainWindow::onModelModified);

    connect(tracersWidget, &IconListWidget::addItemRequested,
            this, &MainWindow::onAddTracer);
    connect(tracersWidget, &IconListWidget::itemPropertiesRequested,
            this, &MainWindow::onEditTracer);
    connect(tracersWidget, &IconListWidget::listModified,
            this, &MainWindow::onModelModified);

    connect(parametersWidget, &IconListWidget::addItemRequested,
            this, &MainWindow::onAddParameter);
    connect(parametersWidget, &IconListWidget::itemPropertiesRequested,
            this, &MainWindow::onEditParameter);
    connect(parametersWidget, &IconListWidget::listModified,
            this, &MainWindow::onModelModified);

    connect(observationsWidget, &IconListWidget::addItemRequested,
            this, &MainWindow::onAddObservation);
    connect(observationsWidget, &IconListWidget::itemPropertiesRequested,
            this, &MainWindow::onEditObservation);
    connect(observationsWidget, &IconListWidget::listModified,
            this, &MainWindow::onModelModified);
    connect(observationsWidget, &IconListWidget::itemContextActionRequested,  // <-- ADD THIS
            this, &MainWindow::onObservationContextAction);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ============================================================================
// Updated onOpenFile() - Add MCMC settings loading
// ============================================================================

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
        currentFilePath_ = fileName;
        statusBar()->showMessage(tr("Loaded: %1").arg(fileName), 3000);

        // Create fresh GA object linked to the loaded model
        ga = CGA<CGWA>(&gwaModel);

        // Create fresh MCMC object linked to the loaded model
        mcmc = CMCMC<CGWA>(&gwaModel);

        // Load GA settings if available
        QString gaFile = getGASettingsFilename(fileName);
        if (QFile::exists(gaFile)) {
            loadGASettings(gaFile);
        }

        // Load MCMC settings if available
        QString mcmcFile = getMCMCSettingsFilename(fileName);
        if (QFile::exists(mcmcFile)) {
            loadMCMCSettings(mcmcFile);
        }

        updateAllWidgets();

    } else {
        QMessageBox::critical(
            this,
            tr("Error Loading File"),
            tr("Failed to load configuration file:\n%1").arg(fileName)
            );
    }
    setModified(false);
    addToRecentFiles(currentFilePath_);
}

// ============================================================================
// Updated onSaveFile() - Add MCMC settings saving
// ============================================================================

void MainWindow::onSaveFile()
{
    // If we don't have a current file, use Save As
    if (currentFilePath_.isEmpty()) {
        onSaveAsFile();
        return;
    }

    // TODO: Update gwaModel with data from UI widgets before saving
    // - Wells
    // - Tracers
    // - Parameters
    // - Observations

    // Save to file
    bool success = gwaModel.exportToFile(currentFilePath_.toStdString());

    if (success) {
        statusBar()->showMessage(tr("Saved: %1").arg(currentFilePath_), 3000);

        // Save GA settings
        QString gaFile = getGASettingsFilename(currentFilePath_);
        saveGASettings(gaFile);

        // Save MCMC settings
        QString mcmcFile = getMCMCSettingsFilename(currentFilePath_);
        saveMCMCSettings(mcmcFile);

    } else {
        QMessageBox::critical(
            this,
            tr("Error Saving File"),
            tr("Failed to save configuration file:\n%1").arg(currentFilePath_)
            );
    }
    setModified(false);
    addToRecentFiles(currentFilePath_);
}

// ============================================================================
// Updated onSaveAsFile() - Add MCMC settings saving
// ============================================================================

void MainWindow::onSaveAsFile()
{
    // Open save file dialog
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save GWA Configuration File As"),
        currentFilePath_.isEmpty() ? QString() : currentFilePath_,
        tr("GWA Files (*.gwa);;Text Files (*.txt);;All Files (*)")
        );

    // Check if user cancelled
    if (fileName.isEmpty()) {
        return;
    }

    // Ensure proper extension
    if (!fileName.endsWith(".gwa", Qt::CaseInsensitive) &&
        !fileName.endsWith(".txt", Qt::CaseInsensitive)) {
        fileName += ".gwa";
    }

    // TODO: Update gwaModel with data from UI widgets before saving
    // - Wells
    // - Tracers
    // - Parameters
    // - Observations

    // Save to file
    bool success = gwaModel.exportToFile(fileName.toStdString());

    if (success) {
        currentFilePath_ = fileName;
        statusBar()->showMessage(tr("Saved: %1").arg(fileName), 3000);

        // Save GA settings
        QString gaFile = getGASettingsFilename(currentFilePath_);
        saveGASettings(gaFile);

        // Save MCMC settings
        QString mcmcFile = getMCMCSettingsFilename(currentFilePath_);
        saveMCMCSettings(mcmcFile);

    } else {
        QMessageBox::critical(
            this,
            tr("Error Saving File"),
            tr("Failed to save configuration file:\n%1").arg(fileName)
            );
    }
    setModified(false);
    addToRecentFiles(currentFilePath_);
}

// ============================================================================
// Updated onRecentFileTriggered() - Add MCMC settings loading
// ============================================================================

void MainWindow::onRecentFileTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString filePath = action->data().toString();
        if (QFile::exists(filePath)) {
            currentFilePath_ = filePath;
            // Load the file using your existing load logic
            gwaModel.loadFromFile(filePath.toStdString());

            // Create fresh GA object linked to the loaded model
            ga = CGA<CGWA>(&gwaModel);

            // Create fresh MCMC object linked to the loaded model
            mcmc = CMCMC<CGWA>(&gwaModel);

            // Load GA settings if available
            QString gaFile = getGASettingsFilename(filePath);
            if (QFile::exists(gaFile)) {
                loadGASettings(gaFile);
            }

            // Load MCMC settings if available
            QString mcmcFile = getMCMCSettingsFilename(filePath);
            if (QFile::exists(mcmcFile)) {
                loadMCMCSettings(mcmcFile);
            }

            updateAllWidgets();
            setModified(false);
            addToRecentFiles(filePath);
        } else {
            QMessageBox::warning(this, "File Not Found",
                                 "The file no longer exists:\n" + filePath);
            // Remove from recent files
            QStringList files = loadRecentFiles();
            files.removeAll(filePath);
            saveRecentFiles(files);
            updateRecentFilesMenu();
        }
    }
}

void MainWindow::onEditWell(const QString& wellName, int index)
{
    if (index < 0 || static_cast<size_t>(index) >= gwaModel.getWellCount()) {
        return;
    }

    // Get mutable reference to the well
    CWell& well = gwaModel.getWellMutable(index);

    // Create dialog for editing (pass pointer to well)
    WellDialog dialog(&gwaModel, &well, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get updated well from dialog
        CWell updatedWell = dialog.getWell();

        // Update the well in the model
        well = updatedWell;

        // Update parameter linkages
        QMap<QString, QString> linkages = dialog.getParameterLinkages();
        std::map<std::string, std::string> linkagesMap;
        for (auto it = linkages.begin(); it != linkages.end(); ++it) {
            linkagesMap[it.key().toStdString()] = it.value().toStdString();
        }
        gwaModel.updateWellParameterLinkages(well.getName(), linkagesMap);

        // Refresh the wells list
        wellsWidget->refresh();

        // Mark as modified
        setModified(true);
    }
}

void MainWindow::onEditTracer(const QString& tracerName, int index)
{
    if (index < 0 || static_cast<size_t>(index) >= gwaModel.getTracerCount()) {
        return;
    }

    // Get mutable reference to the tracer
    CTracer& tracer = gwaModel.getTracerMutable(index);

    // Create dialog for editing (pass pointer to tracer)
    TracerDialog dialog(&gwaModel, &tracer, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get updated tracer from dialog
        CTracer updatedTracer = dialog.getTracer();

        // Update the tracer in the model
        tracer = updatedTracer;

        // Update parameter linkages
        QMap<QString, QString> linkages = dialog.getParameterLinkages();
        std::map<std::string, std::string> linkagesMap;
        for (auto it = linkages.begin(); it != linkages.end(); ++it) {
            linkagesMap[it.key().toStdString()] = it.value().toStdString();
        }
        gwaModel.updateTracerParameterLinkages(tracer.getName(), linkagesMap);

        // Refresh the tracers list
        tracersWidget->refresh();

        // Mark as modified
        setModified(true);
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
        wellsWidget->refresh();
        return;
    }

    // Check for duplicate names
    for (size_t i = 0; i < gwaModel.getWellCount(); ++i) {
        if (i != static_cast<size_t>(wellIndex) &&
            gwaModel.getWell(i).getName() == newName.toStdString()) {
            QMessageBox::warning(this, tr("Duplicate Name"),
                                 tr("A well with name '%1' already exists.").arg(newName));
            // Revert the name
            wellsWidget->refresh();
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
        tracersWidget->refresh();
        return;
    }

    // Check for duplicate names
    for (size_t i = 0; i < gwaModel.getTracerCount(); ++i) {
        if (i != static_cast<size_t>(tracerIndex) &&
            gwaModel.getTracer(i).getName() == newName.toStdString()) {
            QMessageBox::warning(this, tr("Duplicate Name"),
                                 tr("A tracer with name '%1' already exists.").arg(newName));
            // Revert the name
            tracersWidget->refresh();
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

void MainWindow::onAddWell()
{
    // Create dialog for new well (pass nullptr for well pointer to indicate new)
    WellDialog dialog(&gwaModel, nullptr, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get the well from dialog
        CWell well = dialog.getWell();

        // Add to model
        gwaModel.addWell(well);

        // Get parameter linkages and apply them
        QMap<QString, QString> linkages = dialog.getParameterLinkages();
        if (!linkages.isEmpty()) {
            std::map<std::string, std::string> linkagesMap;
            for (auto it = linkages.begin(); it != linkages.end(); ++it) {
                linkagesMap[it.key().toStdString()] = it.value().toStdString();
            }
            gwaModel.updateWellParameterLinkages(well.getName(), linkagesMap);
        }

        // Refresh the wells list
        wellsWidget->refresh();

        // Mark as modified
        setModified(true);
    }
}

void MainWindow::onAddTracer()
{
    // Create dialog for new tracer (pass nullptr for tracer pointer to indicate new)
    TracerDialog dialog(&gwaModel, nullptr, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get the tracer from dialog
        CTracer tracer = dialog.getTracer();

        // Add to model
        gwaModel.addTracer(tracer);

        // Get parameter linkages and apply them
        QMap<QString, QString> linkages = dialog.getParameterLinkages();
        if (!linkages.isEmpty()) {
            std::map<std::string, std::string> linkagesMap;
            for (auto it = linkages.begin(); it != linkages.end(); ++it) {
                linkagesMap[it.key().toStdString()] = it.value().toStdString();
            }
            gwaModel.updateTracerParameterLinkages(tracer.getName(), linkagesMap);
        }

        // Refresh the tracers list
        tracersWidget->refresh();

        // Mark as modified
        setModified(true);
    }
}


void MainWindow::onAddParameter()
{
    // Create dialog for new parameter
    ParameterDialog dialog(&gwaModel, nullptr, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get the parameter from dialog
        Parameter param = dialog.getParameter();

        // Add to model
        gwaModel.addParameter(param);

        // Refresh the parameters list
        parametersWidget->refresh();

        // Mark as modified
        setModified(true);
    }
}



void MainWindow::onAddObservation()
{
    // Create dialog for new observation
    ObservationDialog dialog(&gwaModel, nullptr, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get the observation from dialog
        Observation obs = dialog.getObservation();

        // Add to model
        gwaModel.addObservation(obs);

        // Refresh the observations list
        observationsWidget->refresh();

        // Mark as modified
        setModified(true);
    }
}

void MainWindow::onModelModified()
{
    // Called when any list widget modifies the model
    // (items added, removed, or renamed)

    // Refresh all widgets to ensure consistency
    // (e.g., if a well is deleted, its observations should disappear)
    updateAllWidgets();

    // Mark document as modified
    setModified(true);
}

void MainWindow::setModified(bool modified)
{
    isModified_ = modified;
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    QString title = "ChronoGW";

    if (!currentFilePath_.isEmpty()) {
        QFileInfo fileInfo(currentFilePath_);
        title = fileInfo.fileName() + " - ChronoGW";
    } else {
        title = "Untitled - ChronoGW";
    }

    if (isModified_) {
        title += " *";  // Add asterisk to indicate unsaved changes
    }

    setWindowTitle(title);
}

void MainWindow::updateAllWidgets()
{
    wellsWidget->refresh();
    tracersWidget->refresh();
    parametersWidget->refresh();
    observationsWidget->refresh();
}

void MainWindow::onEditParameter(const QString& paramName, int index)
{
    if (index < 0 || static_cast<size_t>(index) >= gwaModel.getParameterCount()) {
        return;
    }

    // Get mutable reference to the parameter
    Parameter* param = gwaModel.getParameter(index);
    if (!param) return;

    // Create dialog for editing
    ParameterDialog dialog(&gwaModel, param, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get updated parameter from dialog
        Parameter updatedParam = dialog.getParameter();

        // Update the parameter in the model
        *param = updatedParam;

        // Refresh the parameters list
        parametersWidget->refresh();

        // Mark as modified
        setModified(true);
    }
}

void MainWindow::onEditObservation(const QString& obsName, int index)
{
    if (index < 0 || static_cast<size_t>(index) >= gwaModel.getObservationCount()) {
        return;
    }

    // Get mutable reference to the observation
    Observation* obs = gwaModel.observation(index);
    if (!obs) return;

    // Create dialog for editing
    ObservationDialog dialog(&gwaModel, obs, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Get updated observation from dialog
        Observation updatedObs = dialog.getObservation();

        // Update the observation in the model
        *obs = updatedObs;

        // Refresh the observations list
        observationsWidget->refresh();

        // Mark as modified
        setModified(true);
    }
}

void MainWindow::onObservationContextAction(const QString& obsName, int index, const QString& actionType)
{
    if (actionType == "plot") {
        onPlotObservation(obsName, index);
    }
    // Can add more action types here in the future if needed
}

void MainWindow::onPlotObservation(const QString& obsName, int index)
{
    if (index < 0 || static_cast<size_t>(index) >= gwaModel.getObservationCount()) {
        return;
    }

    try {
        const Observation& obs = gwaModel.getObservation(index);

        // Get observed data
        const TimeSeries<double>& observedData = obs.GetObservedData();

        if (observedData.size() == 0) {
            QMessageBox::warning(this, tr("No Data"),
                                 tr("Observation '%1' has no observed data.").arg(obsName));
            return;
        }

        // Calculate modeled data
        TimeSeries<double> modeledData = gwaModel.calculateSingleObservation(index);

        if (modeledData.size() == 0) {
            QMessageBox::warning(this, tr("Calculation Failed"),
                                 tr("Failed to calculate modeled concentrations for '%1'.").arg(obsName));
            return;
        }

        // Create TimeSeriesSet with both datasets
        TimeSeriesSet<double> dataSet;

        TimeSeries<double> obsData = observedData;
        obsData.setName("Observed");
        dataSet.append(obsData);

        modeledData.setName("Modeled");
        dataSet.append(modeledData);

        // Create chart title
        QString title = QString("Concentration Comparison: %1\n%2 - %3")
                            .arg(obsName)
                            .arg(QString::fromStdString(obs.GetLocation()))
                            .arg(QString::fromStdString(obs.GetQuantity()));

        // Show in chart window
        ChartWindow* chartWindow = ChartWindow::showChart(dataSet, title, this);
        chartWindow->setAxisLabels("Time", "Concentration");
        chartWindow->chartViewer()->setPlotMode(ChartViewer::Symbols);

        // Calculate and display fit statistics
        double sse = 0.0;
        double mae = 0.0;
        int n = std::min(observedData.size(), modeledData.size());

        for (int i = 0; i < n; ++i) {
            double diff = observedData.getValue(i) - modeledData.getValue(i);
            sse += diff * diff;
            mae += std::abs(diff);
        }

        double rmse = std::sqrt(sse / n);
        mae /= n;

        QString stats = QString("  [RMSE: %1  MAE: %2]")
                            .arg(rmse, 0, 'g', 4)
                            .arg(mae, 0, 'g', 4);

        chartWindow->setWindowTitle(chartWindow->windowTitle() + stats);

    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Failed to plot observation:\n%1").arg(e.what()));
    }
}

QString MainWindow::getRecentFilesPath() const
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QDir dir(documentsPath);
    if (!dir.exists("ChronoGW")) {
        dir.mkdir("ChronoGW");
    }
    return documentsPath + "/ChronoGW/recent_files.txt";
}

QStringList MainWindow::loadRecentFiles() const
{
    QStringList files;
    QFile file(getRecentFilesPath());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty() && QFile::exists(line)) {
                files.append(line);
            }
        }
        file.close();
    }
    return files;
}

void MainWindow::saveRecentFiles(const QStringList& files) const
{
    QFile file(getRecentFilesPath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString& filePath : files) {
            out << filePath << "\n";
        }
        file.close();
    }
}

void MainWindow::addToRecentFiles(const QString& filePath)
{
    QStringList files = loadRecentFiles();

    // Remove if already exists
    files.removeAll(filePath);

    // Add to front
    files.prepend(filePath);

    // Keep only MaxRecentFiles
    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }

    saveRecentFiles(files);
    updateRecentFilesMenu();
}

void MainWindow::updateRecentFilesMenu()
{
    recentFilesMenu->clear();

    QStringList files = loadRecentFiles();

    if (files.isEmpty()) {
        QAction* noFilesAction = recentFilesMenu->addAction("No Recent Files");
        noFilesAction->setEnabled(false);
        return;
    }

    for (const QString& filePath : files) {
        QAction* action = recentFilesMenu->addAction(QFileInfo(filePath).fileName());
        action->setData(filePath);
        action->setToolTip(filePath);
        connect(action, &QAction::triggered, this, &MainWindow::onRecentFileTriggered);
    }
}

void MainWindow::onGASettingsTriggered()
{
    GASettingsDialog dialog(&ga, this);
    dialog.exec();
}

void MainWindow::onMCMCSettingsTriggered()
{
    MCMCSettingsDialog dialog(&mcmc, this);
    dialog.exec();
}

// ============================================================================
// Helper: Get GA settings filename from project filename
// ============================================================================
QString MainWindow::getGASettingsFilename(const QString& projectFilename)
{
    QFileInfo fileInfo(projectFilename);
    QString baseName = fileInfo.completeBaseName();
    QString path = fileInfo.absolutePath();
    return path + "/" + baseName + ".gasettings";
}

// ============================================================================
// Save GA settings to file
// ============================================================================
void MainWindow::saveGASettings(const QString& filename)
{
    std::ofstream file(filename.toStdString());
    if (!file.is_open()) {
        return;
    }

    // Write GA parameters from the actual GA object
    file << "maxpop " << ga.getPopulationSize() << "\n";
    file << "ngen " << ga.getNumGenerations() << "\n";
    file << "pcross " << ga.getCrossoverProb() << "\n";
    file << "pmute " << ga.getMutationProb() << "\n";
    file << "shakescale " << ga.getShakeScale() << "\n";
    file << "shakescalered " << ga.getShakeScaleRed() << "\n";
    file << "numthreads " << ga.getNumThreads() << "\n";

    file.close();
}

// ============================================================================
// Updated saveMCMCSettings() for mainwindow.cpp
// ============================================================================

void MainWindow::saveMCMCSettings(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);

    // Get current settings from MCMC object
    const MCMCSettings& settings = mcmc.GetSettings();

    // Write MCMC parameters with actual values from the MCMC object
    out << "number_of_samples " << settings.total_number_of_samples << "\n";
    out << "number_of_chains " << settings.number_of_chains << "\n";
    out << "number_of_burnout_samples " << settings.burnout_samples << "\n";
    out << "record_interval " << settings.save_interval << "\n";
    out << "perturbation_factor " << settings.perturbation_factor << "\n";
    out << "perturbation_change_scale " << settings.perturbation_change_scale << "\n";
    out << "acceptance_rate " << settings.acceptance_rate << "\n";
    out << "number_of_threads " << settings.numberOfThreads << "\n";

    // Boolean settings
    // Note: settings.no_initial_perturbation is TRUE when we should SKIP perturbation
    // The property "initial_perturbation" expects "yes" to DO perturbation, "no" to SKIP
    out << "initial_perturbation " << (settings.no_initial_perturbation ? "no" : "yes") << "\n";
    out << "sensitivity_based_perturbation " << (settings.sensitivity_based_perturbation ? "yes" : "no") << "\n";
    out << "perform_global_sensitivity " << (settings.global_sensitivity ? "yes" : "no") << "\n";

    // Post-processing
    out << "number_of_post_estimate_realizations " << settings.number_of_post_estimate_realizations << "\n";
    out << "increment_for_sensitivity_analysis " << settings.increment_for_sensitivity << "\n";
    out << "add_noise_to_realizations " << (settings.noise_realization_writeout ? "yes" : "no") << "\n";

    // Continue file (only if set)
    if (!settings.continue_filename.empty()) {
        out << "continue_based_on_file_name " << QString::fromStdString(settings.continue_filename) << "\n";
    }

    file.close();
}

// ============================================================================
// loadMCMCSettings() remains the same - it already uses SetProperty correctly
// ============================================================================

void MainWindow::loadMCMCSettings(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty()) {
            continue;
        }

        QStringList tokens = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        if (tokens.size() >= 2) {
            QString key = tokens[0];
            QString value = tokens[1];

            mcmc.SetProperty(key.toStdString(), value.toStdString());
        }
    }

    file.close();
}
// ============================================================================
// Load GA settings from file
// ============================================================================
void MainWindow::loadGASettings(const QString& filename)
{
    qDebug() << "=== DEBUG: loadGASettings called with:" << filename;

    std::ifstream file(filename.toStdString());
    if (!file.is_open()) {
        qDebug() << "ERROR: Could not open file!";
        return;
    }

    qDebug() << "DEBUG: File opened successfully";

    std::vector<std::string> s;
    int lineCount = 0;
    while (!file.eof())
    {
        s = aquiutils::getline(file, ' ');
        lineCount++;
        qDebug() << "DEBUG: Line" << lineCount << "- tokens:" << s.size();

        if (s.size() > 1)
        {
            const std::string& key = s[0];
            const std::string& value = s[1];

            qDebug() << "DEBUG: Setting" << QString::fromStdString(key)
                     << "=" << QString::fromStdString(value);

            bool result = false;
            if (key == "maxpop")
                result = ga.SetProperty("maxpop", value);
            else if (key == "ngen")
                result = ga.SetProperty("ngen", value);
            else if (key == "pcross")
                result = ga.SetProperty("pcross", value);
            else if (key == "pmute")
                result = ga.SetProperty("pmute", value);
            else if (key == "shakescale")
                result = ga.SetProperty("shakescale", value);
            else if (key == "shakescalered")
                result = ga.SetProperty("shakescalered", value);
            else if (key == "outputfile")
                result = ga.SetProperty("outputfile", value);
            else if (key == "initial_population")
                result = ga.SetProperty("initial_population", value);
            else if (key == "numthreads")
                result = ga.SetProperty("numthreads", value);

            qDebug() << "DEBUG: SetProperty returned:" << result;
            if (!result) {
                qDebug() << "ERROR:" << QString::fromStdString(ga.getLastError());
            }
        }
    }

    file.close();
    qDebug() << "=== DEBUG: loadGASettings finished, total lines:" << lineCount;
}
void MainWindow::onRunDeterministicGA()
{
    // Check if model is loaded
    if (gwaModel.Parameters().empty()) {
        QMessageBox::warning(this, "No Model",
                             "Please load a model file before running optimization.");
        return;
    }

    // Check if we have a current file path
    if (currentFilePath_.isEmpty()) {
        QMessageBox::warning(this, "No File",
                             "Please load or save a file first.");
        return;
    }

    // ========================================================================
    // Create output folder next to input file (no timestamp)
    // ========================================================================

    QFileInfo inputFileInfo(currentFilePath_);
    QString inputDir = inputFileInfo.absolutePath();
    QString baseName = inputFileInfo.completeBaseName();

    // Create output folder (simple name, no timestamp)
    QString outputFolderName = QString("%1_GA_output").arg(baseName);
    QString outputFolderPath = inputDir + "/" + outputFolderName;

    // Create the directory if it doesn't exist
    QDir dir;
    if (!dir.exists(outputFolderPath)) {
        if (!dir.mkpath(outputFolderPath)) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to create output folder:\n%1").arg(outputFolderPath));
            return;
        }
    }

    // Set output paths for GWA
    gwaModel.SetOutputPath(outputFolderPath.toStdString() + "/");

    // DON'T create new GA - use existing one with settings from dialog
    // ga = CGA<CGWA>(&gwaModel);  // <-- REMOVED THIS LINE

    // Set GA output path and filename
    ga.SetProperty("pathname", outputFolderPath.toStdString() + "/");
    ga.SetProperty("outputfile", "ga_results.txt");

    // Get number of generations (already set from GA Settings dialog)
    int numGenerations = ga.getNumGenerations();

    // ========================================================================
    // Create progress window
    // ========================================================================

    progressWindow_ = new ProgressWindow(this, "GA Optimization");
    progressWindow_->SetProgressLabel("Generation Progress:");
    progressWindow_->SetPrimaryChartTitle("Best Fitness per Generation");
    progressWindow_->SetPrimaryChartYAxisTitle("Best Fitness per Generation");
    progressWindow_->SetPrimaryChartXAxisTitle("Generation");
    progressWindow_->SetPrimaryChartVisible(true);
    progressWindow_->SetSecondaryChartVisible(false);
    progressWindow_->SetSecondaryProgressVisible(true);
    progressWindow_->SetSecondaryProgressLabel("Within generation:");

    // Set X-axis range to number of generations
    progressWindow_->SetPrimaryChartXRange(0, numGenerations);

    // Set the progress window in GA
    ga.SetProgressWindow(progressWindow_);

    // Show window
    progressWindow_->show();
    progressWindow_->SetStatus("Initializing GA...");
    progressWindow_->AppendLog("Starting Genetic Algorithm Optimization");
    progressWindow_->AppendLog(QString("Input file: %1").arg(inputFileInfo.fileName()));
    progressWindow_->AppendLog(QString("Output folder: %1").arg(outputFolderName));
    progressWindow_->AppendLog(QString("Generations: %1").arg(numGenerations));
    progressWindow_->AppendLog(QString("Population size: %1").arg(ga.getPopulationSize()));
    progressWindow_->AppendLog("");

    // Process events to show window
    QApplication::processEvents();

    try {
        // Initialize GA
        ga.initFromModel(&gwaModel);
        ga.initialize();

        progressWindow_->SetStatus("Running optimization...");
        progressWindow_->AppendLog("Starting generation loop...");
        QApplication::processEvents();

        // Run optimization - progress updates happen automatically in GA
        int bestIndex = ga.optimize();

        CGWA* bestModel = ga.getBestModel();
        if (bestModel != nullptr) {
            progressWindow_->AppendLog("");
            progressWindow_->AppendLog("=== Updating Model Parameters ===");

            // Copy parameter values from best model to main model
            Parameter_Set& mainParams = gwaModel.Parameters();
            Parameter_Set& bestParams = bestModel->Parameters();

            for (size_t i = 0; i < mainParams.size() && i < bestParams.size(); ++i) {
                double newValue = bestParams[i]->GetValue();
                mainParams[i]->SetValue(newValue);

                progressWindow_->AppendLog(QString("  Updated %1 = %2")
                                               .arg(QString::fromStdString(mainParams[i]->GetName()), -30)
                                               .arg(newValue, 0, 'e', 6));
            }

            progressWindow_->AppendLog("Model parameters updated with optimized values.");
        }

        // Get results
        const std::vector<double>& finalParams = ga.getFinalParams();
        double bestFitness = ga.getMaxFitness();
        const auto& paramNames = ga.getParamNames();

        // Update progress window with final results
        progressWindow_->SetProgress(1.0);
        progressWindow_->SetStatus("Optimization Complete!");
        progressWindow_->AppendLog("");
        progressWindow_->AppendLog("=== Optimization Complete ===");
        progressWindow_->AppendLog(QString("Best fitness: %1").arg(bestFitness, 0, 'e', 6));
        progressWindow_->AppendLog(QString("Results saved to: %1").arg(outputFolderPath));
        progressWindow_->AppendLog("");
        progressWindow_->AppendLog("Optimized parameter values:");

        for (size_t i = 0; i < finalParams.size() && i < paramNames.size(); ++i) {
            progressWindow_->AppendLog(QString("  %1 = %2")
                                           .arg(QString::fromStdString(paramNames[i]), -30)
                                           .arg(finalParams[i], 0, 'e', 6));
        }

        progressWindow_->SetComplete("Optimization Complete!");

        // Update status bar
        statusBar()->showMessage(
            QString("GA Complete: Best Fitness = %1 | Output: %2")
                .arg(bestFitness, 0, 'e', 6)
                .arg(outputFolderName),
            10000
            );

        // Show results dialog
        QString resultMsg = QString("Optimization Complete!\n\n");
        resultMsg += QString("Best Fitness: %1\n\n").arg(bestFitness, 0, 'e', 6);
        resultMsg += QString("Optimized %1 parameters over %2 generations\n")
                         .arg(paramNames.size())
                         .arg(numGenerations);
        resultMsg += QString("\nResults saved to:\n%1\n\n").arg(outputFolderPath);
        resultMsg += "See progress window for detailed results.";

        QMessageBox::information(this, "GA Complete", resultMsg);

    } catch (const std::exception& e) {
        if (progressWindow_) {
            progressWindow_->AppendLog(QString("ERROR: %1").arg(e.what()));
            progressWindow_->SetComplete("Optimization Failed!");
        }

        QMessageBox::critical(this, "Optimization Error",
                              QString("Error during optimization:\n%1").arg(e.what()));
    }

    // Keep window open until user closes it
    if (progressWindow_) {
        progressWindow_->exec();
        delete progressWindow_;
        progressWindow_ = nullptr;
    }
}


QString MainWindow::getMCMCSettingsFilename(const QString& projectFilename)
{
    QFileInfo fileInfo(projectFilename);
    QString baseName = fileInfo.completeBaseName();
    QString path = fileInfo.absolutePath();
    return path + "/" + baseName + ".mcmcsettings";
}

void MainWindow::onRunMCMC()
{
    // Check if model is loaded
    if (gwaModel.Parameters().empty()) {
        QMessageBox::warning(this, "No Model",
                             "Please load a model file before running MCMC.");
        return;
    }

    // Check if we have parameters to estimate
    int numParams = gwaModel.Parameters().size();

    if (numParams == 0) {
        QMessageBox::warning(this, "No Parameters",
                             "Please define at least one parameter to estimate.");
        return;
    }

    // Check if we have a current file path
    if (currentFilePath_.isEmpty()) {
        QMessageBox::warning(this, "No File",
                             "Please load or save a file first.");
        return;
    }

    // ========================================================================
    // Create output folder next to input file
    // ========================================================================

    QFileInfo inputFileInfo(currentFilePath_);
    QString inputDir = inputFileInfo.absolutePath();
    QString baseName = inputFileInfo.completeBaseName();

    QString outputFolderName = QString("%1_MCMC_output").arg(baseName);
    QString outputFolderPath = inputDir + "/" + outputFolderName;

    QDir dir;
    if (!dir.exists(outputFolderPath)) {
        if (!dir.mkpath(outputFolderPath)) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to create output folder:\n%1").arg(outputFolderPath));
            return;
        }
    }

    // Set output paths for MCMC
    mcmc.SetProperty("outputpath", outputFolderPath.toStdString() + "/");
    mcmc.SetProperty("samples_filename", "mcmc_samples.txt");
    mcmc.SetProperty("detail_filename", "mcmc_detail.txt");

    // Get MCMC settings for display
    const MCMCSettings& settings = mcmc.GetSettings();
    int totalSamples = settings.total_number_of_samples;
    int numChains = settings.number_of_chains;
    int burnout = settings.burnout_samples;

    // ========================================================================
    // Create progress window
    // ========================================================================

    progressWindow_ = new ProgressWindow(this, "MCMC Sampling");
    progressWindow_->SetProgressLabel("Sample Progress:");
    progressWindow_->SetPrimaryChartTitle("Perturbation factor");
    progressWindow_->SetPrimaryChartVisible(true);
    progressWindow_->SetPrimaryChartXRange(0, totalSamples);
    progressWindow_->SetPrimaryChartYAxisTitle("Perturbation factor");
    progressWindow_->SetPrimaryChartXAxisTitle("Sample");

    progressWindow_->SetSecondaryChartVisible(true);  // Show MCMC-specific charts
    progressWindow_->SetSecondaryProgressVisible(false);
    progressWindow_->SetSecondaryChartTitle("Acceptance rate");
    progressWindow_->SetSecondaryChartYAxisTitle("Acceptance rate");
    progressWindow_->SetSecondaryChartXAxisTitle("Sample");
    progressWindow_->SetSecondaryChartXRange(0, totalSamples);


    // Set the progress window in MCMC
    mcmc.SetRunTimeWindow(progressWindow_);

    // Show window
    progressWindow_->show();
    progressWindow_->SetStatus("Initializing MCMC...");
    progressWindow_->AppendLog("Starting Markov Chain Monte Carlo Sampling");
    progressWindow_->AppendLog(QString("Input file: %1").arg(inputFileInfo.fileName()));
    progressWindow_->AppendLog(QString("Output folder: %1").arg(outputFolderName));
    progressWindow_->AppendLog(QString("Total samples: %1").arg(totalSamples));
    progressWindow_->AppendLog(QString("Number of chains: %1").arg(numChains));
    progressWindow_->AppendLog(QString("Burnout samples: %1").arg(burnout));
    progressWindow_->AppendLog(QString("Parameters to estimate: %1").arg(numParams));
    progressWindow_->AppendLog("");

    // Process events to show window
    QApplication::processEvents();

    try {
        // Initialize MCMC (use false for non-random start from current values)
        progressWindow_->SetStatus("Initializing chains...");
        progressWindow_->AppendLog("Initializing MCMC chains...");
        QApplication::processEvents();

        mcmc.Initialize(false);

        progressWindow_->SetStatus("Running MCMC sampling...");
        progressWindow_->AppendLog("Starting MCMC sampling loop...");
        progressWindow_->AppendLog("");
        QApplication::processEvents();

        // Run MCMC - progress updates happen automatically through ProgressWindow
        mcmc.Perform();

        // Get results
        double acceptanceRate = mcmc.GetAcceptanceRate();
        const auto& samples = mcmc.GetParameterSamples();

        // Update progress window with final results
        progressWindow_->SetProgress(1.0);
        progressWindow_->SetStatus("MCMC Complete!");
        progressWindow_->AppendLog("");
        progressWindow_->AppendLog("=== MCMC Sampling Complete ===");
        progressWindow_->AppendLog(QString("Total samples generated: %1").arg(totalSamples));
        progressWindow_->AppendLog(QString("Acceptance rate: %1%").arg(acceptanceRate * 100.0, 0, 'f', 2));
        progressWindow_->AppendLog(QString("Results saved to: %1").arg(outputFolderPath));
        progressWindow_->AppendLog("");

        // Calculate and display parameter statistics
        if (!samples.empty() && samples.size() > burnout) {
            progressWindow_->AppendLog("Parameter posterior statistics (after burnout):");

            for (int i = 0; i < numParams; ++i) {
                Parameter* param = gwaModel.Parameters()[i];
                if (param) {
                    // Calculate mean and std from samples (excluding burnout)
                    double sum = 0.0;
                    double sumSq = 0.0;
                    int count = 0;

                    for (size_t j = burnout; j < samples.size(); ++j) {
                        if (i < samples[j].size()) {
                            double val = samples[j][i];
                            sum += val;
                            sumSq += val * val;
                            count++;
                        }
                    }

                    if (count > 0) {
                        double mean = sum / count;
                        double variance = (sumSq / count) - (mean * mean);
                        double stddev = sqrt(variance);

                        progressWindow_->AppendLog(QString("  %1: mean = %2, std = %3")
                                                       .arg(QString::fromStdString(param->GetName()), -30)
                                                       .arg(mean, 0, 'e', 4)
                                                       .arg(stddev, 0, 'e', 4));
                    }
                }
            }
        }

        progressWindow_->SetComplete("MCMC Complete!");

        // Update status bar
        statusBar()->showMessage(
            QString("MCMC Complete: Acceptance Rate = %1% | Output: %2")
                .arg(acceptanceRate * 100.0, 0, 'f', 2)
                .arg(outputFolderName),
            10000
            );

        // Show results dialog
        QString resultMsg = QString("MCMC Sampling Complete!\n\n");
        resultMsg += QString("Acceptance Rate: %1%\n\n").arg(acceptanceRate * 100.0, 0, 'f', 2);
        resultMsg += QString("Generated %1 samples across %2 chains\n")
                         .arg(totalSamples)
                         .arg(numChains);
        resultMsg += QString("Estimated %1 parameters\n\n").arg(numParams);
        resultMsg += QString("Results saved to:\n%1\n\n").arg(outputFolderPath);
        resultMsg += "See progress window for detailed results and parameter statistics.";

        QMessageBox::information(this, "MCMC Complete", resultMsg);

    } catch (const std::exception& e) {
        if (progressWindow_) {
            progressWindow_->AppendLog(QString("ERROR: %1").arg(e.what()));
            progressWindow_->SetComplete("MCMC Failed!");
        }

        QMessageBox::critical(this, "MCMC Error",
                              QString("Error during MCMC:\n%1").arg(e.what()));
    }

    // Keep window open until user closes it
    if (progressWindow_) {
        progressWindow_->exec();
        delete progressWindow_;
        progressWindow_ = nullptr;
    }
}
