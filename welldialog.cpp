#include "welldialog.h"
#include "parametervaluewidget.h"
#include "GWA.h"
#include "Well.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <chartwindow.h>

WellDialog::WellDialog(CGWA* gwa, CWell* well, QWidget* parent)
    : QDialog(parent)
    , gwa_(gwa)
    , well_(well)
{
    // Set window title based on whether editing or creating
    if (well_) {
        setWindowTitle(tr("Edit Well"));
    } else {
        setWindowTitle(tr("New Well"));
    }

    // Distribution types
    distributionTypes << "Piston" << "Exponential" << "Gamma" << "Piston+Exponential"
                      << "Log-normal" << "Inverse-Gaussian" << "Histogram" << "Shifted Exponential";

    setupUI();

    // Load data if editing existing well
    if (well_) {
        loadWellData(well_);

        // Enable MCMC plot buttons if data exists
        if (well_->GetRealizations().size() > 0) {
            plotRealizationsButton_->setEnabled(true);
        }
        if (well_->GetPercentile95().size() > 0) {
            plotPercentilesButton_->setEnabled(true);
        }
    }
    else {
        onDistributionTypeChanged(distributionTypeCombo->currentText());
    }
}
void WellDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Basic Properties Group
    QGroupBox* basicGroup = new QGroupBox(tr("Basic Properties"), this);
    QFormLayout* basicLayout = new QFormLayout(basicGroup);

    nameEdit = new QLineEdit(this);
    basicLayout->addRow(tr("Name:"), nameEdit);

    distributionTypeCombo = new QComboBox(this);
    distributionTypeCombo->addItems(distributionTypes);
    distributionTypeCombo->setEditable(true);
    connect(distributionTypeCombo, &QComboBox::currentTextChanged,
            this, &WellDialog::onDistributionTypeChanged);
    basicLayout->addRow(tr("Distribution Type:"), distributionTypeCombo);

    mainLayout->addWidget(basicGroup);

    // Distribution Parameters Group
    distributionParamsGroup = new QGroupBox(tr("Distribution Parameters"), this);
    distributionParamsLayout = new QFormLayout(distributionParamsGroup);

    // Add info label at the top
    distributionInfoLabel = new QLabel();
    distributionInfoLabel->setWordWrap(true);
    distributionInfoLabel->setStyleSheet("QLabel { color: #666; font-style: italic; padding: 5px; }");
    distributionParamsLayout->addRow(distributionInfoLabel);

    // Parameter widgets will be added here dynamically

    mainLayout->addWidget(distributionParamsGroup);

    // Add plot button AFTER the distribution group (in main layout, not form layout)
    plotDistributionButton_ = new QPushButton("Plot Distribution", this);
    plotDistributionButton_->setToolTip(tr("Preview age distribution with current parameters"));
    connect(plotDistributionButton_, &QPushButton::clicked, this, &WellDialog::onPlotDistribution);

    QHBoxLayout* plotButtonLayout = new QHBoxLayout();
    plotButtonLayout->addStretch();
    plotButtonLayout->addWidget(plotDistributionButton_);
    plotButtonLayout->addStretch();

    mainLayout->addLayout(plotButtonLayout);

    // Mixing Parameters Group
    QGroupBox* mixingGroup = new QGroupBox(tr("Mixing Parameters"), this);
    QFormLayout* mixingLayout = new QFormLayout(mixingGroup);

    fractionOldWidget = new ParameterValueWidget(this);
    fractionOldWidget->setPlaceholderText("0.0 - 1.0");
    fractionOldWidget->setToolTip(tr("Fraction of old water (0-1)"));
    mixingLayout->addRow(tr("Fraction Old (f):"), fractionOldWidget);

    ageOldWidget = new ParameterValueWidget(this);
    ageOldWidget->setPlaceholderText("Age in years");
    ageOldWidget->setToolTip(tr("Age of old water component"));
    mixingLayout->addRow(tr("Age Old:"), ageOldWidget);

    fractionMineralWidget = new ParameterValueWidget(this);
    fractionMineralWidget->setPlaceholderText("0.0 - 1.0");
    fractionMineralWidget->setToolTip(tr("Fraction mineral (fm) for mineral correction"));
    mixingLayout->addRow(tr("Fraction Mineral (fm):"), fractionMineralWidget);

    vzDelayWidget = new ParameterValueWidget(this);
    vzDelayWidget->setPlaceholderText("Delay in years");
    vzDelayWidget->setToolTip(tr("Vadose zone delay time"));
    mixingLayout->addRow(tr("Vadose Zone Delay:"), vzDelayWidget);

    mainLayout->addWidget(mixingGroup);

    // MCMC Results Group
    QGroupBox* mcmcGroup = new QGroupBox(tr("MCMC Results"), this);
    QHBoxLayout* mcmcLayout = new QHBoxLayout(mcmcGroup);

    plotRealizationsButton_ = new QPushButton(tr("Plot Realizations"), this);
    plotRealizationsButton_->setToolTip(tr("Plot ensemble realizations from MCMC uncertainty analysis"));
    plotRealizationsButton_->setEnabled(false);  // Will be enabled if data exists
    connect(plotRealizationsButton_, &QPushButton::clicked, this, &WellDialog::onPlotRealizations);

    plotPercentilesButton_ = new QPushButton(tr("Plot Percentiles"), this);
    plotPercentilesButton_->setToolTip(tr("Plot 95% prediction intervals from MCMC analysis"));
    plotPercentilesButton_->setEnabled(false);  // Will be enabled if data exists
    connect(plotPercentilesButton_, &QPushButton::clicked, this, &WellDialog::onPlotPercentiles);

    mcmcLayout->addWidget(plotRealizationsButton_);
    mcmcLayout->addWidget(plotPercentilesButton_);

    mainLayout->addWidget(mcmcGroup);

    // Dialog Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &WellDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}

void WellDialog::loadWellData(const CWell* well)
{
    if (!well) return;

    std::string wellName = well->getName();

    // Set name
    nameEdit->setText(QString::fromStdString(wellName));

    // Set distribution type
    QString distType = QString::fromStdString(well->getDistributionType());

    // IMPORTANT: Always call onDistributionTypeChanged explicitly
    // because setting the combo box might not trigger the signal
    onDistributionTypeChanged(distType);

    // Then set the combo box
    int index = distributionTypeCombo->findText(distType, Qt::MatchFixedString);
    if (index >= 0) {
        distributionTypeCombo->setCurrentIndex(index);
    } else {
        distributionTypeCombo->setCurrentText(distType);
    }

    // Set distribution parameters with parameter linkage check
    const std::vector<double>& params = well->getParameters();
    for (int i = 0; i < distributionParamWidgets.size(); ++i) {
        QString paramName = QString("param[%1]").arg(i);
        QString linkedParam = findLinkedParameter(wellName, paramName.toStdString());

        if (!linkedParam.isEmpty()) {
            distributionParamWidgets[i]->setParameter(linkedParam);
        } else if (i < static_cast<int>(params.size())) {
            distributionParamWidgets[i]->setValue(params[i]);
        }
    }

    // Set mixing parameters with parameter linkage check
    QString linkedF = findLinkedParameter(wellName, "f");
    if (!linkedF.isEmpty()) {
        fractionOldWidget->setParameter(linkedF);
    } else {
        fractionOldWidget->setValue(well->getFractionOld());
    }

    QString linkedAgeOld = findLinkedParameter(wellName, "age_old");
    if (!linkedAgeOld.isEmpty()) {
        ageOldWidget->setParameter(linkedAgeOld);
    } else {
        ageOldWidget->setValue(well->getAgeOld());
    }

    QString linkedFm = findLinkedParameter(wellName, "fm");
    if (!linkedFm.isEmpty()) {
        fractionMineralWidget->setParameter(linkedFm);
    } else {
        fractionMineralWidget->setValue(well->getFractionMineral());
    }

    QString linkedVzDelay = findLinkedParameter(wellName, "vz_delay");
    if (!linkedVzDelay.isEmpty()) {
        vzDelayWidget->setParameter(linkedVzDelay);
    } else {
        vzDelayWidget->setValue(well->getVzDelay());
    }
}

void WellDialog::updateParameterWidgets()
{
    if (!gwa_) return;

    // Get parameter names from GWA
    QVector<QString> paramNames;
    for (size_t i = 0; i < gwa_->getParameterCount(); ++i) {
        const Parameter* param = gwa_->getParameter(i);
        if (param) {
            paramNames.append(QString::fromStdString(param->GetName()));
        }
    }

    // Update all parameter widgets
    for (ParameterValueWidget* widget : distributionParamWidgets) {
        widget->setAvailableParameters(paramNames);
    }

    fractionOldWidget->setAvailableParameters(paramNames);
    ageOldWidget->setAvailableParameters(paramNames);
    fractionMineralWidget->setAvailableParameters(paramNames);
    vzDelayWidget->setAvailableParameters(paramNames);

}

void WellDialog::createDistributionParamWidgets(int count)
{
    // Clear existing parameter widgets (but keep the info label)
    // Remove rows from bottom up, starting after the info label (row 0)
    while (distributionParamsLayout->rowCount() > 1) {
        distributionParamsLayout->removeRow(1);
    }
    distributionParamWidgets.clear();

    // Create new widgets
    for (int i = 0; i < count; ++i) {
        ParameterValueWidget* widget = new ParameterValueWidget(this);
        widget->setPlaceholderText("Enter value");
        distributionParamWidgets.append(widget);

        QString label = tr("Parameter %1:").arg(i + 1);
        distributionParamsLayout->addRow(label, widget);
    }

    // Update parameter lists
    updateParameterWidgets();
}

void WellDialog::onDistributionTypeChanged(const QString& type)
{
    // Determine parameter count for this distribution
    QString lowerType = type.toLower();
    int paramCount = 0;
    QString infoText;

    if (lowerType == "piston") {
        paramCount = 1;
        infoText = tr("Parameter 1: Mean age");
    } else if (lowerType == "exponential") {
        paramCount = 1;
        infoText = tr("Parameter 1: Mean residence time (1/λ)");
    } else if (lowerType == "piston+exponential") {
        paramCount = 2;
        infoText = tr("Parameter 1: Exponential ratio, Parameter 2: Piston flow time");
    } else if (lowerType == "gamma") {
        paramCount = 2;
        infoText = tr("Parameter 1: Shape (k), Parameter 2: Scale (θ)");
    } else if (lowerType == "log-normal") {
        paramCount = 2;
        infoText = tr("Parameter 1: Mean (μ), Parameter 2: Standard deviation (σ)");
    } else if (lowerType == "inverse-gaussian") {
        paramCount = 2;
        infoText = tr("Parameter 1: Mean (μ), Parameter 2: Shape (λ)");
    } else if (lowerType == "dispersion") {
        paramCount = 2;
        infoText = tr("Parameter 1: Mean time, Parameter 2: Dispersion parameter");
    } else if (lowerType == "histogram") {
        paramCount = 10;
        infoText = tr("Parameters define the probability for each time bin");
    }
    // Update info label
    distributionInfoLabel->setText(infoText);

    createDistributionParamWidgets(paramCount);
}

void WellDialog::onAccepted()
{
    // Validate name
    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"),
                             tr("Well name cannot be empty."));
        return;
    }

    accept();
}

CWell WellDialog::getWell() const
{
    CWell well;

    // Set name
    well.setName(nameEdit->text().toStdString());

    // Set distribution type
    well.setDistributionType(distributionTypeCombo->currentText().toStdString());

    // Set distribution parameters
    std::vector<double> params;
    for (ParameterValueWidget* widget : distributionParamWidgets) {
        if (widget->isParameterMode()) {
            // For now, just use 0.0 as placeholder
            // TODO: Implement parameter linkage properly
            params.push_back(0.0);
        } else {
            params.push_back(widget->getValue());
        }
    }

    for (size_t i = 0; i < params.size(); ++i) {
        well.setParameter("param[" + std::to_string(i) + "]", params[i]);
    }

    // Set mixing parameters
    if (fractionOldWidget->isParameterMode()) {
        // TODO: Handle parameter linkage
        well.setFractionOld(0.0);
    } else {
        well.setFractionOld(fractionOldWidget->getValue());
    }

    if (ageOldWidget->isParameterMode()) {
        // TODO: Handle parameter linkage
        well.setAgeOld(0.0);
    } else {
        well.setAgeOld(ageOldWidget->getValue());
    }

    if (fractionMineralWidget->isParameterMode()) {
        // TODO: Handle parameter linkage
        well.setFractionModern(0.0);
    } else {
        well.setFractionModern(fractionMineralWidget->getValue());
    }

    if (vzDelayWidget->isParameterMode()) {
        // TODO: Handle parameter linkage
        well.setVzDelay(0.0);
    } else {
        well.setVzDelay(vzDelayWidget->getValue());
    }

    return well;
}

QString WellDialog::findLinkedParameter(const std::string& wellName, const std::string& quantity)
{
    if (!gwa_) return QString();

    // Search through all parameters to find one that links to this well property
    for (size_t i = 0; i < gwa_->getParameterCount(); ++i) {
        const Parameter* param = gwa_->getParameter(i);
        if (!param) continue;

        const std::vector<std::string>& locations = param->GetLocations();
        const std::vector<std::string>& quantities = param->GetQuantities();
        const std::vector<std::string>& locationTypes = param->GetLocationTypes();

        // Check each location this parameter is applied to
        for (size_t j = 0; j < locations.size(); ++j) {
            // Check if it's a well location (location_type == "well" or "0")
            bool isWell = (locationTypes[j] == "well" || locationTypes[j] == "0");

            // Check if it matches our well and property
            if (isWell &&
                locations[j] == wellName &&
                quantities[j] == quantity) {
                return QString::fromStdString(param->GetName());
            }
        }
    }

    return QString();  // No parameter found
}

QMap<QString, QString> WellDialog::getParameterLinkages() const
{
    QMap<QString, QString> linkages;

    // Check mixing parameters
    if (fractionOldWidget->isParameterMode()) {
        linkages["f"] = fractionOldWidget->getParameter();
    }
    if (ageOldWidget->isParameterMode()) {
        linkages["age_old"] = ageOldWidget->getParameter();
    }
    if (fractionMineralWidget->isParameterMode()) {
        linkages["fm"] = fractionMineralWidget->getParameter();
    }
    if (vzDelayWidget->isParameterMode()) {
        linkages["vz_delay"] = vzDelayWidget->getParameter();
    }

    // Check distribution parameters
    for (int i = 0; i < distributionParamWidgets.size(); ++i) {
        if (distributionParamWidgets[i]->isParameterMode()) {
            QString key = QString("param[%1]").arg(i);
            linkages[key] = distributionParamWidgets[i]->getParameter();
        }
    }

    return linkages;
}

void WellDialog::onPlotDistribution()
{
    // Get current parameters from widgets
    std::vector<double> params;
    for (ParameterValueWidget* widget : distributionParamWidgets) {
        if (widget->isParameterMode()) {
            // If linked to parameter, get its value
            if (gwa_) {
                int paramIdx = gwa_->findParameter(widget->getParameter().toStdString());
                if (paramIdx >= 0) {
                    const Parameter* param = gwa_->getParameter(paramIdx);
                    if (param) {
                        params.push_back(param->GetValue());
                    } else {
                        params.push_back(0.0);
                    }
                } else {
                    params.push_back(0.0);
                }
            } else {
                params.push_back(0.0);
            }
        } else {
            params.push_back(widget->getValue());
        }
    }

    // Get distribution type
    QString distType = distributionTypeCombo->currentText();

    // Create age distribution
    double max_age = 100.0;  // You can make this configurable
    int num_intervals = 1000;
    double multiplier = 0.02;

    TimeSeries<double> distribution;
    std::string lowerType = distType.toLower().toStdString();

    // Call appropriate distribution creation function
    if (lowerType == "piston") {
        distribution = CWell::createDiracDistribution(params, max_age, num_intervals, multiplier);
    } else if (lowerType == "exponential") {
        distribution = CWell::createExponentialDistribution(params, max_age, num_intervals, multiplier);
    } else if (lowerType == "gamma") {
        distribution = CWell::createGammaDistribution(params, max_age, num_intervals, multiplier);
    } else if (lowerType == "log-normal") {
        distribution = CWell::createLogNormalDistribution(params, max_age, num_intervals, multiplier);
    } else if (lowerType == "inverse-gaussian") {
        distribution = CWell::createInverseGaussianDistribution(params, max_age, num_intervals, multiplier);
    } else if (lowerType == "piston+exponential") {
        distribution = CWell::createShiftedExponentialDistribution(params, max_age, num_intervals, multiplier);
    } else if (lowerType == "dispersion") {
        distribution = CWell::createDispersionDistribution(params, max_age, num_intervals, multiplier);
    } else if (lowerType == "histogram") {
        // For histogram, need bin count and size
        int bin_count = 10;  // Default
        double bin_size = max_age / bin_count;
        distribution = CWell::createHistogramDistribution(params, bin_count, bin_size,
                                                          max_age, num_intervals, multiplier);
    } else {
        QMessageBox::warning(this, tr("Unknown Distribution"),
                             tr("Cannot plot distribution type: %1").arg(distType));
        return;
    }

    // Create TimeSeriesSet with the distribution
    TimeSeriesSet<double> dataSet;

    dataSet.append(distribution, "Age Distribution");

    // Show in chart window
    ChartWindow* chartWindow = ChartWindow::showChart(
        dataSet,
        QString("Age Distribution: %1").arg(distType),
        this
        );

    chartWindow->setAxisLabels("Age (years)", "Probability Density");
}

void WellDialog::onPlotRealizations()
{
    if (!well_ || well_->GetRealizations().size() == 0) {
        QMessageBox::warning(this, tr("No Data"),
            tr("No realizations available to plot."));
        return;
    }

    // Create chart window
    ChartWindow* window = new ChartWindow(this);
    window->setWindowTitle(tr("Age Distribution Realizations - %1")
        .arg(QString::fromStdString(well_->getName())));
    window->setChartTitle(tr("MCMC Realizations - %1")
        .arg(QString::fromStdString(well_->getName())));
    window->setAxisLabels("Age (years)", "Probability Density");

    // Get realizations
    const TimeSeriesSet<double>& realizations = well_->GetRealizations();

    // Create plot data
    TimeSeriesSet<double> plotData;

    // Add all realizations
    for (int i = 0; i < realizations.size(); ++i) {
        plotData.append(realizations[i], "Realization_" + std::to_string(i + 1));
    }

    window->chartViewer()->setYAxisStartAtZero(true);
    window->chartViewer()->setXAxisStartAtZero(true);
    window->chartViewer()->setPlotMode(ChartViewer::PlotMode::Lines);
    window->setData(plotData);
    window->show();
}

void WellDialog::onPlotPercentiles()
{
    if (!well_ || well_->GetPercentile95().size() == 0) {
        QMessageBox::warning(this, tr("No Data"),
            tr("No percentiles available to plot."));
        return;
    }

    // Create chart window
    ChartWindow* window = new ChartWindow(this);
    window->setWindowTitle(tr("Age Distribution Prediction Intervals - %1")
        .arg(QString::fromStdString(well_->getName())));
    window->setChartTitle(tr("MCMC Prediction Intervals - %1")
        .arg(QString::fromStdString(well_->getName())));
    window->setAxisLabels("Age (years)", "Probability Density");

    // Get percentiles
    const TimeSeriesSet<double>& percentiles = well_->GetPercentile95();

    // Create plot data
    TimeSeriesSet<double> plotData;

    // Add percentiles (typically mean, 2.5%, median, 97.5%)
    if (percentiles.size() >= 4) {
        plotData.append(percentiles[0], "Mean");
        plotData.append(percentiles[1], "2.5%");
        plotData.append(percentiles[2], "Median (50%)");
        plotData.append(percentiles[3], "97.5%");
    }
    else {
        // Add whatever percentiles are available
        for (int i = 0; i < percentiles.size(); ++i) {
            plotData.append(percentiles[i], "Percentile_" + std::to_string(i + 1));
        }
    }
    window->chartViewer()->setYAxisStartAtZero(true);
    window->chartViewer()->setXAxisStartAtZero(true);
    window->chartViewer()->setPlotMode(ChartViewer::PlotMode::Lines);
    window->setData(plotData);
    window->show();
}
