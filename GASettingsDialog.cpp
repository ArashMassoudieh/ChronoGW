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

#include "GASettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QToolTip>
#include <QApplication>
#include <QStyle>

GASettingsDialog::GASettingsDialog(CGA<CGWA>* ga, QWidget* parent)
    : QDialog(parent)
    , m_ga(ga)
{
    if (!m_ga) {
        throw std::invalid_argument("GASettingsDialog: GA pointer cannot be null");
    }

    setWindowTitle("Genetic Algorithm Settings");
    setMinimumWidth(600);

    setupUI();
    loadSettings();

    // Connect signals
    connect(m_okButton, &QPushButton::clicked, this, &GASettingsDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &GASettingsDialog::onReject);
    connect(m_browseInitialPopBtn, &QPushButton::clicked, this, &GASettingsDialog::browseInitialPopulation);
    connect(m_browseOutputPathBtn, &QPushButton::clicked, this, &GASettingsDialog::browseOutputPath);
    connect(m_browseInputFileBtn, &QPushButton::clicked, this, &GASettingsDialog::browseInputFile);
    connect(m_crossoverTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GASettingsDialog::onCrossoverTypeChanged);
    connect(m_rcgaCheckBox, &QCheckBox::toggled, this, &GASettingsDialog::onRCGAToggled);
    connect(m_resetButton, &QPushButton::clicked, this, [this]() {
        loadSettings();  // Reload original settings from GA
    });
}

GASettingsDialog::~GASettingsDialog()
{
}

void GASettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Create tab widget for organized settings
    QTabWidget* tabWidget = new QTabWidget(this);

    // Tab 1: Basic Settings
    QWidget* basicTab = new QWidget();
    QVBoxLayout* basicLayout = new QVBoxLayout(basicTab);
    basicLayout->addWidget(createPopulationGroup());
    basicLayout->addWidget(createGeneticOperatorsGroup());
    basicLayout->addStretch();
    tabWidget->addTab(basicTab, "Basic Settings");

    // Tab 2: Advanced Settings
    QWidget* advancedTab = new QWidget();
    QVBoxLayout* advancedLayout = new QVBoxLayout(advancedTab);
    advancedLayout->addWidget(createShakeOperatorGroup());
    advancedLayout->addWidget(createFitnessGroup());
    advancedLayout->addWidget(createEnhancementGroup());
    advancedLayout->addStretch();
    tabWidget->addTab(advancedTab, "Advanced Settings");

    // Tab 3: File Paths
    QWidget* filesTab = new QWidget();
    QVBoxLayout* filesLayout = new QVBoxLayout(filesTab);
    filesLayout->addWidget(createFilePathsGroup());
    filesLayout->addStretch();
    tabWidget->addTab(filesTab, "File Paths");

    mainLayout->addWidget(tabWidget);

    // Button box at bottom
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_resetButton = new QPushButton("Reset to Current", this);
    m_okButton = new QPushButton("OK", this);
    m_cancelButton = new QPushButton("Cancel", this);

    m_okButton->setDefault(true);

    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);
}

QGroupBox* GASettingsDialog::createPopulationGroup()
{
    QGroupBox* group = new QGroupBox("Population Parameters", this);
    QFormLayout* layout = new QFormLayout(group);

    // Population size
    m_maxPopSpinBox = new QSpinBox(this);
    m_maxPopSpinBox->setRange(10, 10000);
    m_maxPopSpinBox->setSingleStep(10);
    m_maxPopSpinBox->setToolTip("Number of individuals in each generation");
    layout->addRow("Population Size:", m_maxPopSpinBox);

    // Number of generations
    m_nGenSpinBox = new QSpinBox(this);
    m_nGenSpinBox->setRange(1, 10000);
    m_nGenSpinBox->setSingleStep(10);
    m_nGenSpinBox->setToolTip("Total number of generations to evolve");
    layout->addRow("Number of Generations:", m_nGenSpinBox);

    // Number of threads
    m_numThreadsSpinBox = new QSpinBox(this);
    m_numThreadsSpinBox->setRange(1, 128);
    m_numThreadsSpinBox->setSingleStep(1);
    m_numThreadsSpinBox->setToolTip("Number of parallel threads for fitness evaluation");
    layout->addRow("Number of Threads:", m_numThreadsSpinBox);

    return group;
}

QGroupBox* GASettingsDialog::createGeneticOperatorsGroup()
{
    QGroupBox* group = new QGroupBox("Genetic Operators", this);
    QFormLayout* layout = new QFormLayout(group);

    // Crossover probability
    m_pCrossSpinBox = new QDoubleSpinBox(this);
    m_pCrossSpinBox->setRange(0.0, 1.0);
    m_pCrossSpinBox->setSingleStep(0.05);
    m_pCrossSpinBox->setDecimals(3);
    m_pCrossSpinBox->setToolTip("Probability of applying crossover (0.0 to 1.0)");
    layout->addRow("Crossover Probability:", m_pCrossSpinBox);

    // Mutation probability
    m_pMutateSpinBox = new QDoubleSpinBox(this);
    m_pMutateSpinBox->setRange(0.0, 1.0);
    m_pMutateSpinBox->setSingleStep(0.01);
    m_pMutateSpinBox->setDecimals(4);
    m_pMutateSpinBox->setToolTip("Probability of mutation per bit (0.0 to 1.0)");
    layout->addRow("Mutation Probability:", m_pMutateSpinBox);

    // Crossover type
    m_crossoverTypeCombo = new QComboBox(this);
    m_crossoverTypeCombo->addItem("Multi-point Crossover", 1);
    m_crossoverTypeCombo->addItem("Two-point Crossover", 2);
    m_crossoverTypeCombo->setToolTip("Type of crossover operation to use");
    layout->addRow("Crossover Type:", m_crossoverTypeCombo);

    // RCGA checkbox
    m_rcgaCheckBox = new QCheckBox("Use Real-Coded GA", this);
    m_rcgaCheckBox->setToolTip("Use real-coded genetic algorithm with linear crossover");
    layout->addRow("", m_rcgaCheckBox);

    return group;
}

QGroupBox* GASettingsDialog::createShakeOperatorGroup()
{
    QGroupBox* group = new QGroupBox("Shake Operator", this);
    QFormLayout* layout = new QFormLayout(group);

    QLabel* descLabel = new QLabel("The shake operator applies small random perturbations to parameters", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    layout->addRow(descLabel);

    // Shake scale
    m_shakeScaleSpinBox = new QDoubleSpinBox(this);
    m_shakeScaleSpinBox->setRange(0.0, 1.0);
    m_shakeScaleSpinBox->setSingleStep(0.01);
    m_shakeScaleSpinBox->setDecimals(4);
    m_shakeScaleSpinBox->setToolTip("Initial shake scale (e.g., 0.05 = ±5% perturbation)");
    layout->addRow("Shake Scale:", m_shakeScaleSpinBox);

    // Shake scale reduction
    m_shakeScaleRedSpinBox = new QDoubleSpinBox(this);
    m_shakeScaleRedSpinBox->setRange(0.0, 1.0);
    m_shakeScaleRedSpinBox->setSingleStep(0.05);
    m_shakeScaleRedSpinBox->setDecimals(3);
    m_shakeScaleRedSpinBox->setToolTip("Reduction factor applied to shake scale each generation");
    layout->addRow("Shake Scale Reduction:", m_shakeScaleRedSpinBox);

    return group;
}

QGroupBox* GASettingsDialog::createFitnessGroup()
{
    QGroupBox* group = new QGroupBox("Fitness Scaling", this);
    QFormLayout* layout = new QFormLayout(group);

    QLabel* descLabel = new QLabel("Rank-based fitness scaling with exponent N: fitness = (1/rank)^N", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    layout->addRow(descLabel);

    // Fitness exponent
    m_fitnessExpSpinBox = new QDoubleSpinBox(this);
    m_fitnessExpSpinBox->setRange(0.1, 10.0);
    m_fitnessExpSpinBox->setSingleStep(0.1);
    m_fitnessExpSpinBox->setDecimals(2);
    m_fitnessExpSpinBox->setToolTip("Exponent for rank-based fitness scaling (default: 1.0)");
    layout->addRow("Fitness Exponent (N):", m_fitnessExpSpinBox);

    return group;
}

QGroupBox* GASettingsDialog::createEnhancementGroup()
{
    QGroupBox* group = new QGroupBox("Enhancement/Restart", this);
    QFormLayout* layout = new QFormLayout(group);

    QLabel* descLabel = new QLabel("Number of times to restart the optimization with the best solution", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    layout->addRow(descLabel);

    // Number of enhancements
    m_numEnhancementsSpinBox = new QSpinBox(this);
    m_numEnhancementsSpinBox->setRange(0, 100);
    m_numEnhancementsSpinBox->setSingleStep(1);
    m_numEnhancementsSpinBox->setToolTip("Number of enhancement cycles (0 = no enhancements)");
    layout->addRow("Number of Enhancements:", m_numEnhancementsSpinBox);

    return group;
}

QGroupBox* GASettingsDialog::createFilePathsGroup()
{
    QGroupBox* group = new QGroupBox("File Paths", this);
    QGridLayout* layout = new QGridLayout(group);

    int row = 0;

    // Initial population file
    QLabel* initialPopLabel = new QLabel("Initial Population File:", this);
    m_initialPopEdit = new QLineEdit(this);
    m_initialPopEdit->setPlaceholderText("Optional: Load initial population from file");
    m_browseInitialPopBtn = new QPushButton("Browse...", this);
    layout->addWidget(initialPopLabel, row, 0);
    layout->addWidget(m_initialPopEdit, row, 1);
    layout->addWidget(m_browseInitialPopBtn, row, 2);
    row++;

    // Output path
    QLabel* outputPathLabel = new QLabel("Output Directory:", this);
    m_outputPathEdit = new QLineEdit(this);
    m_outputPathEdit->setPlaceholderText("Directory for output files");
    m_browseOutputPathBtn = new QPushButton("Browse...", this);
    layout->addWidget(outputPathLabel, row, 0);
    layout->addWidget(m_outputPathEdit, row, 1);
    layout->addWidget(m_browseOutputPathBtn, row, 2);
    row++;

    // Input results file
    QLabel* inputFileLabel = new QLabel("Input Results File:", this);
    m_inputFileEdit = new QLineEdit(this);
    m_inputFileEdit->setPlaceholderText("Optional: Load parameters from previous run");
    m_browseInputFileBtn = new QPushButton("Browse...", this);
    layout->addWidget(inputFileLabel, row, 0);
    layout->addWidget(m_inputFileEdit, row, 1);
    layout->addWidget(m_browseInputFileBtn, row, 2);
    row++;

    // Output filename
    QLabel* outputFileLabel = new QLabel("Output Filename:", this);
    m_outputFileEdit = new QLineEdit(this);
    m_outputFileEdit->setPlaceholderText("Name for output file");
    layout->addWidget(outputFileLabel, row, 0);
    layout->addWidget(m_outputFileEdit, row, 1);
    row++;

    layout->setColumnStretch(1, 1);

    return group;
}

void GASettingsDialog::loadSettings()
{
    // Get actual parameters from GA object
    m_maxPopSpinBox->setValue(m_ga->getPopulationSize());
    m_nGenSpinBox->setValue(m_ga->getNumGenerations());
    m_numThreadsSpinBox->setValue(m_ga->getNumThreads());

    // Genetic operators
    m_pCrossSpinBox->setValue(m_ga->getCrossoverProb());
    m_pMutateSpinBox->setValue(m_ga->getMutationProb());

    // Crossover type (1 or 2) -> index (0 or 1)
    m_crossoverTypeCombo->setCurrentIndex(m_ga->getCrossoverType() - 1);
    m_rcgaCheckBox->setChecked(m_ga->isRCGA());

    // Shake operator
    m_shakeScaleSpinBox->setValue(m_ga->getShakeScale());
    m_shakeScaleRedSpinBox->setValue(m_ga->getShakeScaleRed());

    // Fitness
    m_fitnessExpSpinBox->setValue(m_ga->getFitnessExponent());

    // Enhancements
    m_numEnhancementsSpinBox->setValue(m_ga->getNumEnhancements());

    // File paths - these would need getters too if you want to load them
    m_initialPopEdit->clear();
    m_outputPathEdit->clear();
    m_inputFileEdit->clear();
    m_outputFileEdit->setText("ga_results.txt");
}

void GASettingsDialog::saveSettings()
{
    // Apply settings directly to GA object using SetProperty
    m_ga->SetProperty("maxpop", std::to_string(m_maxPopSpinBox->value()));
    m_ga->SetProperty("ngen", std::to_string(m_nGenSpinBox->value()));
    m_ga->SetProperty("numthreads", std::to_string(m_numThreadsSpinBox->value()));

    m_ga->SetProperty("pcross", std::to_string(m_pCrossSpinBox->value()));
    m_ga->SetProperty("pmute", std::to_string(m_pMutateSpinBox->value()));

    m_ga->SetProperty("shakescale", std::to_string(m_shakeScaleSpinBox->value()));
    m_ga->SetProperty("shakescalered", std::to_string(m_shakeScaleRedSpinBox->value()));

    // File paths
    if (!m_initialPopEdit->text().isEmpty()) {
        m_ga->SetProperty("initial_population", m_initialPopEdit->text().toStdString());
    }
    if (!m_outputFileEdit->text().isEmpty()) {
        m_ga->SetProperty("outputfile", m_outputFileEdit->text().toStdString());
    }
}

bool GASettingsDialog::validateInputs()
{
    if (m_maxPopSpinBox->value() < 10) {
        QMessageBox::warning(this, "Invalid Input",
                             "Population size must be at least 10.");
        return false;
    }

    if (m_nGenSpinBox->value() < 1) {
        QMessageBox::warning(this, "Invalid Input",
                             "Number of generations must be at least 1.");
        return false;
    }

    if (m_pCrossSpinBox->value() < 0.0 || m_pCrossSpinBox->value() > 1.0) {
        QMessageBox::warning(this, "Invalid Input",
                             "Crossover probability must be between 0.0 and 1.0.");
        return false;
    }

    if (m_pMutateSpinBox->value() < 0.0 || m_pMutateSpinBox->value() > 1.0) {
        QMessageBox::warning(this, "Invalid Input",
                             "Mutation probability must be between 0.0 and 1.0.");
        return false;
    }

    return true;
}

void GASettingsDialog::onAccept()
{
    if (validateInputs()) {
        saveSettings();
        accept();
    }
}

void GASettingsDialog::onReject()
{
    reject();
}

void GASettingsDialog::browseInitialPopulation()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Select Initial Population File",
        m_outputPathEdit->text(),
        "Text Files (*.txt);;All Files (*.*)"
        );

    if (!filename.isEmpty()) {
        m_initialPopEdit->setText(filename);
    }
}

void GASettingsDialog::browseOutputPath()
{
    QString dirname = QFileDialog::getExistingDirectory(
        this,
        "Select Output Directory",
        m_outputPathEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!dirname.isEmpty()) {
        m_outputPathEdit->setText(dirname);
    }
}

void GASettingsDialog::browseInputFile()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Select Input Results File",
        m_outputPathEdit->text(),
        "Text Files (*.txt);;All Files (*.*)"
        );

    if (!filename.isEmpty()) {
        m_inputFileEdit->setText(filename);
    }
}

void GASettingsDialog::onCrossoverTypeChanged(int index)
{
    // Could add additional logic here if needed
}

void GASettingsDialog::onRCGAToggled(bool checked)
{
    m_crossoverTypeCombo->setEnabled(!checked);
}
