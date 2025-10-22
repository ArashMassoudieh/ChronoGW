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

#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QVector>
#include <QtCharts/QAreaSeries>

    /**
 * @class ProgressWindow
 * @brief Generic progress window for optimization and MCMC algorithms
 *
 * Features:
 * - Two progress bars (main and optional secondary)
 * - Two charts (fitness for GA, posterior for MCMC)
 * - Real-time status updates
 * - Scrolling log area
 * - Pause/Resume/Cancel buttons
 */
    class ProgressWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     * @param title Window title (e.g., "GA Optimization" or "MCMC Sampling")
     */
    explicit ProgressWindow(QWidget* parent = nullptr, const QString& title = "Progress");

    /**
     * @brief Destructor
     */
    ~ProgressWindow();

    // ========================================================================
    // Progress Bar Methods
    // ========================================================================

    /**
     * @brief Set main progress (0.0 to 1.0)
     * @param progress Progress value between 0 and 1
     */
    void setProgress(double progress);

    /**
     * @brief Set secondary progress (0.0 to 1.0)
     * @param progress Progress value between 0 and 1
     */
    void setSecondaryProgress(double progress);

    /**
     * @brief Show or hide the secondary progress bar
     * @param visible True to show, false to hide
     */
    void setSecondaryProgressVisible(bool visible);

    /**
     * @brief Set label for main progress bar
     * @param label Text label (e.g., "Generation Progress")
     */
    void setProgressLabel(const QString& label);

    /**
     * @brief Set label for secondary progress bar
     * @param label Text label (e.g., "Enhancement Progress")
     */
    void setSecondaryProgressLabel(const QString& label);

    // ========================================================================
    // Status and Logging Methods
    // ========================================================================

    /**
     * @brief Update status message
     * @param status Status text to display
     */
    void setStatus(const QString& status);

    /**
     * @brief Append message to log
     * @param message Message to append
     */
    void appendLog(const QString& message);

    /**
     * @brief Clear the log
     */
    void clearLog();

    // ========================================================================
    // Information Panel Methods
    // ========================================================================

    /**
     * @brief Append text to information panel
     * @param text Text to append
     */
    void appendInfo(const QString& text);

    /**
     * @brief Set text in information panel (replaces all content)
     * @param text Text to display
     */
    void setInfoText(const QString& text);

    /**
     * @brief Clear information panel
     */
    void clearInfo();

    /**
     * @brief Show or hide information panel
     * @param visible True to show, false to hide
     */
    void setInfoPanelVisible(bool visible);

    /**
     * @brief Set information panel label
     * @param label Label text
     */
    void setInfoPanelLabel(const QString& label);

    // ========================================================================
    // Chart Methods - Fitness (for GA)
    // ========================================================================

    /**
     * @brief Add data point to fitness chart
     * @param generation Generation number (x-axis)
     * @param fitness Best fitness value (y-axis)
     */
    void addFitnessPoint(int generation, double fitness);

    /**
     * @brief Clear all fitness data
     */
    void clearFitnessData();

    /**
     * @brief Set Y-axis range for fitness chart
     * @param min Minimum value
     * @param max Maximum value
     */
    void setFitnessYRange(double min, double max);

    /**
     * @brief Enable auto-scaling for fitness Y-axis
     * @param enabled True to enable auto-scaling
     */
    void setFitnessAutoScale(bool enabled);

    /**
     * @brief Show or hide fitness chart
     * @param visible True to show, false to hide
     */
    void setFitnessChartVisible(bool visible);

    /**
     * @brief Set fitness chart title
     * @param title Chart title
     */
    void setFitnessChartTitle(const QString& title);

    // ========================================================================
    // Chart Methods - MCMC (for posterior distribution)
    // ========================================================================

    /**
     * @brief Add data point to MCMC chart
     * @param sample Sample number (x-axis)
     * @param logPosterior Log posterior value (y-axis)
     */
    void addMCMCPoint(int sample, double logPosterior);

    /**
     * @brief Clear all MCMC data
     */
    void clearMCMCData();

    /**
     * @brief Set Y-axis range for MCMC chart
     * @param min Minimum value
     * @param max Maximum value
     */
    void setMCMCYRange(double min, double max);

    /**
     * @brief Enable auto-scaling for MCMC Y-axis
     * @param enabled True to enable auto-scaling
     */
    void setMCMCAutoScale(bool enabled);

    /**
     * @brief Show or hide MCMC chart
     * @param visible True to show, false to hide
     */
    void setMCMCChartVisible(bool visible);

    /**
     * @brief Set MCMC chart title
     * @param title Chart title
     */
    void setMCMCChartTitle(const QString& title);

    // ========================================================================
    // Control Methods
    // ========================================================================

    /**
     * @brief Check if user requested pause
     * @return True if pause requested
     */
    bool isPauseRequested() const { return pauseRequested_; }

    /**
     * @brief Check if user requested cancel
     * @return True if cancel requested
     */
    bool isCancelRequested() const { return cancelRequested_; }

    /**
     * @brief Reset pause request (after handling)
     */
    void resetPauseRequest() { pauseRequested_ = false; }

    /**
     * @brief Enable or disable pause button
     * @param enabled True to enable
     */
    void setPauseEnabled(bool enabled);

    /**
     * @brief Show completion message
     * @param message Completion message
     */
    void setComplete(const QString& message = "Complete!");

    /**
     * @brief Set X-axis range for fitness chart
     * @param min Minimum value
     * @param max Maximum value
     */
    void setFitnessXRange(double min, double max);

signals:
    /**
     * @brief Emitted when user clicks pause
     */
    void pauseClicked();

    /**
     * @brief Emitted when user clicks resume
     */
    void resumeClicked();

    /**
     * @brief Emitted when user clicks cancel
     */
    void cancelClicked();

private slots:
    void onPauseResumeClicked();
    void onCancelClicked();

private:
    void setupUI();
    void createFitnessChart();
    void createMCMCChart();
    void updateFitnessChart();
    void updateMCMCChart();

    // UI Components
    QLabel* statusLabel_;
    QLabel* progressLabel_;
    QLabel* secondaryProgressLabel_;
    QProgressBar* mainProgressBar_;
    QProgressBar* secondaryProgressBar_;
    QTextEdit* logTextEdit_;

    // Information Panel
    QLabel* infoPanelLabel_;
    QTextEdit* infoTextEdit_;

    QPushButton* pauseResumeButton_;
    QPushButton* cancelButton_;

    QChart* fitnessChart_;
    QChartView* fitnessChartView_;
    QLineSeries* fitnessSeries_;
    QAreaSeries* fitnessAreaSeries_;  // ADD THIS
    QValueAxis* fitnessAxisX_;
    QValueAxis* fitnessAxisY_;
    bool fitnessAutoScale_;

    // MCMC Chart (for posterior)
    QChart* mcmcChart_;
    QChartView* mcmcChartView_;
    QLineSeries* mcmcSeries_;
    QAreaSeries* mcmcAreaSeries_;  // ADD THIS
    QValueAxis* mcmcAxisX_;
    QValueAxis* mcmcAxisY_;
    bool mcmcAutoScale_;

    // Data storage
    QVector<QPointF> fitnessData_;
    QVector<QPointF> mcmcData_;

    // State
    bool pauseRequested_;
    bool cancelRequested_;
    bool isPaused_;
};

#endif // PROGRESSWINDOW_H
