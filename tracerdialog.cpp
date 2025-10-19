#include "tracerdialog.h"
#include "parametervaluewidget.h"
#include "constantortimeserieswidget.h"
#include "GWA.h"
#include "Tracer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

TracerDialog::TracerDialog(CGWA* gwa, CTracer* tracer, QWidget* parent)
    : QDialog(parent)
    , gwa_(gwa)
    , tracer_(tracer)
{
    // Set window title based on whether editing or creating
    if (tracer_) {
        setWindowTitle(tr("Edit Tracer"));
    } else {
        setWindowTitle(tr("New Tracer"));
    }

    setupUI();

    // Load data if editing existing tracer
    if (tracer_) {
        loadTracerData(tracer_);
    } else {
        updateParameterWidgets();
    }
}

void TracerDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Basic Properties Group
    QGroupBox* basicGroup = new QGroupBox(tr("Basic Properties"), this);
    QFormLayout* basicLayout = new QFormLayout(basicGroup);

    nameEdit = new QLineEdit(this);
    basicLayout->addRow(tr("Name:"), nameEdit);

    inputWidget = new ConstantOrTimeSeriesWidget(this);
    inputWidget->setPlaceholderText("Enter constant or load time series");
    inputWidget->setToolTip(tr("Atmospheric/input concentration (constant or time series)"));
    basicLayout->addRow(tr("Input:"), inputWidget);

    mainLayout->addWidget(basicGroup);

    // Transport Properties Group
    QGroupBox* transportGroup = new QGroupBox(tr("Transport Properties"), this);
    QFormLayout* transportLayout = new QFormLayout(transportGroup);

    inputMultiplierWidget = new ParameterValueWidget(this);
    inputMultiplierWidget->setPlaceholderText("1.0");
    inputMultiplierWidget->setToolTip(tr("Multiplier for input concentrations"));
    transportLayout->addRow(tr("Input Multiplier:"), inputMultiplierWidget);

    decayRateWidget = new ParameterValueWidget(this);
    decayRateWidget->setPlaceholderText("0.0");
    decayRateWidget->setToolTip(tr("First-order decay rate (1/time)"));
    transportLayout->addRow(tr("Decay Rate:"), decayRateWidget);

    retardationWidget = new ParameterValueWidget(this);
    retardationWidget->setPlaceholderText("1.0");
    retardationWidget->setToolTip(tr("Retardation factor (>=1)"));
    transportLayout->addRow(tr("Retardation:"), retardationWidget);

    mainLayout->addWidget(transportGroup);

    // Water Component Concentrations Group
    QGroupBox* concentrationGroup = new QGroupBox(tr("Water Component Concentrations"), this);
    QFormLayout* concentrationLayout = new QFormLayout(concentrationGroup);

    oldWaterConcWidget = new ParameterValueWidget(this);
    oldWaterConcWidget->setPlaceholderText("0.0");
    oldWaterConcWidget->setToolTip(tr("Concentration in old water component"));
    concentrationLayout->addRow(tr("Old Water (c_old):"), oldWaterConcWidget);

    modernWaterConcWidget = new ParameterValueWidget(this);
    modernWaterConcWidget->setPlaceholderText("0.0");
    modernWaterConcWidget->setToolTip(tr("Concentration in modern water component"));
    concentrationLayout->addRow(tr("Modern Water (c_modern):"), modernWaterConcWidget);

    maxFractionMineralWidget = new ParameterValueWidget(this);
    maxFractionMineralWidget->setPlaceholderText("0.0");
    maxFractionMineralWidget->setToolTip(tr("Maximum fraction mineral"));
    concentrationLayout->addRow(tr("Max Fraction Mineral:"), maxFractionMineralWidget);

    mainLayout->addWidget(concentrationGroup);

    // Options Group
    QGroupBox* optionsGroup = new QGroupBox(tr("Options"), this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);

    vzDelayCheckBox = new QCheckBox(tr("Include vadose zone delay"), this);
    vzDelayCheckBox->setToolTip(tr("Include vadose zone delay in calculations"));
    optionsLayout->addWidget(vzDelayCheckBox);

    linearProductionCheckBox = new QCheckBox(tr("Use linear production model"), this);
    linearProductionCheckBox->setToolTip(tr("Use linear production model for decay chains"));
    optionsLayout->addWidget(linearProductionCheckBox);

    mainLayout->addWidget(optionsGroup);

    // Source Tracer Group
    QGroupBox* sourceGroup = new QGroupBox(tr("Source Tracer (for decay chains)"), this);
    QFormLayout* sourceLayout = new QFormLayout(sourceGroup);

    sourceTracerCombo = new QComboBox(this);
    sourceTracerCombo->addItem(tr("(None)"), "");
    sourceTracerCombo->setToolTip(tr("Parent tracer for decay chain calculations"));
    sourceLayout->addRow(tr("Source Tracer:"), sourceTracerCombo);

    mainLayout->addWidget(sourceGroup);

    // Dialog Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TracerDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
}

void TracerDialog::loadTracerData(const CTracer* tracer)
{
    if (!tracer) return;

    std::string tracerName = tracer->getName();

    // Set name
    nameEdit->setText(QString::fromStdString(tracerName));

    // Set input (constant or time series)
    if (tracer->hasConstantInput()) {
        inputWidget->setConstantValue(tracer->getConstantInputValue());
    } else if (tracer->hasInputFile()) {
        const TimeSeries<double>& input = tracer->getInput();
        inputWidget->setTimeSeries(input);
    }

    // Update parameter widgets first
    updateParameterWidgets();

    // Set transport properties with parameter linkage check
    QString linkedInputMult = findLinkedParameter(tracerName, "input_multiplier");
    if (!linkedInputMult.isEmpty()) {
        inputMultiplierWidget->setParameter(linkedInputMult);
    } else {
        inputMultiplierWidget->setValue(tracer->getInputMultiplier());
    }

    QString linkedDecay = findLinkedParameter(tracerName, "decay");
    if (!linkedDecay.isEmpty()) {
        decayRateWidget->setParameter(linkedDecay);
    } else {
        decayRateWidget->setValue(tracer->getDecayRate());
    }

    QString linkedRetard = findLinkedParameter(tracerName, "retard");
    if (!linkedRetard.isEmpty()) {
        retardationWidget->setParameter(linkedRetard);
    } else {
        retardationWidget->setValue(tracer->getRetardation());
    }

    // Set concentration properties with parameter linkage check
    QString linkedCo = findLinkedParameter(tracerName, "co");
    if (!linkedCo.isEmpty()) {
        oldWaterConcWidget->setParameter(linkedCo);
    } else {
        oldWaterConcWidget->setValue(tracer->getOldWaterConcentration());
    }

    QString linkedCm = findLinkedParameter(tracerName, "cm");
    if (!linkedCm.isEmpty()) {
        modernWaterConcWidget->setParameter(linkedCm);
    } else {
        modernWaterConcWidget->setValue(tracer->getModernWaterConcentration());
    }

    QString linkedFm = findLinkedParameter(tracerName, "fm");
    if (!linkedFm.isEmpty()) {
        maxFractionMineralWidget->setParameter(linkedFm);
    } else {
        maxFractionMineralWidget->setValue(tracer->getMaxFractionModern());
    }

    // Set checkboxes
    vzDelayCheckBox->setChecked(tracer->hasVzDelay());
    linearProductionCheckBox->setChecked(tracer->hasLinearProduction());

    // Set source tracer
    QString sourceTracerName = QString::fromStdString(tracer->getSourceTracerName());
    if (!sourceTracerName.isEmpty()) {
        int index = sourceTracerCombo->findData(sourceTracerName);
        if (index >= 0) {
            sourceTracerCombo->setCurrentIndex(index);
        }
    }
}

void TracerDialog::updateParameterWidgets()
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
    inputMultiplierWidget->setAvailableParameters(paramNames);
    decayRateWidget->setAvailableParameters(paramNames);
    retardationWidget->setAvailableParameters(paramNames);
    oldWaterConcWidget->setAvailableParameters(paramNames);
    modernWaterConcWidget->setAvailableParameters(paramNames);
    maxFractionMineralWidget->setAvailableParameters(paramNames);

    // Update source tracer combo box
    sourceTracerCombo->clear();
    sourceTracerCombo->addItem(tr("(None)"), "");

    for (size_t i = 0; i < gwa_->getTracerCount(); ++i) {
        const CTracer& tracer = gwa_->getTracer(i);
        QString tracerName = QString::fromStdString(tracer.getName());

        // Don't add the current tracer as its own source
        if (tracer_ && tracer.getName() == tracer_->getName()) {
            continue;
        }

        sourceTracerCombo->addItem(tracerName, tracerName);
    }
}

void TracerDialog::onAccepted()
{
    // Validate name
    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Input"),
                             tr("Tracer name cannot be empty."));
        return;
    }

    accept();
}

CTracer TracerDialog::getTracer() const
{
    CTracer tracer;

    // Set name
    tracer.setName(nameEdit->text().toStdString());

    // Set input (constant or time series)
    if (inputWidget->isConstantMode()) {
        tracer.setConstantInput(inputWidget->getConstantValue());
    } else {
        tracer.setInput(inputWidget->getTimeSeries());
    }

    // Set transport properties
    if (!inputMultiplierWidget->isParameterMode()) {
        tracer.setInputMultiplier(inputMultiplierWidget->getValue());
    }

    if (!decayRateWidget->isParameterMode()) {
        tracer.setDecayRate(decayRateWidget->getValue());
    }

    if (!retardationWidget->isParameterMode()) {
        tracer.setRetardation(retardationWidget->getValue());
    }

    // Set concentration properties
    if (!oldWaterConcWidget->isParameterMode()) {
        tracer.setOldWaterConcentration(oldWaterConcWidget->getValue());
    }

    if (!modernWaterConcWidget->isParameterMode()) {
        tracer.setModernWaterConcentration(modernWaterConcWidget->getValue());
    }

    if (!maxFractionMineralWidget->isParameterMode()) {
        tracer.setMaxFractionModern(maxFractionMineralWidget->getValue());
    }

    // Set options
    tracer.setVzDelay(vzDelayCheckBox->isChecked());
    tracer.setLinearProduction(linearProductionCheckBox->isChecked());

    // Set source tracer
    QString sourceTracer = sourceTracerCombo->currentData().toString();
    if (!sourceTracer.isEmpty()) {
        tracer.setSourceTracerName(sourceTracer.toStdString());
    }

    return tracer;
}

QMap<QString, QString> TracerDialog::getParameterLinkages() const
{
    QMap<QString, QString> linkages;

    // Check transport properties
    if (inputMultiplierWidget->isParameterMode()) {
        linkages["input_multiplier"] = inputMultiplierWidget->getParameter();
    }
    if (decayRateWidget->isParameterMode()) {
        linkages["decay"] = decayRateWidget->getParameter();
    }
    if (retardationWidget->isParameterMode()) {
        linkages["retard"] = retardationWidget->getParameter();
    }

    // Check concentration properties
    if (oldWaterConcWidget->isParameterMode()) {
        linkages["co"] = oldWaterConcWidget->getParameter();
    }
    if (modernWaterConcWidget->isParameterMode()) {
        linkages["cm"] = modernWaterConcWidget->getParameter();
    }
    if (maxFractionMineralWidget->isParameterMode()) {
        linkages["fm"] = maxFractionMineralWidget->getParameter();
    }

    return linkages;
}

QString TracerDialog::findLinkedParameter(const std::string& tracerName, const std::string& quantity)
{
    if (!gwa_) return QString();

    // Search through all parameters to find one that links to this tracer property
    for (size_t i = 0; i < gwa_->getParameterCount(); ++i) {
        const Parameter* param = gwa_->getParameter(i);
        if (!param) continue;

        const std::vector<std::string>& locations = param->GetLocations();
        const std::vector<std::string>& quantities = param->GetQuantities();
        const std::vector<std::string>& locationTypes = param->GetLocationTypes();

        // Check each location this parameter is applied to
        for (size_t j = 0; j < locations.size(); ++j) {
            // Check if it's a tracer location (location_type == "tracer" or "1")
            bool isTracer = (locationTypes[j] == "tracer" || locationTypes[j] == "1");

            // Check if it matches our tracer and property
            if (isTracer &&
                locations[j] == tracerName &&
                quantities[j] == quantity) {
                return QString::fromStdString(param->GetName());
            }
        }
    }

    return QString();  // No parameter found
}
