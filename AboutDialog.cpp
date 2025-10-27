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

#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QPixmap>
#include <QApplication>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
    , version_("1.0.0")
{
    setWindowTitle("About ChronoGW");
    setMinimumSize(600, 500);
    setMaximumSize(700, 600);

    setupUI();
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    createHeader();
    createTabs();
    createButtons();

    setLayout(mainLayout);
}

void AboutDialog::createHeader()
{
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());

    // Header with icon and title
    QHBoxLayout* headerLayout = new QHBoxLayout();

    // Icon
    iconLabel_ = new QLabel(this);
    iconLabel_->setFixedSize(64, 64);

    // Load and scale icon
    QPixmap icon = QPixmap(":/icons/ChronoGW.png");
    if (icon.isNull()) {
        qDebug() << "Warning: Icon not found";
        icon = QPixmap(64, 64);
        icon.fill(QColor(70, 130, 180));
    }
    else {
        // Scale the pixmap BEFORE setting it
        icon = icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    iconLabel_->setPixmap(icon);
    iconLabel_->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(iconLabel_);
    headerLayout->addSpacing(15);

    // Title and version
    QVBoxLayout* titleLayout = new QVBoxLayout();

    titleLabel_ = new QLabel("<h1>ChronoGW</h1>", this);
    titleLabel_->setStyleSheet("QLabel { font-weight: bold; }");

    versionLabel_ = new QLabel("Version " + version_, this);
    versionLabel_->setStyleSheet("QLabel { color: #666; }");

    titleLayout->addWidget(titleLabel_);
    titleLayout->addWidget(versionLabel_);
    titleLayout->addStretch();

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // Separator line
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);
}

void AboutDialog::createTabs()
{
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());

    QTabWidget* tabWidget = new QTabWidget(this);

    // ========================================================================
    // About Tab
    // ========================================================================
    aboutText_ = new QTextBrowser(this);
    aboutText_->setOpenExternalLinks(true);
    aboutText_->setHtml(
        "<h2>ChronoGW</h2>"
        "<p><b>Groundwater Age Distribution Inference Using Environmental Tracers</b></p>"
        "<p>ChronoGW is a specialized tool for inferring groundwater age distributions "
        "through inverse modeling of environmental tracer data.</p>"

        "<h3>Features</h3>"
        "<ul>"
        "<li>Environmental tracer modeling (tritium, SF6, CFCs, etc.)</li>"
        "<li>Groundwater age distribution inference</li>"
        "<li>Parameter estimation using Genetic Algorithms (GA)</li>"
        "<li>Uncertainty quantification using Markov Chain Monte Carlo (MCMC)</li>"
        "<li>Advanced optimization methods (Levenberg-Marquardt)</li>"
        "<li>Real-time visualization and progress tracking</li>"
        "<li>CSV data import/export for tracer observations</li>"
        "</ul>"

        "<h3>Author</h3>"
        "<p>Developed by Arash Massoudieh<br>"
        "EnviroInformatics, LLC</p>"

        "<h3>Contact</h3>"
        "<p>For support and inquiries, please visit:<br>"
        "<a href='https://github.com/ArashMassoudieh/ChronoGW'>GitHub</a></p>"
    );

    tabWidget->addTab(aboutText_, "About");

    // ========================================================================
    // License Tab
    // ========================================================================
    licenseText_ = new QTextBrowser(this);
    licenseText_->setHtml(
        "<h2>License</h2>"
        "<p><b>GNU Affero General Public License v3.0</b></p>"
        "<p>ChronoGW - Groundwater Age Distribution Inference<br>"
        "Copyright (C) 2025 Arash Massoudieh</p>"

        "<p>This file is part of ChronoGW.</p>"

        "<p>ChronoGW is free software: you can redistribute it and/or modify it "
        "under the terms of the GNU Affero General Public License as published by "
        "the Free Software Foundation, either version 3 of the License, or (at your "
        "option) any later version.</p>"

        "<p>This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
        "GNU Affero General Public License for more details.</p>"

        "<p>You should have received a copy of the GNU Affero General Public License "
        "along with this program. If not, see "
        "<a href='https://www.gnu.org/licenses/'>https://www.gnu.org/licenses/</a>.</p>"
    );

    tabWidget->addTab(licenseText_, "License");

    // ========================================================================
    // Credits Tab
    // ========================================================================
    creditsText_ = new QTextBrowser(this);
    creditsText_->setOpenExternalLinks(true);
    creditsText_->setHtml(
        "<h2>Credits</h2>"

        "<h3>Development</h3>"
        "<p><b>Lead Developer:</b> Arash Massoudieh</p>"

        "<h3>Libraries and Components</h3>"
        "<ul>"
        "<li><b>Qt Framework</b> - Cross-platform GUI framework<br>"
        "<a href='https://www.qt.io/'>https://www.qt.io/</a></li>"

        "<li><b>GNU Scientific Library (GSL)</b> - Numerical computations<br>"
        "<a href='https://www.gnu.org/software/gsl/'>https://www.gnu.org/software/gsl/</a></li>"

        "<li><b>Armadillo</b> - Linear algebra library<br>"
        "<a href='http://arma.sourceforge.net/'>http://arma.sourceforge.net/</a></li>"

        "<li><b>OpenBLAS</b> - Optimized BLAS library<br>"
        "<a href='https://www.openblas.net/'>https://www.openblas.net/</a></li>"
        "</ul>"

        "<h3>Build Information</h3>"
        "<ul>"
        "<li><b>Qt Version:</b> " + QString(qVersion()) + "</li>"
        "<li><b>Build Type:</b> Release</li>"
        "<li><b>Compiler:</b> MSVC</li>"
        "</ul>"

        "<h3>Special Thanks</h3>"
        "<p>Thanks to all contributors and users who have provided feedback "
        "and helped improve ChronoGW.</p>"
    );

    tabWidget->addTab(creditsText_, "Credits");

    mainLayout->addWidget(tabWidget);
}

void AboutDialog::createButtons()
{
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());

    // Close button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    closeButton_ = new QPushButton("Close", this);
    closeButton_->setDefault(true);
    closeButton_->setMinimumWidth(100);

    connect(closeButton_, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addWidget(closeButton_);

    mainLayout->addLayout(buttonLayout);
}

void AboutDialog::setVersion(const QString& version)
{
    version_ = version;
    if (versionLabel_) {
        versionLabel_->setText("Version " + version_);
    }
}

void AboutDialog::setIcon(const QString& iconPath)
{
    if (iconLabel_) {
        QPixmap icon(iconPath);
        if (!icon.isNull()) {
            iconLabel_->setPixmap(icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}