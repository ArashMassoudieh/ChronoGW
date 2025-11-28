#include "TimeSeriesPointDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <limits>

TimeSeriesPointDialog::TimeSeriesPointDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
}

void TimeSeriesPointDialog::setupUi()
{
    setWindowTitle(tr("Add Point"));
    setModal(true);
    setMinimumWidth(300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Form layout for inputs
    QFormLayout* formLayout = new QFormLayout();

    // Time input
    timeSpinBox_ = new QDoubleSpinBox();
    timeSpinBox_->setRange(-1e9, 1e9);
    timeSpinBox_->setDecimals(4);
    timeSpinBox_->setSingleStep(1.0);
    formLayout->addRow(tr("Time:"), timeSpinBox_);

    // Value input
    valueSpinBox_ = new QDoubleSpinBox();
    valueSpinBox_->setRange(-1e15, 1e15);
    valueSpinBox_->setDecimals(6);
    valueSpinBox_->setSingleStep(0.1);
    formLayout->addRow(tr("Value:"), valueSpinBox_);

    mainLayout->addLayout(formLayout);

    // Spacer
    mainLayout->addStretch();

    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    // Set focus to time field
    timeSpinBox_->setFocus();
    timeSpinBox_->selectAll();
}

void TimeSeriesPointDialog::setEditMode(double time, double value)
{
    editMode_ = true;
    setWindowTitle(tr("Edit Point"));
    timeSpinBox_->setValue(time);
    valueSpinBox_->setValue(value);
}

void TimeSeriesPointDialog::setAddMode(double suggestedTime)
{
    editMode_ = false;
    setWindowTitle(tr("Add Point"));
    timeSpinBox_->setValue(suggestedTime);
    valueSpinBox_->setValue(0.0);
}

double TimeSeriesPointDialog::getTime() const
{
    return timeSpinBox_->value();
}

double TimeSeriesPointDialog::getValue() const
{
    return valueSpinBox_->value();
}

void TimeSeriesPointDialog::setTimeRange(double min, double max)
{
    timeSpinBox_->setRange(min, max);
}

void TimeSeriesPointDialog::setValueRange(double min, double max)
{
    valueSpinBox_->setRange(min, max);
}

void TimeSeriesPointDialog::setDecimals(int timeDecimals, int valueDecimals)
{
    timeSpinBox_->setDecimals(timeDecimals);
    valueSpinBox_->setDecimals(valueDecimals);
}
