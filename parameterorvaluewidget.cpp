#include "parameterorvaluewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>

ParameterOrValueWidget::ParameterOrValueWidget(CGWA* gwa, const QString& label, QWidget* parent)
    : QWidget(parent)
    , gwa_(gwa)
    , useParameter(false)
{
    setupUI(label);
}

void ParameterOrValueWidget::setupUI(const QString& label)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Radio buttons for mode selection
    QHBoxLayout* radioLayout = new QHBoxLayout();
    valueRadio = new QRadioButton(tr("Value"), this);
    paramRadio = new QRadioButton(tr("Parameter"), this);
    valueRadio->setChecked(true);

    QButtonGroup* radioGroup = new QButtonGroup(this);
    radioGroup->addButton(valueRadio);
    radioGroup->addButton(paramRadio);

    radioLayout->addWidget(valueRadio);
    radioLayout->addWidget(paramRadio);
    radioLayout->addStretch();
    mainLayout->addLayout(radioLayout);

    // Stacked widget for value/parameter input
    inputStack = new QStackedWidget(this);

    // Value input page
    valueSpin = new QDoubleSpinBox(this);
    valueSpin->setRange(-10000.0, 10000.0);
    valueSpin->setSingleStep(0.1);
    valueSpin->setDecimals(4);
    inputStack->addWidget(valueSpin);

    // Parameter selection page
    paramCombo = new QComboBox(this);
    if (gwa_) {
        for (size_t i = 0; i < gwa_->getParameterCount(); ++i) {
            const Parameter* param = gwa_->getParameter(i);
            if (param) {
                paramCombo->addItem(QString::fromStdString(param->GetName()));
            }
        }
    }
    inputStack->addWidget(paramCombo);

    mainLayout->addWidget(inputStack);

    // Connect radio buttons
    connect(valueRadio, &QRadioButton::toggled, this, &ParameterOrValueWidget::updateMode);
    connect(valueSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ParameterOrValueWidget::valueChanged);
    connect(paramCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ParameterOrValueWidget::valueChanged);
}

void ParameterOrValueWidget::updateMode()
{
    useParameter = paramRadio->isChecked();
    inputStack->setCurrentIndex(useParameter ? 1 : 0);
    emit valueChanged();
}

void ParameterOrValueWidget::setValue(double value)
{
    valueRadio->setChecked(true);
    valueSpin->setValue(value);
    useParameter = false;
    inputStack->setCurrentIndex(0);
}

void ParameterOrValueWidget::setParameter(const QString& paramName)
{
    paramRadio->setChecked(true);
    int index = paramCombo->findText(paramName);
    if (index >= 0) {
        paramCombo->setCurrentIndex(index);
    }
    useParameter = true;
    inputStack->setCurrentIndex(1);
}

double ParameterOrValueWidget::getValue() const
{
    if (useParameter) {
        // If using parameter, return its current value
        if (gwa_) {
            int paramIdx = gwa_->findParameter(paramCombo->currentText().toStdString());
            if (paramIdx >= 0) {
                const Parameter* param = gwa_->getParameter(paramIdx);
                if (param) {
                    return param->GetValue();
                }
            }
        }
        return 0.0;
    }
    return valueSpin->value();
}

QString ParameterOrValueWidget::getParameter() const
{
    if (useParameter) {
        return paramCombo->currentText();
    }
    return QString();
}
