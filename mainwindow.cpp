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
    recentFilesMenu = new QMenu("Recent Projects", this);
    ui->actionRecent_Projects->setMenu(recentFilesMenu);
    updateRecentFilesMenu();

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

        updateAllWidgets();

    } else {
        QMessageBox::critical(
            this,
            tr("Error Loading File"),
            tr("Failed to load configuration file:\n%1").arg(fileName)
            );
    }
    addToRecentFiles(currentFilePath_);
}

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
    } else {
        QMessageBox::critical(
            this,
            tr("Error Saving File"),
            tr("Failed to save configuration file:\n%1").arg(currentFilePath_)
            );
    }
    addToRecentFiles(currentFilePath_);
}

void MainWindow::onSaveAsFile()
{
    // Always prompt for new filename
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save GWA Configuration File As"),
        currentFilePath_.isEmpty() ? QString() : currentFilePath_,
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
        currentFilePath_ = fileName;  // Update current file path
        statusBar()->showMessage(tr("Saved: %1").arg(currentFilePath_), 3000);
    } else {
        QMessageBox::critical(
            this,
            tr("Error Saving File"),
            tr("Failed to save configuration file:\n%1").arg(fileName)
            );
    }
    addToRecentFiles(currentFilePath_);
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

void MainWindow::onRecentFileTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString filePath = action->data().toString();
        if (QFile::exists(filePath)) {
            currentFilePath_ = filePath;
            // Load the file using your existing load logic
            gwaModel.loadFromFile(filePath.toStdString());
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

void MainWindow::onGASettingsTriggered()
{
    GASettingsDialog dialog(&ga, this);
    dialog.exec();
}
