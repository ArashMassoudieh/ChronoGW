/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 *
 * This file is part of OpenHydroQual.
 *
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 */

#include "ProgressWindow.h"
#include <QGridLayout>
#include <QDateTime>
#include <QApplication>
#include <QAreaSeries>

ProgressWindow::ProgressWindow(QWidget* parent, const QString& title)
    : QDialog(parent)
    , primaryAutoScale_(true)
    , secondaryAutoScale_(true)
    , pauseRequested_(false)
    , cancelRequested_(false)
    , isPaused_(false)
{
    setWindowTitle(title);
    setMinimumSize(800, 600);

    setupUI();
    createPrimaryChart();
    createSecondaryChart();

    // Default: Show primary chart, hide secondary
    SetPrimaryChartVisible(true);
    SetSecondaryChartVisible(false);
    SetSecondaryProgressVisible(false);
}

ProgressWindow::~ProgressWindow()
{
}

void ProgressWindow::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Status label
    statusLabel_ = new QLabel("Ready", this);
    statusLabel_->setStyleSheet("QLabel { font-weight: bold; font-size: 12pt; }");
    mainLayout->addWidget(statusLabel_);

    // Primary progress bar
    primaryProgressLabel_ = new QLabel("Progress:", this);
    mainLayout->addWidget(primaryProgressLabel_);

    primaryProgressBar_ = new QProgressBar(this);
    primaryProgressBar_->setRange(0, 1000);
    primaryProgressBar_->setValue(0);
    primaryProgressBar_->setTextVisible(true);
    primaryProgressBar_->setFormat("%p%");
    mainLayout->addWidget(primaryProgressBar_);

    // Secondary progress bar (initially hidden)
    secondaryProgressLabel_ = new QLabel("Secondary Progress:", this);
    mainLayout->addWidget(secondaryProgressLabel_);

    secondaryProgressBar_ = new QProgressBar(this);
    secondaryProgressBar_->setRange(0, 1000);
    secondaryProgressBar_->setValue(0);
    secondaryProgressBar_->setTextVisible(true);
    secondaryProgressBar_->setFormat("%p%");
    mainLayout->addWidget(secondaryProgressBar_);

    // Chart placeholders (will be created later)
    primaryChartView_ = nullptr;
    secondaryChartView_ = nullptr;

    // Information Panel (initially hidden)
    infoPanelLabel_ = new QLabel("Information:", this);
    mainLayout->addWidget(infoPanelLabel_);

    infoTextEdit_ = new QTextEdit(this);
    infoTextEdit_->setReadOnly(true);
    infoTextEdit_->setMaximumHeight(200);
    infoTextEdit_->setStyleSheet(
        "QTextEdit { "
        "background-color: #f5f5f5; "
        "border: 1px solid #cccccc; "
        "border-radius: 3px; "
        "padding: 5px; "
        "font-family: 'Courier New', monospace; "
        "}"
        );
    mainLayout->addWidget(infoTextEdit_);

    // Initially hide info panel
    infoPanelLabel_->setVisible(false);
    infoTextEdit_->setVisible(false);

    // Log area
    QLabel* logLabel = new QLabel("Log:", this);
    mainLayout->addWidget(logLabel);

    logTextEdit_ = new QTextEdit(this);
    logTextEdit_->setReadOnly(true);
    logTextEdit_->setMaximumHeight(150);
    mainLayout->addWidget(logTextEdit_);

    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    pauseResumeButton_ = new QPushButton("Pause", this);
    cancelButton_ = new QPushButton("Cancel", this);

    buttonLayout->addWidget(pauseResumeButton_);
    buttonLayout->addWidget(cancelButton_);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(pauseResumeButton_, &QPushButton::clicked, this, &ProgressWindow::onPauseResumeClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &ProgressWindow::onCancelClicked);
}

void ProgressWindow::createPrimaryChart()
{
    // Create chart
    primaryChart_ = new QChart();
    primaryChart_->setTitle("Primary Chart");
    primaryChart_->setAnimationOptions(QChart::NoAnimation);
    primaryChart_->legend()->hide();

    // Create line series
    primarySeries_ = new QLineSeries();
    primarySeries_->setName("Primary Data");

    // Set line pen
    QPen pen(Qt::blue);
    pen.setWidth(2);

    // Create area series for filling
    primaryAreaSeries_ = new QAreaSeries(primarySeries_);
    primaryAreaSeries_->setName("Primary Data");

    // Set fill color with transparency
    QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
    gradient.setColorAt(0.0, QColor(0, 0, 255, 180));    // Blue at top
    gradient.setColorAt(1.0, QColor(0, 0, 255, 40));     // Light blue at bottom
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    primaryAreaSeries_->setBrush(gradient);
    primaryAreaSeries_->setPen(pen);

    // Add area series to chart
    primaryChart_->addSeries(primaryAreaSeries_);

    // Create axes
    primaryAxisX_ = new QValueAxis();
    primaryAxisX_->setTitleText("X");
    primaryAxisX_->setLabelFormat("%g");
    primaryAxisX_->setRange(0, 100);
    primaryChart_->addAxis(primaryAxisX_, Qt::AlignBottom);
    primaryAreaSeries_->attachAxis(primaryAxisX_);

    primaryAxisY_ = new QValueAxis();
    primaryAxisY_->setTitleText("Y");
    primaryAxisY_->setLabelFormat("%.2e");
    primaryAxisY_->setRange(0, 1);
    primaryChart_->addAxis(primaryAxisY_, Qt::AlignLeft);
    primaryAreaSeries_->attachAxis(primaryAxisY_);

    // Create chart view
    primaryChartView_ = new QChartView(primaryChart_, this);
    primaryChartView_->setRenderHint(QPainter::Antialiasing);
    primaryChartView_->setMinimumHeight(250);

    // Add to layout (after secondary progress bar)
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->insertWidget(4, primaryChartView_);
    }
}

void ProgressWindow::createSecondaryChart()
{
    // Create chart
    secondaryChart_ = new QChart();
    secondaryChart_->setTitle("Secondary Chart");
    secondaryChart_->setAnimationOptions(QChart::NoAnimation);
    secondaryChart_->legend()->hide();

    // Create line series
    secondarySeries_ = new QLineSeries();
    secondarySeries_->setName("Secondary Data");

    // Set line pen
    QPen pen(Qt::red);
    pen.setWidth(2);

    // Create area series for filling
    secondaryAreaSeries_ = new QAreaSeries(secondarySeries_);
    secondaryAreaSeries_->setName("Secondary Data");

    // Set fill color with transparency
    QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
    gradient.setColorAt(0.0, QColor(255, 0, 0, 180));    // Red at top
    gradient.setColorAt(1.0, QColor(255, 0, 0, 40));     // Light red at bottom
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    secondaryAreaSeries_->setBrush(gradient);
    secondaryAreaSeries_->setPen(pen);

    // Add area series to chart
    secondaryChart_->addSeries(secondaryAreaSeries_);

    // Create axes
    secondaryAxisX_ = new QValueAxis();
    secondaryAxisX_->setTitleText("X");
    secondaryAxisX_->setLabelFormat("%g");
    secondaryAxisX_->setRange(0, 1000);
    secondaryChart_->addAxis(secondaryAxisX_, Qt::AlignBottom);
    secondaryAreaSeries_->attachAxis(secondaryAxisX_);

    secondaryAxisY_ = new QValueAxis();
    secondaryAxisY_->setTitleText("Y");
    secondaryAxisY_->setLabelFormat("%.2f");
    secondaryAxisY_->setRange(-1000, 0);
    secondaryChart_->addAxis(secondaryAxisY_, Qt::AlignLeft);
    secondaryAreaSeries_->attachAxis(secondaryAxisY_);

    // Create chart view
    secondaryChartView_ = new QChartView(secondaryChart_, this);
    secondaryChartView_->setRenderHint(QPainter::Antialiasing);
    secondaryChartView_->setMinimumHeight(250);

    // Add to layout (after primary chart)
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->insertWidget(5, secondaryChartView_);
    }
}

// ============================================================================
// Progress Bar Methods
// ============================================================================

void ProgressWindow::SetProgress(double progress)
{
    int value = static_cast<int>(progress * 1000);
    primaryProgressBar_->setValue(value);
    QApplication::processEvents();
}

void ProgressWindow::SetSecondaryProgress(double progress)
{
    int value = static_cast<int>(progress * 1000);
    secondaryProgressBar_->setValue(value);
    QApplication::processEvents();
}

void ProgressWindow::SetSecondaryProgressVisible(bool visible)
{
    secondaryProgressLabel_->setVisible(visible);
    secondaryProgressBar_->setVisible(visible);
}

void ProgressWindow::SetProgressLabel(const QString& label)
{
    primaryProgressLabel_->setText(label);
}

void ProgressWindow::SetSecondaryProgressLabel(const QString& label)
{
    secondaryProgressLabel_->setText(label);
}

// ============================================================================
// Status and Logging Methods
// ============================================================================

void ProgressWindow::SetStatus(const QString& status)
{
    statusLabel_->setText(status);
    QApplication::processEvents();
}

void ProgressWindow::AppendLog(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logTextEdit_->append(QString("[%1] %2").arg(timestamp).arg(message));

    // Auto-scroll to bottom
    QTextCursor cursor = logTextEdit_->textCursor();
    cursor.movePosition(QTextCursor::End);
    logTextEdit_->setTextCursor(cursor);

    QApplication::processEvents();
}

void ProgressWindow::ClearLog()
{
    logTextEdit_->clear();
}

// ============================================================================
// Information Panel Methods
// ============================================================================

void ProgressWindow::AppendInfo(const QString& text)
{
    infoTextEdit_->append(text);

    // Auto-scroll to bottom
    QTextCursor cursor = infoTextEdit_->textCursor();
    cursor.movePosition(QTextCursor::End);
    infoTextEdit_->setTextCursor(cursor);

    QApplication::processEvents();
}

void ProgressWindow::SetInfoText(const QString& text)
{
    infoTextEdit_->setPlainText(text);
    QApplication::processEvents();
}

void ProgressWindow::ClearInfo()
{
    infoTextEdit_->clear();
}

void ProgressWindow::SetInfoPanelVisible(bool visible)
{
    infoPanelLabel_->setVisible(visible);
    infoTextEdit_->setVisible(visible);
}

void ProgressWindow::SetInfoPanelLabel(const QString& label)
{
    infoPanelLabel_->setText(label);
}

// ============================================================================
// Primary Chart Methods
// ============================================================================

void ProgressWindow::AddPrimaryChartPoint(double x, double y)
{
    primaryData_.append(QPointF(x, y));
    updatePrimaryChart();
}

void ProgressWindow::ClearPrimaryChartData()
{
    primaryData_.clear();
    primarySeries_->clear();
}

void ProgressWindow::SetPrimaryChartYRange(double min, double max)
{
    primaryAutoScale_ = false;
    primaryAxisY_->setRange(min, max);
}

void ProgressWindow::SetPrimaryChartXRange(double min, double max)
{
    if (primaryAxisX_) {
        primaryAxisX_->setRange(min, max);
    }
}

void ProgressWindow::SetPrimaryChartAutoScale(bool enabled)
{
    primaryAutoScale_ = enabled;
}

void ProgressWindow::SetPrimaryChartVisible(bool visible)
{
    if (primaryChartView_) {
        primaryChartView_->setVisible(visible);
    }
}

void ProgressWindow::SetPrimaryChartTitle(const QString& title)
{
    if (primaryChart_) {
        primaryChart_->setTitle(title);
    }
}

void ProgressWindow::SetPrimaryChartXAxisTitle(const QString& title)
{
    if (primaryAxisX_) {
        primaryAxisX_->setTitleText(title);
    }
}

void ProgressWindow::SetPrimaryChartYAxisTitle(const QString& title)
{
    if (primaryAxisY_) {
        primaryAxisY_->setTitleText(title);
    }
}

void ProgressWindow::SetPrimaryChartColor(const QColor& color)
{
    if (primaryAreaSeries_) {
        // Update line pen
        QPen pen = primaryAreaSeries_->pen();
        pen.setColor(color);
        primaryAreaSeries_->setPen(pen);

        // Update gradient
        QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
        gradient.setColorAt(0.0, QColor(color.red(), color.green(), color.blue(), 180));
        gradient.setColorAt(1.0, QColor(color.red(), color.green(), color.blue(), 40));
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        primaryAreaSeries_->setBrush(gradient);
    }
}

void ProgressWindow::updatePrimaryChart()
{
    if (primaryData_.isEmpty()) {
        return;
    }

    // Update series
    primarySeries_->replace(primaryData_);

    // Auto-scale if enabled
    if (primaryAutoScale_ && !primaryData_.isEmpty()) {
        // Find min/max
        double minVal = primaryData_[0].y();
        double maxVal = primaryData_[0].y();

        for (const QPointF& point : primaryData_) {
            minVal = qMin(minVal, point.y());
            maxVal = qMax(maxVal, point.y());
        }

        // Add 10% padding
        double range = maxVal - minVal;
        if (range > 0) {
            minVal -= range * 0.1;
            maxVal += range * 0.1;
        } else {
            minVal -= 0.1;
            maxVal += 0.1;
        }

        primaryAxisY_->setRange(minVal, maxVal);
    }

    QApplication::processEvents();
}

// ============================================================================
// Secondary Chart Methods
// ============================================================================

void ProgressWindow::AddSecondaryChartPoint(double x, double y)
{
    secondaryData_.append(QPointF(x, y));
    updateSecondaryChart();
}

void ProgressWindow::ClearSecondaryChartData()
{
    secondaryData_.clear();
    secondarySeries_->clear();
}

void ProgressWindow::SetSecondaryChartYRange(double min, double max)
{
    secondaryAutoScale_ = false;
    secondaryAxisY_->setRange(min, max);
}

void ProgressWindow::SetSecondaryChartXRange(double min, double max)
{
    if (secondaryAxisX_) {
        secondaryAxisX_->setRange(min, max);
    }
}

void ProgressWindow::SetSecondaryChartAutoScale(bool enabled)
{
    secondaryAutoScale_ = enabled;
}

void ProgressWindow::SetSecondaryChartVisible(bool visible)
{
    if (secondaryChartView_) {
        secondaryChartView_->setVisible(visible);
    }
}

void ProgressWindow::SetSecondaryChartTitle(const QString& title)
{
    if (secondaryChart_) {
        secondaryChart_->setTitle(title);
    }
}

void ProgressWindow::SetSecondaryChartXAxisTitle(const QString& title)
{
    if (secondaryAxisX_) {
        secondaryAxisX_->setTitleText(title);
    }
}

void ProgressWindow::SetSecondaryChartYAxisTitle(const QString& title)
{
    if (secondaryAxisY_) {
        secondaryAxisY_->setTitleText(title);
    }
}

void ProgressWindow::SetSecondaryChartColor(const QColor& color)
{
    if (secondaryAreaSeries_) {
        // Update line pen
        QPen pen = secondaryAreaSeries_->pen();
        pen.setColor(color);
        secondaryAreaSeries_->setPen(pen);

        // Update gradient
        QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
        gradient.setColorAt(0.0, QColor(color.red(), color.green(), color.blue(), 180));
        gradient.setColorAt(1.0, QColor(color.red(), color.green(), color.blue(), 40));
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        secondaryAreaSeries_->setBrush(gradient);
    }
}

void ProgressWindow::updateSecondaryChart()
{
    if (secondaryData_.isEmpty()) {
        return;
    }

    // Update series
    secondarySeries_->replace(secondaryData_);

    // Auto-scale if enabled
    if (secondaryAutoScale_ && !secondaryData_.isEmpty()) {
        // Find min/max
        double minVal = secondaryData_[0].y();
        double maxVal = secondaryData_[0].y();

        for (const QPointF& point : secondaryData_) {
            minVal = qMin(minVal, point.y());
            maxVal = qMax(maxVal, point.y());
        }

        // Add 10% padding
        double range = maxVal - minVal;
        if (range > 0) {
            minVal -= range * 0.1;
            maxVal += range * 0.1;
        } else {
            minVal -= 0.1;
            maxVal += 0.1;
        }

        secondaryAxisY_->setRange(minVal, maxVal);
    }

    QApplication::processEvents();
}

// ============================================================================
// Control Methods
// ============================================================================

void ProgressWindow::SetPauseEnabled(bool enabled)
{
    pauseResumeButton_->setEnabled(enabled);
}

void ProgressWindow::SetComplete(const QString& message)
{
    SetStatus(message);
    pauseResumeButton_->setEnabled(false);
    cancelButton_->setText("Close");
}

void ProgressWindow::onPauseResumeClicked()
{
    if (isPaused_) {
        // Resume
        isPaused_ = false;
        pauseRequested_ = false;
        pauseResumeButton_->setText("Pause");
        AppendLog("Resumed");
        emit resumeClicked();
    } else {
        // Pause
        isPaused_ = true;
        pauseRequested_ = true;
        pauseResumeButton_->setText("Resume");
        AppendLog("Pause requested...");
        emit pauseClicked();
    }
}

void ProgressWindow::onCancelClicked()
{
    if (cancelButton_->text() == "Close") {
        accept();
        return;
    }

    cancelRequested_ = true;
    AppendLog("Cancel requested...");
    cancelButton_->setEnabled(false);
    emit cancelClicked();
}
