/*
 * ChronoGW - Groundwater Age Modeling
 * Copyright (C) 2025
 */

#ifndef MCMCSETTINGSDIALOG_H
#define MCMCSETTINGSDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include "MCMC.h"

// Forward declaration
template<class T> class CMCMC;
class CGWA;

/**
 * @class MCMCSettingsDialog
 * @brief Dialog window for configuring MCMC (Markov Chain Monte Carlo) parameters
 *
 * Provides a user-friendly interface for setting all MCMC parameters including:
 * - Sample and chain configuration
 * - Perturbation settings
 * - Output options
 * - Algorithm options
 * - Performance settings
 */
class MCMCSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param mcmc Pointer to MCMC object to configure
     * @param parent Parent widget
     */
    explicit MCMCSettingsDialog(CMCMC<CGWA>* mcmc, QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~MCMCSettingsDialog();

private slots:
    /**
     * @brief Handle OK button click
     *
     * Validates inputs and saves all settings to the MCMC object
     */
    void onAccepted();

    /**
     * @brief Handle Cancel button click
     */
    void onRejected();

    /**
     * @brief Handle browse button for continue file
     */
    void onBrowseContinueFile();

private:
    // MCMC object reference
    CMCMC<CGWA>* mcmc_;

    // Sample Configuration Widgets
    QSpinBox* samplesSpinBox_;
    QSpinBox* chainsSpinBox_;
    QSpinBox* burnoutSpinBox_;
    QSpinBox* saveIntervalSpinBox_;

    // Perturbation Settings Widgets
    QDoubleSpinBox* perturbationFactorSpinBox_;
    QDoubleSpinBox* perturbationScaleSpinBox_;
    QDoubleSpinBox* acceptanceRateSpinBox_;

    // Algorithm Options Widgets
    QCheckBox* noInitialPerturbationCheckBox_;
    QCheckBox* sensitivityBasedCheckBox_;
    QCheckBox* globalSensitivityCheckBox_;
    QCheckBox* continueMCMCCheckBox_;
    QLineEdit* continueFileLineEdit_;
    QPushButton* browseContinueFileButton_;

    // Post-processing Widgets
    QSpinBox* realizationsSpinBox_;
    QDoubleSpinBox* sensitivityIncrementSpinBox_;
    QCheckBox* noiseRealizationCheckBox_;

    // Performance Widgets
    QSpinBox* threadsSpinBox_;

    // Buttons
    QPushButton* okButton_;
    QPushButton* cancelButton_;

    /**
     * @brief Create and layout all UI components
     */
    void setupUI();

    /**
     * @brief Load current settings from MCMC object
     */
    void loadSettings();

    /**
     * @brief Create the sample configuration group
     */
    QGroupBox* createSampleGroup();

    /**
     * @brief Create the perturbation settings group
     */
    QGroupBox* createPerturbationGroup();

    /**
     * @brief Create the algorithm options group
     */
    QGroupBox* createAlgorithmGroup();

    /**
     * @brief Create the post-processing group
     */
    QGroupBox* createPostProcessingGroup();

    /**
     * @brief Create the performance group
     */
    QGroupBox* createPerformanceGroup();
};

#endif // MCMCSETTINGSDIALOG_H
