#include "parameterdialog.h"
#include "chartwindow.h"
#include "GWA.h"
#include "parameter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>
#include <QPushButton>
#include <limits>
#include <cmath>

ParameterDialog::ParameterDialog(CGWA* gwa, Parameter* param, QWidget* parent)
    : QDialog(parent)
    , gwa_(gwa)
    , param_(param)
{
    // Set window title based on whether editing or creating
    if (param_) {
        setWindowTitle(tr("Edit Parameter"));
    } else {
        setWindowTitle(tr("New Parameter"));
    }

    setupUI();

    // Load data if editing existing parameter
    if (param_) {
        loadParameterData(param_);
    }
}

void ParameterDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Basic Properties Group
    QGroupBox* basicGroup = new QGroupBox(tr("Basic Properties"), this);
    QFormLayout* basicLayout = new QFormLayout(basicGroup);

    // Name
    nameEdit_ = new QLineEdit(this);
    basicLayout->addRow(tr("Name:"), nameEdit_);

    // Value
    valueSpinBox_ = new QDoubleSpinBox(this);
    valueSpinBox_->setRange(-std::numeric_limits<double>::max(),
                            std::numeric_limits<double>::max());
    valueSpinBox_->setDecimals(6);
    valueSpinBox_->setValue(0.0);
    valueSpinBox_->setToolTip(tr("Current parameter value"));
    basicLayout->addRow(tr("Value:"), valueSpinBox_);

    mainLayout->addWidget(basicGroup);

    // Range Group
    QGroupBox* rangeGroup = new QGroupBox(tr("Valid Range"), this);
    QFormLayout* rangeLayout = new QFormLayout(rangeGroup);

    // Low bound
    lowSpinBox_ = new QDoubleSpinBox(this);
    lowSpinBox_->setRange(-std::numeric_limits<double>::max(),
                          std::numeric_limits<double>::max());
    lowSpinBox_->setDecimals(6);
    lowSpinBox_->setValue(0.0);
    lowSpinBox_->setToolTip(tr("Lower bound of valid range (2.5th percentile)"));
    rangeLayout->addRow(tr("Low:"), lowSpinBox_);

    // High bound
    highSpinBox_ = new QDoubleSpinBox(this);
    highSpinBox_->setRange(-std::numeric_limits<double>::max(),
                           std::numeric_limits<double>::max());
    highSpinBox_->setDecimals(6);
    highSpinBox_->setValue(1.0);
    highSpinBox_->setToolTip(tr("Upper bound of valid range (97.5th percentile)"));
    rangeLayout->addRow(tr("High:"), highSpinBox_);

    mainLayout->addWidget(rangeGroup);

    // Prior Distribution Group
    priorParamsGroup_ = new QGroupBox(tr("Prior Distribution"), this);
    QFormLayout* priorLayout = new QFormLayout(priorParamsGroup_);

    // Distribution type
    priorDistributionCombo_ = new QComboBox(this);
    priorDistributionCombo_->addItems(QStringList() << "uniform" << "normal" << "log-normal");
    priorDistributionCombo_->setToolTip(tr("Prior probability distribution for Bayesian inference"));
    connect(priorDistributionCombo_, &QComboBox::currentTextChanged,
            this, &ParameterDialog::onPriorDistributionChanged);
    priorLayout->addRow(tr("Distribution:"), priorDistributionCombo_);

    // REMOVED: Prior mean and std dev spin boxes - these are calculated automatically

    // Add info label explaining automatic calculation
    QLabel* priorInfoLabel = new QLabel(
        tr("<i>Mean and standard deviation are automatically calculated from the range:<br>"
           "• Normal: mean = (low + high)/2, σ = (high - low)/4<br>"
           "• Log-normal: mean = √(low × high), σ = (ln(high) - ln(low))/4</i>"), this);
    priorInfoLabel->setWordWrap(true);
    priorInfoLabel->setStyleSheet("QLabel { color: #666; font-style: italic; padding: 5px; }");
    priorLayout->addRow(priorInfoLabel);

    // Add plot button
    plotPriorButton_ = new QPushButton(this);
    QIcon priorIcon = QIcon::fromTheme("office-chart-area", QIcon(":/icons/distribution.png"));
    if (!priorIcon.isNull()) {
        plotPriorButton_->setIcon(priorIcon);
    }
    plotPriorButton_->setToolTip(tr("Plot Prior Distribution"));  // Tooltip is important now!
    plotPriorButton_->setIconSize(QSize(48, 48));  // Optional: adjust icon size
    connect(plotPriorButton_, &QPushButton::clicked, this, &ParameterDialog::onPlotPriorDistribution);

    QHBoxLayout* plotButtonLayout = new QHBoxLayout();
    plotButtonLayout->addStretch();
    plotButtonLayout->addWidget(plotPriorButton_);
    plotButtonLayout->addStretch();
    priorLayout->addRow(plotButtonLayout);

    mainLayout->addWidget(priorParamsGroup_);

    // Info label
    QLabel* infoLabel = new QLabel(
        tr("<i>Note: Parameters can be linked to well/tracer properties through the "
           "respective dialogs.</i>"), this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { color: #666; padding: 10px; }");
    mainLayout->addWidget(infoLabel);

    mcmcResultsGroup_ = new QGroupBox(tr("MCMC Results"), this);
    QVBoxLayout* mcmcLayout = new QVBoxLayout(mcmcResultsGroup_);

    // MCMC Chains button
    plotMCMCChainsButton_ = new QPushButton(this);
    QIcon chainsIcon = QIcon::fromTheme("office-chart-line", QIcon(":/icons/chains.png"));
    if (!chainsIcon.isNull()) {
        plotMCMCChainsButton_->setIcon(chainsIcon);
    }
    plotMCMCChainsButton_->setToolTip(tr("Plot MCMC Chain Samples"));
    plotMCMCChainsButton_->setIconSize(QSize(48, 48));
    connect(plotMCMCChainsButton_, &QPushButton::clicked,
            this, &ParameterDialog::onPlotMCMCChains);

    // Posterior Distribution button
    plotPosteriorButton_ = new QPushButton(this);
    QIcon posteriorIcon = QIcon::fromTheme("office-chart-area", QIcon(":/icons/posterior.png"));
    if (!posteriorIcon.isNull()) {
        plotPosteriorButton_->setIcon(posteriorIcon);
    }
    plotPosteriorButton_->setToolTip(tr("Plot Posterior Distribution"));
    plotPosteriorButton_->setIconSize(QSize(48, 48));
    connect(plotPosteriorButton_, &QPushButton::clicked,
            this, &ParameterDialog::onPlotPosteriorDistribution);

    // Layout for buttons
    QHBoxLayout* mcmcButtonsLayout = new QHBoxLayout();
    mcmcButtonsLayout->addStretch();
    mcmcButtonsLayout->addWidget(plotMCMCChainsButton_);
    mcmcButtonsLayout->addWidget(plotPosteriorButton_);
    mcmcButtonsLayout->addStretch();
    mcmcLayout->addLayout(mcmcButtonsLayout);

    // Info label
    QLabel* mcmcInfoLabel = new QLabel(
        tr("<i>MCMC results are available after running Markov Chain Monte Carlo sampling.</i>"),
        this);
    mcmcInfoLabel->setWordWrap(true);
    mcmcInfoLabel->setStyleSheet("QLabel { color: #666; font-style: italic; padding: 5px; }");
    mcmcLayout->addWidget(mcmcInfoLabel);

    mainLayout->addWidget(mcmcResultsGroup_);

    // Initially hide MCMC results group if no data
    updateMCMCButtonsVisibility();

    // Dialog Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ParameterDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}

void ParameterDialog::loadParameterData(const Parameter* param)
{
    if (!param) return;

    // Set name
    nameEdit_->setText(QString::fromStdString(param->GetName()));

    // Set value
    valueSpinBox_->setValue(param->GetValue());

    // Set range
    Parameter::Range range = param->GetRange();
    lowSpinBox_->setValue(range.low);
    highSpinBox_->setValue(range.high);

    // Set prior distribution
    QString priorDist = QString::fromStdString(param->GetPriorDistribution());
    int index = priorDistributionCombo_->findText(priorDist, Qt::MatchFixedString);
    if (index >= 0) {
        priorDistributionCombo_->setCurrentIndex(index);
    }

    // Note: Mean and std dev are calculated automatically, no need to load them
}

void ParameterDialog::onPriorDistributionChanged(const QString& dist)
{
    // Nothing special to do - mean and std are calculated on-the-fly
    // This slot can be removed or used for other purposes
}

void ParameterDialog::onPlotPriorDistribution()
{
    QString distType = priorDistributionCombo_->currentText();
    double low = lowSpinBox_->value();
    double high = highSpinBox_->value();

    // Validate range
    if (low >= high) {
        QMessageBox::warning(this, tr("Invalid Range"),
                             tr("Lower bound must be less than upper bound."));
        return;
    }

    // Calculate mean and std dev based on distribution type
    double mean, stdDev;

    if (distType == "uniform") {
        // No mean/std needed for uniform
        mean = 0.0;
        stdDev = 0.0;
    }
    else if (distType == "normal") {
        // Mean = arithmetic mean of bounds
        // Std dev assumes bounds are ±2σ (95% CI)
        mean = (low + high) / 2.0;
        stdDev = (high - low) / 4.0;
    }
    else if (distType == "log-normal") {
        // Mean = geometric mean of bounds
        // Std dev assumes bounds are at ±2σ on log scale
        if (low <= 0.0 || high <= 0.0) {
            QMessageBox::warning(this, tr("Invalid Range"),
                                 tr("Log-normal distribution requires positive bounds."));
            return;
        }
        mean = std::sqrt(low * high);  // Geometric mean
        stdDev = (log(high) - log(low)) / 4.0;
    }
    else {
        return;  // Unknown distribution
    }

    // Create distribution
    int numPoints = 1000;
    double range = high - low;
    double padding = range * 0.1;  // 10% padding on each side
    double xMin = low - padding;
    double xMax = high + padding;
    double dx = (xMax - xMin) / numPoints;

    TimeSeries<double> distribution;
    distribution.setName("Prior Distribution");

    if (distType == "uniform") {
        // Uniform distribution: constant within range, zero outside
        for (int i = 0; i <= numPoints; ++i) {
            double x = xMin + i * dx;
            double y = 0.0;

            if (x >= low && x <= high) {
                y = 1.0 / (high - low);  // Constant probability density
            }

            distribution.append(x, y);
        }
    }
    else if (distType == "normal") {
        // Normal distribution
        double pi = 3.14159265358979323846;

        for (int i = 0; i <= numPoints; ++i) {
            double x = xMin + i * dx;
            double z = (x - mean) / stdDev;
            double y = (1.0 / (stdDev * std::sqrt(2.0 * pi))) *
                       std::exp(-0.5 * z * z);

            distribution.append(x, y);
        }
    }
    else if (distType == "log-normal") {
        // Log-normal distribution
        // For log-normal, need positive x values
        if (xMin <= 0) {
            xMin = 0.001;  // Small positive value
        }
        dx = (xMax - xMin) / numPoints;

        double pi = 3.14159265358979323846;

        for (int i = 0; i <= numPoints; ++i) {
            double x = xMin + i * dx;

            if (x > 0) {
                double z = (std::log(x) - std::log(mean)) / stdDev;
                double y = (1.0 / (x * stdDev * std::sqrt(2.0 * pi))) *
                           std::exp(-0.5 * z * z);

                distribution.append(x, y);
            } else {
                distribution.append(x, 0.0);
            }
        }
    }

    // Create TimeSeriesSet with the distribution
    TimeSeriesSet<double> dataSet;
    dataSet.append(distribution, "Prior Distribution");

    // Show in chart window
    QString title = QString("Prior Distribution: %1 (%2)")
                        .arg(nameEdit_->text().isEmpty() ? "Parameter" : nameEdit_->text())
                        .arg(distType);

    if (distType != "uniform") {
        title += QString("\nμ = %1, σ = %2")
                     .arg(mean, 0, 'f', 4)
                     .arg(stdDev, 0, 'f', 4);
    }

    ChartWindow* chartWindow = ChartWindow::showChart(dataSet, title, this);
    chartWindow->chartViewer()->setYAxisStartAtZero(true);
    chartWindow->chartViewer()->setPlotMode(ChartViewer::Filled);
    chartWindow->setAxisLabels("Parameter Value", "Probability Density");
}

void ParameterDialog::onAccepted()
{
    // Validate name
    QString name = nameEdit_->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"),
                             tr("Parameter name cannot be empty."));
        return;
    }

    // Check for duplicate names (only if creating new or name changed)
    if (gwa_) {
        bool nameChanged = !param_ ||
                           (QString::fromStdString(param_->GetName()) != name);

        if (nameChanged) {
            int existingIdx = gwa_->findParameter(name.toStdString());
            if (existingIdx >= 0) {
                QMessageBox::warning(this, tr("Duplicate Name"),
                                     tr("A parameter with name '%1' already exists.").arg(name));
                return;
            }
        }
    }

    // Validate range
    double low = lowSpinBox_->value();
    double high = highSpinBox_->value();
    if (low >= high) {
        QMessageBox::warning(this, tr("Invalid Range"),
                             tr("Lower bound must be less than upper bound."));
        return;
    }

    // For log-normal, ensure positive bounds
    QString dist = priorDistributionCombo_->currentText();
    if (dist == "log-normal" && (low <= 0.0 || high <= 0.0)) {
        QMessageBox::warning(this, tr("Invalid Range"),
                             tr("Log-normal distribution requires positive bounds."));
        return;
    }

    // Validate value is within range
    double value = valueSpinBox_->value();
    if (value < low || value > high) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Value Out of Range"),
            tr("Current value (%1) is outside the valid range [%2, %3]. Continue anyway?")
                .arg(value).arg(low).arg(high),
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::No) {
            return;
        }
    }

    accept();
}

Parameter ParameterDialog::getParameter() const
{
    double low = lowSpinBox_->value();
    double high = highSpinBox_->value();
    QString dist = priorDistributionCombo_->currentText();

    // Calculate mean and std dev automatically
    double mean, stdDev;

    if (dist == "normal") {
        mean = (low + high) / 2.0;
        stdDev = (high - low) / 4.0;
    }
    else if (dist == "log-normal") {
        mean = std::sqrt(low * high);  // Geometric mean
        stdDev = (std::log(high) - std::log(low)) / 4.0;
    }
    else {
        // Uniform - these values aren't used but set them anyway
        mean = (low + high) / 2.0;
        stdDev = (high - low) / 4.0;
    }

    // Create parameter with basic properties
    Parameter param(
        nameEdit_->text().toStdString(),
        low,
        high,
        dist.toStdString(),
        valueSpinBox_->value()
        );

    // Set calculated prior parameters
    param.SetPriorParameters(mean, stdDev);

    // If editing existing parameter, preserve location linkages
    if (param_) {
        param.SetLocations(param_->GetLocations());
        param.SetQuantities(param_->GetQuantities());
        param.SetLocationTypes(param_->GetLocationTypes());
    }

    return param;
}

void ParameterDialog::updateMCMCButtonsVisibility()
{
    // Only show MCMC results group if editing existing parameter
    if (!param_) {
        mcmcResultsGroup_->setVisible(false);
        return;
    }

    // Always show the group for existing parameters
    mcmcResultsGroup_->setVisible(true);

    // Enable/disable individual buttons based on available data
    bool hasMCMCChains = param_->hasMCMCSamples();
    bool hasPosterior = param_->hasPosteriorDistribution();

    plotMCMCChainsButton_->setEnabled(hasMCMCChains);
    plotPosteriorButton_->setEnabled(hasPosterior);

    // Update button text to indicate availability
    if (hasMCMCChains) {
        plotMCMCChainsButton_->setText(tr("Plot MCMC Chains"));
    } else {
        plotMCMCChainsButton_->setText(tr("Plot MCMC Chains (No Data)"));
    }

    if (hasPosterior) {
        plotPosteriorButton_->setText(tr("Plot Posterior"));
    } else {
        plotPosteriorButton_->setText(tr("Plot Posterior (No Data)"));
    }
}

void ParameterDialog::onPlotMCMCChains()
{
    if (!param_) {
        QMessageBox::information(this, tr("No Data"),
                                 tr("No MCMC chain data available."));
        return;
    }

    // Get MCMC samples from parameter
    // Assuming Parameter has a getter for mcmcSamples
    const TimeSeriesSet<double>& mcmcSamples = param_->GetMCMCSamples();

    if (mcmcSamples.size() == 0) {
        QMessageBox::information(this, tr("No Data"),
                                 tr("No MCMC chain samples available for this parameter."));
        return;
    }

    // Create chart title
    QString title = QString("MCMC Chains: %1")
                        .arg(QString::fromStdString(param_->GetName()));

    // Show in chart window
    ChartWindow* chartWindow = ChartWindow::showChart(mcmcSamples, title, this);
    chartWindow->setAxisLabels("Sample Number", "Parameter Value");
}

void ParameterDialog::onPlotPosteriorDistribution()
{
    if (!param_) {
        QMessageBox::information(this, tr("No Data"),
                                 tr("No posterior distribution data available."));
        return;
    }

    // Get posterior distribution from parameter
    // Assuming Parameter has a getter for posteriorDist
    const TimeSeries<double>& posteriorDist = param_->GetPosteriorDistribution();

    if (posteriorDist.size() == 0) {
        QMessageBox::information(this, tr("No Data"),
                                 tr("No posterior distribution available for this parameter."));
        return;
    }

    // Create TimeSeriesSet with the distribution
    TimeSeriesSet<double> dataSet;
    dataSet.append(posteriorDist, "Posterior Distribution");

    // Create chart title
    QString title = QString("Posterior Distribution: %1")
                        .arg(QString::fromStdString(param_->GetName()));

    // Show in chart window
    ChartWindow* chartWindow = ChartWindow::showChart(dataSet, title, this);
    chartWindow->chartViewer()->setYAxisStartAtZero(true);
    chartWindow->chartViewer()->setPlotMode(ChartViewer::Filled);
    chartWindow->setAxisLabels("Parameter Value", "Probability Density");
}
