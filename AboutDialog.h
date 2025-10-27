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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>

 /**
  * @class AboutDialog
  * @brief About dialog displaying application information, version, and credits
  */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit AboutDialog(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~AboutDialog();

    /**
     * @brief Set application version
     * @param version Version string (e.g., "1.0.0")
     */
    void setVersion(const QString& version);

    /**
     * @brief Set application icon
     * @param iconPath Path to icon file
     */
    void setIcon(const QString& iconPath);

private:
    void setupUI();
    void createHeader();
    void createTabs();
    void createButtons();

    // UI Components
    QLabel* iconLabel_;
    QLabel* titleLabel_;
    QLabel* versionLabel_;
    QTextBrowser* aboutText_;
    QTextBrowser* licenseText_;
    QTextBrowser* creditsText_;
    QPushButton* closeButton_;

    QString version_;
};

#endif // ABOUTDIALOG_H