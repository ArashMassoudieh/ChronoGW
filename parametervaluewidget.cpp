#include "parametervaluewidget.h"
#include <QDoubleValidator>

ParameterValueWidget::ParameterValueWidget(QWidget *parent)
    : QWidget(parent)
    , updatingInternally(false)
{
    // Create widgets
    lineEdit = new QLineEdit(this);
    comboBox = new QComboBox(this);

    // Configure line edit
    lineEdit->setValidator(new QDoubleValidator(this));
    lineEdit->setPlaceholderText("Enter value");

    // Configure combo box
    comboBox->addItem("(Value)", "");  // Default item for value mode
    comboBox->setMinimumWidth(150);

    // Create layout - just line edit and combo box side by side
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(lineEdit);
    layout->addWidget(comboBox);

    // Connect signals
    connect(lineEdit, &QLineEdit::textChanged,
            this, &ParameterValueWidget::onLineEditChanged);
    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ParameterValueWidget::onComboBoxChanged);

    setLayout(layout);
}

void ParameterValueWidget::setAvailableParameters(const QVector<QString>& parameterNames)
{
    updatingInternally = true;

    QString currentParam = getParameter();

    // Clear and rebuild combo box
    comboBox->clear();
    comboBox->addItem("(Value)", "");  // Default item

    for (const QString& name : parameterNames) {
        comboBox->addItem(name, name);
    }

    // Restore previous selection if it still exists
    if (!currentParam.isEmpty()) {
        int index = comboBox->findData(currentParam);
        if (index >= 0) {
            comboBox->setCurrentIndex(index);
        }
    }

    updatingInternally = false;
}

void ParameterValueWidget::setValue(double value)
{
    updatingInternally = true;

    lineEdit->setText(QString::number(value));
    comboBox->setCurrentIndex(0);  // Select "(Value)"
    lineEdit->setEnabled(true);

    updatingInternally = false;
    emit valueChanged();
}

void ParameterValueWidget::setParameter(const QString& parameterName)
{
    updatingInternally = true;

    if (parameterName.isEmpty()) {
        // Switch to value mode
        comboBox->setCurrentIndex(0);
        lineEdit->setEnabled(true);
    } else {
        // Find and select the parameter
        int index = comboBox->findData(parameterName);
        if (index >= 0) {
            comboBox->setCurrentIndex(index);
            lineEdit->setEnabled(false);
            lineEdit->clear();
        }
    }

    updatingInternally = false;
    emit valueChanged();
}

bool ParameterValueWidget::isParameterMode() const
{
    return comboBox->currentIndex() > 0;
}

double ParameterValueWidget::getValue() const
{
    if (isParameterMode()) {
        return 0.0;
    }
    return lineEdit->text().toDouble();
}

QString ParameterValueWidget::getParameter() const
{
    if (comboBox->currentIndex() == 0) {
        return QString();
    }
    return comboBox->currentData().toString();
}

void ParameterValueWidget::setEnabled(bool enabled)
{
    lineEdit->setEnabled(enabled && !isParameterMode());
    comboBox->setEnabled(enabled);
    QWidget::setEnabled(enabled);
}

void ParameterValueWidget::setPlaceholderText(const QString& text)
{
    lineEdit->setPlaceholderText(text);
}

void ParameterValueWidget::onLineEditChanged()
{
    if (updatingInternally) {
        return;
    }

    // If user types in line edit, switch to value mode
    if (!lineEdit->text().isEmpty() && comboBox->currentIndex() != 0) {
        updatingInternally = true;
        comboBox->setCurrentIndex(0);
        updatingInternally = false;
    }

    emit valueChanged();
}

void ParameterValueWidget::onComboBoxChanged(int index)
{
    if (updatingInternally) {
        return;
    }

    if (index == 0) {
        // Value mode - enable line edit
        lineEdit->setEnabled(true);
    } else {
        // Parameter mode - disable line edit and clear it
        lineEdit->setEnabled(false);
        lineEdit->clear();
    }

    emit valueChanged();
}
