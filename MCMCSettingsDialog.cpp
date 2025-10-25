/*
 * ChronoGW - Groundwater Age Modeling
 * Copyright (C) 2025
 */

#include "MCMCSettingsDialog.h"
#include "GWA.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <limits>

MCMCSettingsDialog::MCMCSettingsDialog(CMCMC<CGWA>* mcmc, QWidget* parent)
    : QDialog(parent)
    , mcmc_(mcmc)
{
    setWindowTitle("MCMC Settings");
    setModal(true);
    resize(600, 700);

    setupUI();
    loadSettings();

    // Connect signals
    connect(okButton_, &QPushButton::clicked, this, &MCMCSettingsDialog::onAccepted);
    connect(cancelButton_, &QPushButton::clicked, this, &MCMCSettingsDialog::onRejected);
    connect(browseContinueFileButton_, &QPushButton::clicked, this, &MCMCSettingsDialog::onBrowseContinueFile);
    connect(continueMCMCCheckBox_, &QCheckBox::toggled, [this](bool checked) {
        continueFileLineEdit_->setEnabled(checked);
        browseContinueFileButton_->setEnabled(checked);
    });
}

MCMCSettingsDialog::~MCMCSettingsDialog()
{
}

void MCMCSettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Add group boxes
    mainLayout->addWidget(createSampleGroup());
    mainLayout->addWidget(createPerturbationGroup());
    mainLayout->addWidget(createAlgorithmGroup());
    mainLayout->addWidget(createPostProcessingGroup());
    mainLayout->addWidget(createPerformanceGroup());

    // Add stretch to push everything to the top
    mainLayout->addStretch();

    // Buttons at bottom
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    okButton_ = new QPushButton("OK", this);
    cancelButton_ = new QPushButton("Cancel", this);

    okButton_->setDefault(true);

    buttonLayout->addWidget(okButton_);
    buttonLayout->addWidget(cancelButton_);

    mainLayout->addLayout(buttonLayout);
}

QGroupBox* MCMCSettingsDialog::createSampleGroup()
{
    QGroupBox* group = new QGroupBox("Sample Configuration", this);
    QFormLayout* layout = new QFormLayout(group);

    // Total samples
    samplesSpinBox_ = new QSpinBox(this);
    samplesSpinBox_->setRange(100, 10000000);
    samplesSpinBox_->setValue(10000);
    samplesSpinBox_->setSingleStep(1000);
    samplesSpinBox_->setToolTip("Total number of MCMC samples to generate");
    layout->addRow("Number of Samples:", samplesSpinBox_);

    // Number of chains
    chainsSpinBox_ = new QSpinBox(this);
    chainsSpinBox_->setRange(1, 100);
    chainsSpinBox_->setValue(4);
    chainsSpinBox_->setToolTip("Number of parallel MCMC chains");
    layout->addRow("Number of Chains:", chainsSpinBox_);

    // Burnout samples
    burnoutSpinBox_ = new QSpinBox(this);
    burnoutSpinBox_->setRange(0, 1000000);
    burnoutSpinBox_->setValue(1000);
    burnoutSpinBox_->setSingleStep(100);
    burnoutSpinBox_->setToolTip("Number of initial samples to discard (burn-in period)");
    layout->addRow("Burnout Samples:", burnoutSpinBox_);

    // Save interval
    saveIntervalSpinBox_ = new QSpinBox(this);
    saveIntervalSpinBox_->setRange(1, 1000);
    saveIntervalSpinBox_->setValue(1);
    saveIntervalSpinBox_->setToolTip("Save every nth sample (thinning interval)");
    layout->addRow("Save Interval:", saveIntervalSpinBox_);

    return group;
}

QGroupBox* MCMCSettingsDialog::createPerturbationGroup()
{
    QGroupBox* group = new QGroupBox("Perturbation Settings", this);
    QFormLayout* layout = new QFormLayout(group);

    // Perturbation factor
    perturbationFactorSpinBox_ = new QDoubleSpinBox(this);
    perturbationFactorSpinBox_->setRange(0.001, 1.0);
    perturbationFactorSpinBox_->setValue(0.05);
    perturbationFactorSpinBox_->setDecimals(4);
    perturbationFactorSpinBox_->setSingleStep(0.01);
    perturbationFactorSpinBox_->setToolTip("Standard perturbation magnitude during sampling");
    layout->addRow("Perturbation Factor:", perturbationFactorSpinBox_);

    // Perturbation scale
    perturbationScaleSpinBox_ = new QDoubleSpinBox(this);
    perturbationScaleSpinBox_->setRange(0.1, 1.0);
    perturbationScaleSpinBox_->setValue(0.75);
    perturbationScaleSpinBox_->setDecimals(2);
    perturbationScaleSpinBox_->setSingleStep(0.05);
    perturbationScaleSpinBox_->setToolTip("Scaling factor for adaptive perturbation adjustment");
    layout->addRow("Perturbation Scale:", perturbationScaleSpinBox_);

    // Target acceptance rate
    acceptanceRateSpinBox_ = new QDoubleSpinBox(this);
    acceptanceRateSpinBox_->setRange(0.1, 0.5);
    acceptanceRateSpinBox_->setValue(0.234);
    acceptanceRateSpinBox_->setDecimals(3);
    acceptanceRateSpinBox_->setSingleStep(0.01);
    acceptanceRateSpinBox_->setToolTip("Target acceptance rate for adaptive MCMC (optimal ≈ 0.234)");
    layout->addRow("Target Acceptance Rate:", acceptanceRateSpinBox_);

    return group;
}

QGroupBox* MCMCSettingsDialog::createAlgorithmGroup()
{
    QGroupBox* group = new QGroupBox("Algorithm Options", this);
    QVBoxLayout* layout = new QVBoxLayout(group);

    // No initial perturbation
    noInitialPerturbationCheckBox_ = new QCheckBox("Skip initial perturbation", this);
    noInitialPerturbationCheckBox_->setToolTip("Start chains from exact nominal values without perturbation");
    layout->addWidget(noInitialPerturbationCheckBox_);

    // Sensitivity-based perturbation
    sensitivityBasedCheckBox_ = new QCheckBox("Use sensitivity-based perturbation", this);
    sensitivityBasedCheckBox_->setToolTip("Scale perturbations based on parameter sensitivities");
    layout->addWidget(sensitivityBasedCheckBox_);

    // Global sensitivity
    globalSensitivityCheckBox_ = new QCheckBox("Calculate global sensitivity", this);
    globalSensitivityCheckBox_->setToolTip("Perform global sensitivity analysis after sampling");
    layout->addWidget(globalSensitivityCheckBox_);

    // Continue MCMC
    continueMCMCCheckBox_ = new QCheckBox("Continue from previous run", this);
    continueMCMCCheckBox_->setToolTip("Continue MCMC from a previous output file");
    layout->addWidget(continueMCMCCheckBox_);

    // Continue file
    QHBoxLayout* continueLayout = new QHBoxLayout();
    QLabel* continueLabel = new QLabel("Continue File:", this);
    continueFileLineEdit_ = new QLineEdit(this);
    continueFileLineEdit_->setEnabled(false);
    continueFileLineEdit_->setToolTip("File to continue MCMC from");
    browseContinueFileButton_ = new QPushButton("Browse...", this);
    browseContinueFileButton_->setEnabled(false);

    continueLayout->addWidget(continueLabel);
    continueLayout->addWidget(continueFileLineEdit_, 1);
    continueLayout->addWidget(browseContinueFileButton_);

    layout->addLayout(continueLayout);

    return group;
}

QGroupBox* MCMCSettingsDialog::createPostProcessingGroup()
{
    QGroupBox* group = new QGroupBox("Post-Processing", this);
    QFormLayout* layout = new QFormLayout(group);

    // Number of realizations
    realizationsSpinBox_ = new QSpinBox(this);
    realizationsSpinBox_->setRange(0, 10000);
    realizationsSpinBox_->setValue(0);
    realizationsSpinBox_->setSingleStep(100);
    realizationsSpinBox_->setToolTip("Number of model realizations to generate after sampling");
    layout->addRow("Post-Estimate Realizations:", realizationsSpinBox_);

    // Sensitivity increment
    sensitivityIncrementSpinBox_ = new QDoubleSpinBox(this);
    sensitivityIncrementSpinBox_->setRange(0.0001, 1.0);
    sensitivityIncrementSpinBox_->setValue(0.01);
    sensitivityIncrementSpinBox_->setDecimals(4);
    sensitivityIncrementSpinBox_->setSingleStep(0.001);
    sensitivityIncrementSpinBox_->setToolTip("Increment for sensitivity analysis calculations");
    layout->addRow("Sensitivity Increment:", sensitivityIncrementSpinBox_);

    // Noise in realizations
    noiseRealizationCheckBox_ = new QCheckBox("Add noise to realizations", this);
    noiseRealizationCheckBox_->setToolTip("Include observational noise in realization outputs");
    layout->addRow("", noiseRealizationCheckBox_);

    return group;
}

QGroupBox* MCMCSettingsDialog::createPerformanceGroup()
{
    QGroupBox* group = new QGroupBox("Performance", this);
    QFormLayout* layout = new QFormLayout(group);

    // Number of threads
    threadsSpinBox_ = new QSpinBox(this);
    threadsSpinBox_->setRange(1, 128);
    threadsSpinBox_->setValue(8);
    threadsSpinBox_->setToolTip("Number of parallel threads for computation");
    layout->addRow("Number of Threads:", threadsSpinBox_);

    return group;
}

void MCMCSettingsDialog::loadSettings()
{
    // Load current settings from MCMC object
    const MCMCSettings& settings = mcmc_->GetSettings();

    // Sample Configuration
    samplesSpinBox_->setValue(settings.total_number_of_samples);
    chainsSpinBox_->setValue(settings.number_of_chains);
    burnoutSpinBox_->setValue(settings.burnout_samples);
    saveIntervalSpinBox_->setValue(settings.save_interval);

    // Perturbation Settings
    perturbationFactorSpinBox_->setValue(settings.perturbation_factor);
    perturbationScaleSpinBox_->setValue(settings.perturbation_change_scale);
    acceptanceRateSpinBox_->setValue(settings.acceptance_rate);

    // Algorithm Options
    noInitialPerturbationCheckBox_->setChecked(settings.no_initial_perturbation);
    sensitivityBasedCheckBox_->setChecked(settings.sensitivity_based_perturbation);
    globalSensitivityCheckBox_->setChecked(settings.global_sensitivity);
    continueMCMCCheckBox_->setChecked(settings.continue_mcmc);

    if (!settings.continue_filename.empty()) {
        continueFileLineEdit_->setText(QString::fromStdString(settings.continue_filename));
    }

    // Post-processing
    realizationsSpinBox_->setValue(settings.number_of_post_estimate_realizations);
    sensitivityIncrementSpinBox_->setValue(settings.increment_for_sensitivity);
    noiseRealizationCheckBox_->setChecked(settings.noise_realization_writeout);

    // Performance
    threadsSpinBox_->setValue(settings.numberOfThreads);
}

void MCMCSettingsDialog::onAccepted()
{
    // Validate inputs
    if (samplesSpinBox_->value() < burnoutSpinBox_->value()) {
        QMessageBox::warning(this, "Invalid Settings",
                             "Number of samples must be greater than burnout samples.");
        return;
    }

    if (continueMCMCCheckBox_->isChecked() && continueFileLineEdit_->text().isEmpty()) {
        QMessageBox::warning(this, "Invalid Settings",
                             "Please specify a file to continue from.");
        return;
    }

    // Save all settings to MCMC object
    mcmc_->SetProperty("number_of_samples", std::to_string(samplesSpinBox_->value()));
    mcmc_->SetProperty("number_of_chains", std::to_string(chainsSpinBox_->value()));
    mcmc_->SetProperty("number_of_burnout_samples", std::to_string(burnoutSpinBox_->value()));
    mcmc_->SetProperty("record_interval", std::to_string(saveIntervalSpinBox_->value()));

    mcmc_->SetProperty("perturbation_factor", std::to_string(perturbationFactorSpinBox_->value()));
    mcmc_->SetProperty("perturbation_change_scale", std::to_string(perturbationScaleSpinBox_->value()));
    mcmc_->SetProperty("acceptance_rate", std::to_string(acceptanceRateSpinBox_->value()));

    // Note: checkbox is "Skip initial perturbation", but property is "initial_perturbation"
    // If checkbox is CHECKED (skip=true) → send "no" (don't do initial perturbation)
    // If checkbox is UNCHECKED (skip=false) → send "yes" (do initial perturbation)
    mcmc_->SetProperty("initial_perturbation", noInitialPerturbationCheckBox_->isChecked() ? "no" : "yes");
    mcmc_->SetProperty("sensitivity_based_perturbation", sensitivityBasedCheckBox_->isChecked() ? "yes" : "no");
    mcmc_->SetProperty("perform_global_sensitivity", globalSensitivityCheckBox_->isChecked() ? "yes" : "no");

    if (continueMCMCCheckBox_->isChecked()) {
        mcmc_->SetProperty("continue_based_on_file_name", continueFileLineEdit_->text().toStdString());
    }

    mcmc_->SetProperty("number_of_post_estimate_realizations", std::to_string(realizationsSpinBox_->value()));
    mcmc_->SetProperty("increment_for_sensitivity_analysis", std::to_string(sensitivityIncrementSpinBox_->value()));
    mcmc_->SetProperty("add_noise_to_realizations", noiseRealizationCheckBox_->isChecked() ? "yes" : "no");

    mcmc_->SetProperty("number_of_threads", std::to_string(threadsSpinBox_->value()));

    accept();
}

void MCMCSettingsDialog::onRejected()
{
    reject();
}

void MCMCSettingsDialog::onBrowseContinueFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select MCMC Continue File"),
        QString(),
        tr("Text Files (*.txt);;All Files (*)")
        );

    if (!fileName.isEmpty()) {
        continueFileLineEdit_->setText(fileName);
    }
}
