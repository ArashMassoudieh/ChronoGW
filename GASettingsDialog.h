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

#ifndef GASETTINGSDIALOG_H
#define GASETTINGSDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include "GA.h"
#include "GWA.h"

/**
 * @class GASettingsDialog
 * @brief Dialog window for configuring Genetic Algorithm parameters
 *
 * Provides a user-friendly interface for setting all GA parameters including:
 * - Population and generation settings
 * - Genetic operator probabilities
 * - Crossover configuration
 * - Shake operator parameters
 * - File paths for input/output
 */
class GASettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor with CGA pointer
     * @param ga Pointer to CGA object to configure
     * @param parent Parent widget
     */
    explicit GASettingsDialog(CGA<CGWA>* ga, QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~GASettingsDialog();

private slots:
    /**
     * @brief Slot called when OK button is clicked
     * Validates and saves all settings
     */
    void onAccept();

    /**
     * @brief Slot called when Cancel button is clicked
     */
    void onReject();

    /**
     * @brief Browse for initial population file
     */
    void browseInitialPopulation();

    /**
     * @brief Browse for output path
     */
    void browseOutputPath();

    /**
     * @brief Browse for input results file
     */
    void browseInputFile();

    /**
     * @brief Handle crossover type change
     */
    void onCrossoverTypeChanged(int index);

    /**
     * @brief Update UI when RCGA checkbox is toggled
     */
    void onRCGAToggled(bool checked);

private:
    /**
     * @brief Create and layout all UI elements
     */
    void setupUI();

    /**
     * @brief Load current settings into UI controls
     */
    void loadSettings();

    /**
     * @brief Save UI control values to settings
     */
    void saveSettings();

    /**
     * @brief Create the population parameters group
     */
    QGroupBox* createPopulationGroup();

    /**
     * @brief Create the genetic operators group
     */
    QGroupBox* createGeneticOperatorsGroup();

    /**
     * @brief Create the shake operator group
     */
    QGroupBox* createShakeOperatorGroup();

    /**
     * @brief Create the fitness scaling group
     */
    QGroupBox* createFitnessGroup();

    /**
     * @brief Create the enhancement parameters group
     */
    QGroupBox* createEnhancementGroup();

    /**
     * @brief Create the file paths group
     */
    QGroupBox* createFilePathsGroup();

    /**
     * @brief Validate all input values
     * @return true if all values are valid
     */
    bool validateInputs();

    // Reference to GA object
    CGA<CGWA>* m_ga;

    // Local copies of parameters for dialog
    GAParameters m_params;
    GAFilenames m_filenames;

    // Population parameters
    QSpinBox* m_maxPopSpinBox;
    QSpinBox* m_nGenSpinBox;
    QSpinBox* m_numThreadsSpinBox;

    // Genetic operator parameters
    QDoubleSpinBox* m_pCrossSpinBox;
    QDoubleSpinBox* m_pMutateSpinBox;
    QComboBox* m_crossoverTypeCombo;
    QCheckBox* m_rcgaCheckBox;

    // Shake operator parameters
    QDoubleSpinBox* m_shakeScaleSpinBox;
    QDoubleSpinBox* m_shakeScaleRedSpinBox;

    // Fitness parameters
    QDoubleSpinBox* m_fitnessExpSpinBox;

    // Enhancement parameters
    QSpinBox* m_numEnhancementsSpinBox;

    // File paths
    QLineEdit* m_initialPopEdit;
    QLineEdit* m_outputPathEdit;
    QLineEdit* m_inputFileEdit;
    QLineEdit* m_outputFileEdit;
    QPushButton* m_browseInitialPopBtn;
    QPushButton* m_browseOutputPathBtn;
    QPushButton* m_browseInputFileBtn;

    // Dialog buttons
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_resetButton;
};

#endif // GASETTINGSDIALOG_H
