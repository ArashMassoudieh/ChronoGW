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
    , fitnessAutoScale_(true)
    , mcmcAutoScale_(true)
    , pauseRequested_(false)
    , cancelRequested_(false)
    , isPaused_(false)
{
    setWindowTitle(title);
    setMinimumSize(800, 600);

    setupUI();
    createFitnessChart();
    createMCMCChart();

    // Default: Show fitness chart for GA, hide MCMC
    setFitnessChartVisible(true);
    setMCMCChartVisible(false);
    setSecondaryProgressVisible(false);
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

    // Main progress bar
    progressLabel_ = new QLabel("Progress:", this);
    mainLayout->addWidget(progressLabel_);

    mainProgressBar_ = new QProgressBar(this);
    mainProgressBar_->setRange(0, 1000);
    mainProgressBar_->setValue(0);
    mainProgressBar_->setTextVisible(true);
    mainProgressBar_->setFormat("%p%");
    mainLayout->addWidget(mainProgressBar_);

    // Secondary progress bar (initially hidden) - MOVED HERE
    secondaryProgressLabel_ = new QLabel("Secondary Progress:", this);
    mainLayout->addWidget(secondaryProgressLabel_);

    secondaryProgressBar_ = new QProgressBar(this);
    secondaryProgressBar_->setRange(0, 1000);
    secondaryProgressBar_->setValue(0);
    secondaryProgressBar_->setTextVisible(true);
    secondaryProgressBar_->setFormat("%p%");
    mainLayout->addWidget(secondaryProgressBar_);

    // Fitness chart placeholder (will be created later)
    fitnessChartView_ = nullptr;  // Created in createFitnessChart()

    // MCMC chart placeholder (will be created later)
    mcmcChartView_ = nullptr;  // Created in createMCMCChart()

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


void ProgressWindow::createFitnessChart()
{
    // Create chart
    fitnessChart_ = new QChart();
    fitnessChart_->setTitle("Best Fitness per Generation");
    fitnessChart_->setAnimationOptions(QChart::NoAnimation);
    fitnessChart_->legend()->hide();

    // Create line series
    fitnessSeries_ = new QLineSeries();
    fitnessSeries_->setName("Best Fitness");

    // Set line pen
    QPen pen(Qt::blue);
    pen.setWidth(2);

    // Create area series for filling
    fitnessAreaSeries_ = new QAreaSeries(fitnessSeries_);
    fitnessAreaSeries_->setName("Best Fitness");

    // Set fill color with transparency
    QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
    gradient.setColorAt(0.0, QColor(0, 0, 255, 180));    // Blue at top
    gradient.setColorAt(1.0, QColor(0, 0, 255, 40));     // Light blue at bottom
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    fitnessAreaSeries_->setBrush(gradient);
    fitnessAreaSeries_->setPen(pen);

    // Add area series to chart
    fitnessChart_->addSeries(fitnessAreaSeries_);

    // Create axes
    fitnessAxisX_ = new QValueAxis();
    fitnessAxisX_->setTitleText("Generation");
    fitnessAxisX_->setLabelFormat("%d");
    fitnessAxisX_->setRange(0, 100);
    fitnessChart_->addAxis(fitnessAxisX_, Qt::AlignBottom);
    fitnessAreaSeries_->attachAxis(fitnessAxisX_);

    fitnessAxisY_ = new QValueAxis();
    fitnessAxisY_->setTitleText("Fitness");
    fitnessAxisY_->setLabelFormat("%.2e");
    fitnessAxisY_->setRange(0, 1);
    fitnessChart_->addAxis(fitnessAxisY_, Qt::AlignLeft);
    fitnessAreaSeries_->attachAxis(fitnessAxisY_);

    // Create chart view
    fitnessChartView_ = new QChartView(fitnessChart_, this);
    fitnessChartView_->setRenderHint(QPainter::Antialiasing);
    fitnessChartView_->setMinimumHeight(250);

    // Add to layout (after secondary progress bar)
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->insertWidget(4, fitnessChartView_);
    }
}

void ProgressWindow::createMCMCChart()
{
    // Create chart
    mcmcChart_ = new QChart();
    mcmcChart_->setTitle("Log Posterior");
    mcmcChart_->setAnimationOptions(QChart::NoAnimation);
    mcmcChart_->legend()->hide();

    // Create line series
    mcmcSeries_ = new QLineSeries();
    mcmcSeries_->setName("Log Posterior");

    // Set line pen
    QPen pen(Qt::red);
    pen.setWidth(2);

    // Create area series for filling
    mcmcAreaSeries_ = new QAreaSeries(mcmcSeries_);
    mcmcAreaSeries_->setName("Log Posterior");

    // Set fill color with transparency
    QLinearGradient gradient(QPointF(0, 0), QPointF(0, 1));
    gradient.setColorAt(0.0, QColor(255, 0, 0, 180));    // Red at top
    gradient.setColorAt(1.0, QColor(255, 0, 0, 40));     // Light red at bottom
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    mcmcAreaSeries_->setBrush(gradient);
    mcmcAreaSeries_->setPen(pen);

    // Add area series to chart
    mcmcChart_->addSeries(mcmcAreaSeries_);

    // Create axes
    mcmcAxisX_ = new QValueAxis();
    mcmcAxisX_->setTitleText("Sample");
    mcmcAxisX_->setLabelFormat("%d");
    mcmcAxisX_->setRange(0, 1000);
    mcmcChart_->addAxis(mcmcAxisX_, Qt::AlignBottom);
    mcmcAreaSeries_->attachAxis(mcmcAxisX_);

    mcmcAxisY_ = new QValueAxis();
    mcmcAxisY_->setTitleText("Log Posterior");
    mcmcAxisY_->setLabelFormat("%.2f");
    mcmcAxisY_->setRange(-1000, 0);
    mcmcChart_->addAxis(mcmcAxisY_, Qt::AlignLeft);
    mcmcAreaSeries_->attachAxis(mcmcAxisY_);

    // Create chart view
    mcmcChartView_ = new QChartView(mcmcChart_, this);
    mcmcChartView_->setRenderHint(QPainter::Antialiasing);
    mcmcChartView_->setMinimumHeight(250);

    // Add to layout (after fitness chart)
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->insertWidget(5, mcmcChartView_);
    }
}
// ============================================================================
// Progress Bar Methods
// ============================================================================

void ProgressWindow::setProgress(double progress)
{
    int value = static_cast<int>(progress * 1000);
    mainProgressBar_->setValue(value);
    QApplication::processEvents();
}

void ProgressWindow::setSecondaryProgress(double progress)
{
    int value = static_cast<int>(progress * 1000);
    secondaryProgressBar_->setValue(value);
    QApplication::processEvents();
}

void ProgressWindow::setSecondaryProgressVisible(bool visible)
{
    secondaryProgressLabel_->setVisible(visible);
    secondaryProgressBar_->setVisible(visible);
}

void ProgressWindow::setProgressLabel(const QString& label)
{
    progressLabel_->setText(label);
}

void ProgressWindow::setSecondaryProgressLabel(const QString& label)
{
    secondaryProgressLabel_->setText(label);
}

// ============================================================================
// Status and Logging Methods
// ============================================================================

void ProgressWindow::setStatus(const QString& status)
{
    statusLabel_->setText(status);
    QApplication::processEvents();
}

void ProgressWindow::appendLog(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logTextEdit_->append(QString("[%1] %2").arg(timestamp).arg(message));

    // Auto-scroll to bottom
    QTextCursor cursor = logTextEdit_->textCursor();
    cursor.movePosition(QTextCursor::End);
    logTextEdit_->setTextCursor(cursor);

    QApplication::processEvents();
}

void ProgressWindow::clearLog()
{
    logTextEdit_->clear();
}

// ============================================================================
// Information Panel Methods
// ============================================================================

void ProgressWindow::appendInfo(const QString& text)
{
    infoTextEdit_->append(text);

    // Auto-scroll to bottom
    QTextCursor cursor = infoTextEdit_->textCursor();
    cursor.movePosition(QTextCursor::End);
    infoTextEdit_->setTextCursor(cursor);

    QApplication::processEvents();
}

void ProgressWindow::setInfoText(const QString& text)
{
    infoTextEdit_->setPlainText(text);
    QApplication::processEvents();
}

void ProgressWindow::clearInfo()
{
    infoTextEdit_->clear();
}

void ProgressWindow::setInfoPanelVisible(bool visible)
{
    infoPanelLabel_->setVisible(visible);
    infoTextEdit_->setVisible(visible);
}

void ProgressWindow::setInfoPanelLabel(const QString& label)
{
    infoPanelLabel_->setText(label);
}

// ============================================================================
// Fitness Chart Methods
// ============================================================================

void ProgressWindow::addFitnessPoint(int generation, double fitness)
{
    fitnessData_.append(QPointF(generation, fitness));
    updateFitnessChart();
}

void ProgressWindow::clearFitnessData()
{
    fitnessData_.clear();
    fitnessSeries_->clear();
}

void ProgressWindow::setFitnessYRange(double min, double max)
{
    fitnessAutoScale_ = false;
    fitnessAxisY_->setRange(min, max);
}

void ProgressWindow::setFitnessAutoScale(bool enabled)
{
    fitnessAutoScale_ = enabled;
}

void ProgressWindow::setFitnessChartVisible(bool visible)
{
    if (fitnessChartView_) {
        fitnessChartView_->setVisible(visible);
    }
}

void ProgressWindow::setFitnessChartTitle(const QString& title)
{
    if (fitnessChart_) {
        fitnessChart_->setTitle(title);
    }
}

void ProgressWindow::updateFitnessChart()
{
    if (fitnessData_.isEmpty()) {
        return;
    }

    // Update series
    fitnessSeries_->replace(fitnessData_);

    // Auto-scale if enabled
    if (fitnessAutoScale_ && !fitnessData_.isEmpty()) {
        // Find min/max
        double minVal = fitnessData_[0].y();
        double maxVal = fitnessData_[0].y();
        int maxGen = fitnessData_[0].x();

        for (const QPointF& point : fitnessData_) {
            minVal = qMin(minVal, point.y());
            maxVal = qMax(maxVal, point.y());
            maxGen = qMax(maxGen, static_cast<int>(point.x()));
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

        fitnessAxisY_->setRange(minVal, maxVal);

    }

    QApplication::processEvents();
}

// ============================================================================
// MCMC Chart Methods
// ============================================================================

void ProgressWindow::addMCMCPoint(int sample, double logPosterior)
{
    mcmcData_.append(QPointF(sample, logPosterior));
    updateMCMCChart();
}

void ProgressWindow::clearMCMCData()
{
    mcmcData_.clear();
    mcmcSeries_->clear();
}

void ProgressWindow::setMCMCYRange(double min, double max)
{
    mcmcAutoScale_ = false;
    mcmcAxisY_->setRange(min, max);
}

void ProgressWindow::setMCMCAutoScale(bool enabled)
{
    mcmcAutoScale_ = enabled;
}

void ProgressWindow::setMCMCChartVisible(bool visible)
{
    if (mcmcChartView_) {
        mcmcChartView_->setVisible(visible);
    }
}

void ProgressWindow::setMCMCChartTitle(const QString& title)
{
    if (mcmcChart_) {
        mcmcChart_->setTitle(title);
    }
}

void ProgressWindow::updateMCMCChart()
{
    if (mcmcData_.isEmpty()) {
        return;
    }

    // Update series
    mcmcSeries_->replace(mcmcData_);

    // Auto-scale if enabled
    if (mcmcAutoScale_ && !mcmcData_.isEmpty()) {
        // Find min/max
        double minVal = mcmcData_[0].y();
        double maxVal = mcmcData_[0].y();
        int maxSample = mcmcData_[0].x();

        for (const QPointF& point : mcmcData_) {
            minVal = qMin(minVal, point.y());
            maxVal = qMax(maxVal, point.y());
            maxSample = qMax(maxSample, static_cast<int>(point.x()));
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

        mcmcAxisY_->setRange(minVal, maxVal);
        mcmcAxisX_->setRange(0, maxSample + 100);
    }

    QApplication::processEvents();
}

// ============================================================================
// Control Methods
// ============================================================================

void ProgressWindow::setPauseEnabled(bool enabled)
{
    pauseResumeButton_->setEnabled(enabled);
}

void ProgressWindow::setComplete(const QString& message)
{
    setStatus(message);
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
        appendLog("Resumed");
        emit resumeClicked();
    } else {
        // Pause
        isPaused_ = true;
        pauseRequested_ = true;
        pauseResumeButton_->setText("Resume");
        appendLog("Pause requested...");
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
    appendLog("Cancel requested...");
    cancelButton_->setEnabled(false);
    emit cancelClicked();
}

void ProgressWindow::setFitnessXRange(double min, double max)
{
    if (fitnessAxisX_) {
        fitnessAxisX_->setRange(min, max);
    }
}
