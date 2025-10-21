#include "observationdialog.h"
#include "GWA.h"
#include "observation.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QFileInfo>
#include <limits>
#include "chartwindow.h"

ObservationDialog::ObservationDialog(CGWA* gwa, Observation* obs, QWidget* parent)
    : QDialog(parent)
    , gwa_(gwa)
    , obs_(obs)
{
    // Set window title based on whether editing or creating
    if (obs_) {
        setWindowTitle(tr("Edit Observation"));
    } else {
        setWindowTitle(tr("New Observation"));
    }

    setupUI();

    // Load data if editing existing observation
    if (obs_) {
        loadObservationData(obs_);
    }
}

void ObservationDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Basic Properties Group
    QGroupBox* basicGroup = new QGroupBox(tr("Basic Properties"), this);
    QFormLayout* basicLayout = new QFormLayout(basicGroup);

    // Name
    nameEdit_ = new QLineEdit(this);
    basicLayout->addRow(tr("Name:"), nameEdit_);

    // Well selection
    wellCombo_ = new QComboBox(this);
    wellCombo_->setEditable(false);
    wellCombo_->setToolTip(tr("Well or monitoring location"));
    basicLayout->addRow(tr("Well:"), wellCombo_);

    // Tracer selection
    tracerCombo_ = new QComboBox(this);
    tracerCombo_->setEditable(false);
    tracerCombo_->setToolTip(tr("Tracer or constituent being measured"));
    basicLayout->addRow(tr("Tracer:"), tracerCombo_);

    mainLayout->addWidget(basicGroup);

    // Populate well and tracer lists
    if (gwa_) {
        for (size_t i = 0; i < gwa_->getWellCount(); ++i) {
            wellCombo_->addItem(QString::fromStdString(gwa_->getWell(i).getName()));
        }

        for (size_t i = 0; i < gwa_->getTracerCount(); ++i) {
            tracerCombo_->addItem(QString::fromStdString(gwa_->getTracer(i).getName()));
        }
    }

    // Observed Data Group
    QGroupBox* dataGroup = new QGroupBox(tr("Observed Data"), this);
    QVBoxLayout* dataLayout = new QVBoxLayout(dataGroup);  // Changed to QVBoxLayout

    // File browser row
    QHBoxLayout* fileLayout = new QHBoxLayout();
    QLabel* fileLabel = new QLabel(tr("Data File:"), this);
    observedDataLabel_ = new QLabel(tr("<i>No file selected</i>"), this);
    observedDataLabel_->setWordWrap(true);
    observedDataLabel_->setStyleSheet("QLabel { padding: 5px; background-color: #f0f0f0; border: 1px solid #ccc; }");
    browseDataButton_ = new QPushButton(tr("Browse..."), this);
    connect(browseDataButton_, &QPushButton::clicked, this, &ObservationDialog::onBrowseObservedData);

    fileLayout->addWidget(fileLabel);
    fileLayout->addWidget(observedDataLabel_, 1);
    fileLayout->addWidget(browseDataButton_);
    dataLayout->addLayout(fileLayout);

    // Plot button row
    QPushButton* plotDataButton = new QPushButton(tr("Plot Observed Data"), this);
    plotDataButton->setToolTip(tr("Preview the observed data time series"));
    connect(plotDataButton, &QPushButton::clicked, this, &ObservationDialog::onPlotObservedData);

    QHBoxLayout* plotButtonLayout = new QHBoxLayout();
    plotButtonLayout->addStretch();
    plotButtonLayout->addWidget(plotDataButton);
    plotButtonLayout->addStretch();
    dataLayout->addLayout(plotButtonLayout);

    mainLayout->addWidget(dataGroup);
    // Error Structure Group
    QGroupBox* errorGroup = new QGroupBox(tr("Error Structure"), this);
    QFormLayout* errorLayout = new QFormLayout(errorGroup);

    // Standard deviation parameter
    stdParameterCombo_ = new QComboBox(this);
    stdParameterCombo_->setEditable(false);
    stdParameterCombo_->setToolTip(tr("Parameter representing measurement error standard deviation"));
    errorLayout->addRow(tr("Std Dev Parameter:"), stdParameterCombo_);

    // Error structure type
    errorStructureCombo_ = new QComboBox(this);
    errorStructureCombo_->addItems(QStringList() << "normal" << "log-normal");
    errorStructureCombo_->setToolTip(tr("Distribution of measurement errors"));
    errorLayout->addRow(tr("Error Structure:"), errorStructureCombo_);

    mainLayout->addWidget(errorGroup);

    // Update parameter list
    updateParameterList();

    // Detection Limit Group
    QGroupBox* detectionGroup = new QGroupBox(tr("Detection Limit"), this);
    QFormLayout* detectionLayout = new QFormLayout(detectionGroup);

    // Detection limit checkbox
    detectionLimitCheckBox_ = new QCheckBox(tr("Enable detection limit"), this);
    connect(detectionLimitCheckBox_, &QCheckBox::toggled,
            this, &ObservationDialog::onDetectionLimitToggled);
    detectionLayout->addRow(detectionLimitCheckBox_);

    // Detection limit value
    detectionLimitSpinBox_ = new QDoubleSpinBox(this);
    detectionLimitSpinBox_->setRange(0.0, std::numeric_limits<double>::max());
    detectionLimitSpinBox_->setDecimals(6);
    detectionLimitSpinBox_->setValue(0.0);
    detectionLimitSpinBox_->setEnabled(false);
    detectionLimitSpinBox_->setToolTip(tr("Minimum detectable value"));
    detectionLayout->addRow(tr("Detection Limit:"), detectionLimitSpinBox_);

    mainLayout->addWidget(detectionGroup);

    // Other Options Group
    QGroupBox* optionsGroup = new QGroupBox(tr("Options"), this);
    QFormLayout* optionsLayout = new QFormLayout(optionsGroup);

    // Count max checkbox
    countMaxCheckBox_ = new QCheckBox(tr("Normalize by maximum data count"), this);
    countMaxCheckBox_->setToolTip(tr("Normalize likelihood by max observations per well"));
    optionsLayout->addRow(countMaxCheckBox_);

    mainLayout->addWidget(optionsGroup);

    // Info label
    QLabel* infoLabel = new QLabel(
        tr("<i>Note: Observed data file should be a CSV with two columns: time and value.</i>"),
        this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { color: #666; padding: 10px; }");
    mainLayout->addWidget(infoLabel);

    // Dialog Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ObservationDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}

void ObservationDialog::loadObservationData(const Observation* obs)
{
    if (!obs) return;

    // Set name
    nameEdit_->setText(QString::fromStdString(obs->GetName()));

    // Set well
    QString wellName = QString::fromStdString(obs->GetLocation());
    int wellIndex = wellCombo_->findText(wellName);
    if (wellIndex >= 0) {
        wellCombo_->setCurrentIndex(wellIndex);
    }

    // Set tracer
    QString tracerName = QString::fromStdString(obs->GetQuantity());
    int tracerIndex = tracerCombo_->findText(tracerName);
    if (tracerIndex >= 0) {
        tracerCombo_->setCurrentIndex(tracerIndex);
    }

    // Set std parameter
    QString stdParam = QString::fromStdString(obs->GetStdParameterName());
    int stdIndex = stdParameterCombo_->findText(stdParam);
    if (stdIndex >= 0) {
        stdParameterCombo_->setCurrentIndex(stdIndex);
    }

    // Set error structure
    QString errorStructure = QString::fromStdString(obs->GetErrorStructure());
    int errorIndex = errorStructureCombo_->findText(errorStructure);
    if (errorIndex >= 0) {
        errorStructureCombo_->setCurrentIndex(errorIndex);
    }

    // Set observed data file path - show full path
    const TimeSeries<double>& obsData = obs->GetObservedData();
    if (!obsData.getFilename().empty()) {
        observedDataPath_ = QString::fromStdString(obsData.getFilename());
        observedDataLabel_->setText(observedDataPath_);  // Show full path
        observedDataLabel_->setToolTip(observedDataPath_);
    }

    // Set detection limit
    if (obs->HasDetectionLimit()) {
        detectionLimitCheckBox_->setChecked(true);
        detectionLimitSpinBox_->setValue(obs->GetDetectionLimitValue());
    }

    // Set count max
    countMaxCheckBox_->setChecked(obs->GetCountMax());
}

void ObservationDialog::updateParameterList()
{
    stdParameterCombo_->clear();

    if (gwa_) {
        for (size_t i = 0; i < gwa_->getParameterCount(); ++i) {
            const Parameter* param = gwa_->getParameter(i);
            if (param) {
                stdParameterCombo_->addItem(QString::fromStdString(param->GetName()));
            }
        }
    }
}

void ObservationDialog::onBrowseObservedData()
{
    // Get input path from GWA model if available
    QString startPath;
    if (gwa_ && !gwa_->InputPath().empty()) {
        startPath = QString::fromStdString(gwa_->InputPath());
    }

    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select Observed Data File"),
        startPath,
        tr("CSV Files (*.csv);;Text Files (*.txt);;All Files (*)")
        );

    if (!fileName.isEmpty()) {
        observedDataPath_ = fileName;
        // Show full path instead of just filename
        observedDataLabel_->setText(fileName);
        observedDataLabel_->setToolTip(fileName);  // Also set tooltip for easy copying
    }
}

void ObservationDialog::onDetectionLimitToggled(bool checked)
{
    detectionLimitSpinBox_->setEnabled(checked);
}

void ObservationDialog::onAccepted()
{
    // Validate name
    QString name = nameEdit_->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"),
                             tr("Observation name cannot be empty."));
        return;
    }

    // Validate well selection
    if (wellCombo_->currentIndex() < 0) {
        QMessageBox::warning(this, tr("Invalid Input"),
                             tr("Please select a well."));
        return;
    }

    // Validate tracer selection
    if (tracerCombo_->currentIndex() < 0) {
        QMessageBox::warning(this, tr("Invalid Input"),
                             tr("Please select a tracer."));
        return;
    }

    // Validate std parameter selection
    if (stdParameterCombo_->currentIndex() < 0) {
        QMessageBox::warning(this, tr("Invalid Input"),
                             tr("Please select a standard deviation parameter."));
        return;
    }

    // Validate observed data file
    if (observedDataPath_.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"),
                             tr("Please select an observed data file."));
        return;
    }

    // Check if file exists
    QFileInfo fileInfo(observedDataPath_);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, tr("File Not Found"),
                             tr("The observed data file does not exist:\n%1").arg(observedDataPath_));
        return;
    }

    accept();
}

Observation ObservationDialog::getObservation() const
{
    Observation obs;

    // Set name
    obs.SetName(nameEdit_->text().toStdString());

    // Set location (well)
    obs.SetLocation(wellCombo_->currentText().toStdString());

    // Set quantity (tracer)
    obs.SetQuantity(tracerCombo_->currentText().toStdString());

    // Set std parameter
    obs.SetStdParameterName(stdParameterCombo_->currentText().toStdString());

    // Set error structure
    obs.SetErrorStructure(errorStructureCombo_->currentText().toStdString());

    // Load observed data from file
    if (!observedDataPath_.isEmpty()) {
        TimeSeries<double> observedData(observedDataPath_.toStdString());
        obs.SetObservedTimeSeries(observedData);
    }

    // Set detection limit
    if (detectionLimitCheckBox_->isChecked()) {
        obs.SetHasDetectionLimit(true);
        obs.SetDetectionLimitValue(detectionLimitSpinBox_->value());
    } else {
        obs.SetHasDetectionLimit(false);
    }

    // Set count max
    obs.SetCountMax(countMaxCheckBox_->isChecked());

    return obs;
}

void ObservationDialog::onPlotObservedData()
{
    // Check if file is selected
    if (observedDataPath_.isEmpty()) {
        QMessageBox::warning(this, tr("No File Selected"),
                             tr("Please select an observed data file first."));
        return;
    }

    // Check if file exists
    QFileInfo fileInfo(observedDataPath_);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, tr("File Not Found"),
                             tr("The observed data file does not exist:\n%1").arg(observedDataPath_));
        return;
    }

    // Try to load the data
    try {
        TimeSeries<double> observedData(observedDataPath_.toStdString());

        if (observedData.size() == 0) {
            QMessageBox::warning(this, tr("Empty Data"),
                                 tr("The observed data file contains no data points."));
            return;
        }

        // Create TimeSeriesSet
        TimeSeriesSet<double> dataSet;
        observedData.setName("Observed Data");
        dataSet.append(observedData, "Observed Data");

        // Create chart title
        QString title = QString("Observed Data");
        if (!nameEdit_->text().isEmpty()) {
            title += QString(": %1").arg(nameEdit_->text());
        }
        if (wellCombo_->currentIndex() >= 0 && tracerCombo_->currentIndex() >= 0) {
            title += QString(" (%1 - %2)")
            .arg(wellCombo_->currentText())
                .arg(tracerCombo_->currentText());
        }

        // Show in chart window
        ChartWindow* chartWindow = ChartWindow::showChart(dataSet, title, this);
        chartWindow->setAxisLabels("Time", "Value");

        // Set plot mode to symbols (or lines+symbols for better visibility)
        // Use LinesAndSymbols if multiple points, Symbols if single point
        if (observedData.size() == 1) {
            chartWindow->chartViewer()->setPlotMode(ChartViewer::Symbols);
        } else {
            chartWindow->chartViewer()->setPlotMode(ChartViewer::LinesAndSymbols);
        }

    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Error Loading Data"),
                              tr("Failed to load observed data file:\n%1\n\nError: %2")
                                  .arg(observedDataPath_)
                                  .arg(e.what()));
    } catch (...) {
        QMessageBox::critical(this, tr("Error Loading Data"),
                              tr("Failed to load observed data file:\n%1")
                                  .arg(observedDataPath_));
    }
}
