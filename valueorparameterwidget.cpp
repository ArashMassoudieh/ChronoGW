#include "valueorparameterwidget.h"
#include <QLabel>
#include <QDoubleValidator>

ValueOrParameterWidget::ValueOrParameterWidget(CGWA* gwa, QWidget *parent)
    : QWidget(parent)
    , gwa_(gwa)
    , updatingInternally_(false)
{
    setupUI();
}

void ValueOrParameterWidget::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Line edit for numeric value
    lineEdit_ = new QLineEdit(this);
    lineEdit_->setPlaceholderText("Enter value");

    // Set up validator for numeric input
    QDoubleValidator* validator = new QDoubleValidator(this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    lineEdit_->setValidator(validator);

    // Combo box for parameter selection
    paramCombo_ = new QComboBox(this);
    paramCombo_->addItem("(Value)", "");

    // Populate with parameters
    populateParameters();

    layout->addWidget(lineEdit_, 2);
    layout->addWidget(new QLabel("or", this), 0);
    layout->addWidget(paramCombo_, 1);

    // Connect signals
    connect(lineEdit_, &QLineEdit::textChanged,
            this, &ValueOrParameterWidget::onLineEditChanged);
    connect(paramCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ValueOrParameterWidget::onComboBoxChanged);

    setLayout(layout);
}

void ValueOrParameterWidget::populateParameters()
{
    if (!gwa_) return;

    for (size_t i = 0; i < gwa_->getParameterCount(); ++i) {
        const Parameter* param = gwa_->getParameter(i);
        if (param) {
            QString paramName = QString::fromStdString(param->GetName());
            paramCombo_->addItem(paramName, paramName);
        }
    }
}

void ValueOrParameterWidget::setValue(double value)
{
    updatingInternally_ = true;

    // Set line edit
    QString text = QString::number(value, 'g', 10);
    if (!suffix_.isEmpty()) {
        text += suffix_;
    }
    lineEdit_->setText(text);

    // Clear parameter selection
    paramCombo_->setCurrentIndex(0); // "(Value)"

    updatingInternally_ = false;
}

void ValueOrParameterWidget::setParameter(const QString& paramName)
{
    if (paramName.isEmpty()) {
        paramCombo_->setCurrentIndex(0);
        return;
    }

    updatingInternally_ = true;

    // Find and select the parameter
    int index = paramCombo_->findData(paramName);
    if (index >= 0) {
        paramCombo_->setCurrentIndex(index);

        // Clear line edit when parameter is selected
        lineEdit_->clear();
    }

    updatingInternally_ = false;
}

bool ValueOrParameterWidget::isParameter() const
{
    // If combo box is not at "(Value)" position, it's a parameter
    return paramCombo_->currentIndex() > 0 &&
           !paramCombo_->currentData().toString().isEmpty();
}

double ValueOrParameterWidget::getValue() const
{
    if (isParameter()) {
        // Get value from the selected parameter
        if (gwa_) {
            QString paramName = paramCombo_->currentData().toString();
            int paramIdx = gwa_->findParameter(paramName.toStdString());
            if (paramIdx >= 0) {
                const Parameter* param = gwa_->getParameter(paramIdx);
                if (param) {
                    return param->GetValue();
                }
            }
        }
        return 0.0;
    } else {
        // Get value from line edit
        QString text = lineEdit_->text();

        // Remove suffix if present
        if (!suffix_.isEmpty() && text.endsWith(suffix_)) {
            text = text.left(text.length() - suffix_.length());
        }

        bool ok;
        double value = text.toDouble(&ok);
        return ok ? value : 0.0;
    }
}

QString ValueOrParameterWidget::getParameter() const
{
    if (isParameter()) {
        return paramCombo_->currentData().toString();
    }
    return QString();
}

void ValueOrParameterWidget::setPlaceholder(const QString& text)
{
    lineEdit_->setPlaceholderText(text);
}

void ValueOrParameterWidget::setSuffix(const QString& suffix)
{
    suffix_ = suffix;
}

void ValueOrParameterWidget::onLineEditChanged()
{
    if (updatingInternally_) return;

    // When user types in line edit, reset combo box to "(Value)"
    if (!lineEdit_->text().isEmpty()) {
        updatingInternally_ = true;
        paramCombo_->setCurrentIndex(0);
        updatingInternally_ = false;

        emit valueChanged();
    }
}

void ValueOrParameterWidget::onComboBoxChanged(int index)
{
    if (updatingInternally_) return;

    // When user selects a parameter, clear the line edit
    if (index > 0) {
        updatingInternally_ = true;
        lineEdit_->clear();
        updatingInternally_ = false;

        emit parameterChanged();
    } else {
        emit valueChanged();
    }
}
