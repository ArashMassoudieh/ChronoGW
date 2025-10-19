#include "welldialog.h"
#include "parameterorvaluewidget.h"
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>


WellDialog::WellDialog(CWell* well, CGWA* gwa, QWidget *parent)
    : QDialog(parent)
    , well_(well)
    , gwa_(gwa)
{
    // Common distribution types
    distributionTypes << "Exponential"
                      << "Exponential-Piston Flow"
                      << "Dispersion"
                      << "Linear"
                      << "Histogram"
                      << "Gamma";

    setupUI();
    loadWellData();

    setWindowTitle(tr("Well Properties"));
    resize(500, 600);
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
    mainLayout->addWidget(distributionParamsGroup);

    // Mixing Parameters Group
    QGroupBox* mixingGroup = new QGroupBox(tr("Mixing Parameters"), this);
    QFormLayout* mixingLayout = new QFormLayout(mixingGroup);

    fractionOldWidget = new ParameterOrValueWidget(gwa_, tr("Fraction Old"), this);
    fractionOldWidget->valueSpin->setRange(0.0, 1.0);
    fractionOldWidget->valueSpin->setSingleStep(0.01);
    fractionOldWidget->valueSpin->setToolTip(tr("Fraction of old water (0-1)"));
    mixingLayout->addRow(tr("Fraction Old (f):"), fractionOldWidget);

    ageOldWidget = new ParameterOrValueWidget(gwa_, tr("Age Old"), this);
    ageOldWidget->valueSpin->setRange(0.0, 10000.0);
    ageOldWidget->valueSpin->setSingleStep(1.0);
    ageOldWidget->valueSpin->setSuffix(tr(" years"));
    ageOldWidget->valueSpin->setToolTip(tr("Age of old water component"));
    mixingLayout->addRow(tr("Age Old:"), ageOldWidget);

    fractionMineralWidget = new ParameterOrValueWidget(gwa_, tr("Fraction Mineral"), this);
    fractionMineralWidget->valueSpin->setRange(0.0, 2.0);
    fractionMineralWidget->valueSpin->setSingleStep(0.01);
    fractionMineralWidget->valueSpin->setToolTip(tr("Fraction mineral (fm) for mineral correction"));
    mixingLayout->addRow(tr("Fraction Mineral (fm):"), fractionMineralWidget);

    vzDelayWidget = new ParameterOrValueWidget(gwa_, tr("Vadose Zone Delay"), this);
    vzDelayWidget->valueSpin->setRange(0.0, 100.0);
    vzDelayWidget->valueSpin->setSingleStep(0.1);
    vzDelayWidget->valueSpin->setSuffix(tr(" years"));
    vzDelayWidget->valueSpin->setToolTip(tr("Vadose zone delay time"));
    mixingLayout->addRow(tr("Vadose Zone Delay:"), vzDelayWidget);

    mainLayout->addWidget(mixingGroup);

    // Dialog Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &WellDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}

void WellDialog::loadWellData()
{
    if (!well_ || !gwa_) return;

    // Load basic properties
    nameEdit->setText(QString::fromStdString(well_->getName()));

    QString distType = QString::fromStdString(well_->getDistributionType());
    int index = distributionTypeCombo->findText(distType);
    if (index >= 0) {
        distributionTypeCombo->setCurrentIndex(index);
    } else {
        distributionTypeCombo->setCurrentText(distType);
    }

    // Load mixing parameters - check if they're linked to parameters
    std::string wellName = well_->getName();

    // Check fraction old (f)
    QString linkedParam = findLinkedParameter(wellName, "f");
    if (!linkedParam.isEmpty()) {
        fractionOldWidget->setParameter(linkedParam);
    } else {
        fractionOldWidget->setValue(well_->getFractionOld());
    }

    // Check age old
    linkedParam = findLinkedParameter(wellName, "age_old");
    if (!linkedParam.isEmpty()) {
        ageOldWidget->setParameter(linkedParam);
    } else {
        ageOldWidget->setValue(well_->getAgeOld());
    }

    // Check fraction mineral (fm)
    linkedParam = findLinkedParameter(wellName, "fm");
    if (!linkedParam.isEmpty()) {
        fractionMineralWidget->setParameter(linkedParam);
    } else {
        fractionMineralWidget->setValue(well_->getFractionMineral());
    }

    // Check vadose zone delay
    linkedParam = findLinkedParameter(wellName, "vz_delay");
    if (!linkedParam.isEmpty()) {
        vzDelayWidget->setParameter(linkedParam);
    } else {
        vzDelayWidget->setValue(well_->getVzDelay());
    }

    // Load distribution parameters
    createDistributionParameterInputs();
}

void WellDialog::onDistributionTypeChanged(const QString& type)
{
    createDistributionParameterInputs();
}

void WellDialog::createDistributionParameterInputs()
{
    // Clear existing parameter inputs
    while (distributionParamsLayout->count() > 0) {
        QLayoutItem* item = distributionParamsLayout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    distributionParamWidgets.clear();

    if (!well_) return;

    // Get current parameters from well
    const std::vector<double>& params = well_->getParameters();

    // Determine number of parameters based on distribution type
    QString distType = distributionTypeCombo->currentText();
    int paramCount = 0;

    if (distType.contains("Exponential", Qt::CaseInsensitive) &&
        !distType.contains("Piston", Qt::CaseInsensitive)) {
        paramCount = 1;
    } else if (distType.contains("Dispersion", Qt::CaseInsensitive)) {
        paramCount = 2;
    } else if (distType.contains("Piston", Qt::CaseInsensitive)) {
        paramCount = 2;
    } else if (distType.contains("Gamma", Qt::CaseInsensitive)) {
        paramCount = 2;
    } else if (distType.contains("Linear", Qt::CaseInsensitive)) {
        paramCount = 1;
    } else {
        paramCount = params.size();
    }

    // Create parameter/value widgets
    for (int i = 0; i < paramCount; ++i) {
        ParameterOrValueWidget* widget = new ParameterOrValueWidget(gwa_,
                                                                    tr("Distribution Parameter %1").arg(i), this);

        // Check if this parameter is linked to a parameter
        QString linkedParam = findLinkedParameter(well_->getName(),
                                                  QString("param[%1]").arg(i).toStdString());
        if (!linkedParam.isEmpty()) {
            widget->setParameter(linkedParam);
        } else {
            // Set value if available
            if (i < static_cast<int>(params.size())) {
                widget->setValue(params[i]);
            }
        }

        distributionParamWidgets.append(widget);
        distributionParamsLayout->addRow(tr("Parameter [%1]:").arg(i), widget);
    }

    // Add help text
    QString helpText;
    if (distType.contains("Exponential", Qt::CaseInsensitive) &&
        !distType.contains("Piston", Qt::CaseInsensitive)) {
        helpText = tr("Parameter [0]: Mean residence time (years)");
    } else if (distType.contains("Dispersion", Qt::CaseInsensitive)) {
        helpText = tr("Parameter [0]: Mean transit time\nParameter [1]: Dispersion parameter");
    } else if (distType.contains("Piston", Qt::CaseInsensitive)) {
        helpText = tr("Parameter [0]: Exponential fraction\nParameter [1]: Piston flow time");
    } else if (distType.contains("Gamma", Qt::CaseInsensitive)) {
        helpText = tr("Parameter [0]: Alpha (shape)\nParameter [1]: Beta (scale)");
    }

    if (!helpText.isEmpty()) {
        QLabel* helpLabel = new QLabel(helpText, this);
        helpLabel->setStyleSheet("QLabel { color: gray; font-size: 9pt; }");
        helpLabel->setWordWrap(true);
        distributionParamsLayout->addRow(helpLabel);
    }
}

void WellDialog::onAccepted()
{
    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"),
                             tr("Well name cannot be empty."));
        return;
    }

    accept();
}

CWell WellDialog::getWell() const
{
    CWell updatedWell;

    // Set basic properties
    updatedWell.setName(nameEdit->text().toStdString());
    updatedWell.setDistributionType(distributionTypeCombo->currentText().toStdString());

    // Set mixing parameters (get actual values, not parameter names)
    updatedWell.setFractionOld(fractionOldWidget->getValue());
    updatedWell.setAgeOld(ageOldWidget->getValue());
    updatedWell.setFractionModern(fractionMineralWidget->getValue());
    updatedWell.setVzDelay(vzDelayWidget->getValue());

    // Set distribution parameters
    std::vector<double> params;
    for (ParameterOrValueWidget* widget : distributionParamWidgets) {
        params.push_back(widget->getValue());
    }
    updatedWell.setParameters(params);

    return updatedWell;
}

QMap<QString, QString> WellDialog::getParameterLinkages() const
{
    QMap<QString, QString> linkages;

    if (fractionOldWidget->isParameter()) {
        linkages["f"] = fractionOldWidget->getParameter();
    }
    if (ageOldWidget->isParameter()) {
        linkages["age_old"] = ageOldWidget->getParameter();
    }
    if (fractionMineralWidget->isParameter()) {
        linkages["fm"] = fractionMineralWidget->getParameter();
    }
    if (vzDelayWidget->isParameter()) {
        linkages["vz_delay"] = vzDelayWidget->getParameter();
    }

    for (int i = 0; i < distributionParamWidgets.size(); ++i) {
        if (distributionParamWidgets[i]->isParameter()) {
            linkages[QString("param[%1]").arg(i)] = distributionParamWidgets[i]->getParameter();
        }
    }

    return linkages;
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
