#include "constantortimeserieswidget.h"
#include "chartwindow.h"
#include "TimeSeriesDialog.h"
#include <QFileDialog>
#include <QDoubleValidator>
#include <QFileInfo>
#include <QMessageBox>
#include <QDate>
#include <QMouseEvent>
#include <fstream>

ConstantOrTimeSeriesWidget::ConstantOrTimeSeriesWidget(QWidget *parent)
    : QWidget(parent)
    , isConstant(true)
{
    // Create widgets
    lineEdit = new QLineEdit(this);
    lineEdit->setValidator(new QDoubleValidator(this));
    lineEdit->setPlaceholderText("Enter constant value");
    lineEdit->setEnabled(true);

    lineEdit->installEventFilter(this);

    // Load button
    loadButton = new QPushButton(this);
    loadButton->setIcon(QIcon(":/icons/open.png"));
    loadButton->setToolTip("Load time series from file");
    loadButton->setMaximumWidth(30);
    loadButton->setIconSize(QSize(20, 20));

    // Edit button
    editButton = new QPushButton(this);
    editButton->setIcon(QIcon(":/icons/edittimeseries.png"));
    editButton->setToolTip("Create or edit time series");
    editButton->setMaximumWidth(30);
    editButton->setIconSize(QSize(20, 20));

    // View button
    viewButton = new QPushButton(this);
    viewButton->setIcon(QIcon(":/icons/observeddata.png"));
    viewButton->setToolTip("View time series chart");
    viewButton->setMaximumWidth(30);
    viewButton->setIconSize(QSize(20, 20));
    viewButton->setEnabled(false);

    // Create layout
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(lineEdit);
    layout->addWidget(loadButton);
    layout->addWidget(editButton);
    layout->addWidget(viewButton);

    // Connect signals
    connect(lineEdit, &QLineEdit::textChanged,
            this, &ConstantOrTimeSeriesWidget::onLineEditChanged);
    connect(loadButton, &QPushButton::clicked,
            this, &ConstantOrTimeSeriesWidget::onLoadButtonClicked);
    connect(editButton, &QPushButton::clicked,
            this, &ConstantOrTimeSeriesWidget::onEditButtonClicked);
    connect(viewButton, &QPushButton::clicked,
            this, &ConstantOrTimeSeriesWidget::onViewButtonClicked);

    setLayout(layout);
}

void ConstantOrTimeSeriesWidget::setConstantValue(double value)
{
    isConstant = true;
    timeSeriesFilename.clear();

    // Create a time series from 1960 to current year with constant value
    QDate currentDate = QDate::currentDate();
    int currentYear = currentDate.year();

    timeSeries = TimeSeries<double>();
    timeSeries.append(1960.0, value);
    timeSeries.append(static_cast<double>(currentYear), value);

    // Block signals to prevent onLineEditChanged from being triggered
    lineEdit->blockSignals(true);
    lineEdit->setText(QString::number(value));
    lineEdit->blockSignals(false);

    lineEdit->setReadOnly(false);
    lineEdit->setEnabled(true);

    updateViewButtonState();
    updateEditButtonState();

    emit valueChanged();
}

void ConstantOrTimeSeriesWidget::setTimeSeriesFile(const QString& filename)
{
    if (filename.isEmpty()) {
        return;
    }

    // Clear any existing time series
    timeSeries = TimeSeries<double>();

    // Try to read the file
    bool success = timeSeries.readfile(filename.toStdString());

    if (!success) {
        QMessageBox::warning(this, "Error Loading Time Series",
                             QString("Failed to load time series from file:\n%1\n\n"
                                     "Please check that the file exists and is in the correct format.").arg(filename));
        return;
    }

    // Check if data was actually loaded
    if (timeSeries.size() == 0) {
        QMessageBox::warning(this, "Error Loading Time Series",
                             QString("Time series file is empty or contains no valid data:\n%1").arg(filename));
        return;
    }

    // Successfully loaded - update widget state
    isConstant = false;
    timeSeriesFilename = filename;
    timeSeries.setFilename(filename.toStdString());

    lineEdit->blockSignals(true);
    lineEdit->setText(filename);
    lineEdit->blockSignals(false);
    lineEdit->setReadOnly(true);

    updateViewButtonState();
    updateEditButtonState();

    emit timeSeriesLoaded();
    emit valueChanged();
}

bool ConstantOrTimeSeriesWidget::isConstantMode() const
{
    return isConstant;
}

double ConstantOrTimeSeriesWidget::getConstantValue() const
{
    if (isConstant) {
        return lineEdit->text().toDouble();
    }
    return 0.0;
}

QString ConstantOrTimeSeriesWidget::getTimeSeriesFilename() const
{
    return timeSeriesFilename;
}

const TimeSeries<double>& ConstantOrTimeSeriesWidget::getTimeSeries() const
{
    return timeSeries;
}

bool ConstantOrTimeSeriesWidget::hasTimeSeries() const
{
    return timeSeries.size() > 0;
}

void ConstantOrTimeSeriesWidget::setPlaceholderText(const QString& text)
{
    lineEdit->setPlaceholderText(text);
}

void ConstantOrTimeSeriesWidget::setEnabled(bool enabled)
{
    lineEdit->setEnabled(enabled && isConstant);
    loadButton->setEnabled(enabled);
    editButton->setEnabled(enabled);
    viewButton->setEnabled(enabled && hasTimeSeries());
    QWidget::setEnabled(enabled);
}

void ConstantOrTimeSeriesWidget::onLineEditChanged()
{
    if (isConstant && !lineEdit->text().isEmpty()) {
        double value = lineEdit->text().toDouble();
        QDate currentDate = QDate::currentDate();
        int currentYear = currentDate.year();

        timeSeries = TimeSeries<double>();
        timeSeries.append(1960.0, value);
        timeSeries.append(static_cast<double>(currentYear), value);

        updateViewButtonState();
        updateEditButtonState();
        emit valueChanged();
    }
}

void ConstantOrTimeSeriesWidget::onLoadButtonClicked()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Load Time Series"),
        QString(),
        tr("Text Files (*.txt);;CSV Files (*.csv);;All Files (*)")
        );

    if (!filename.isEmpty()) {
        setTimeSeriesFile(filename);
    }
}

void ConstantOrTimeSeriesWidget::onEditButtonClicked()
{
    // Determine if we're editing existing file or creating new
    bool hasExistingFile = !timeSeriesFilename.isEmpty() && QFileInfo::exists(timeSeriesFilename);

    // Create a working copy of the time series
    TimeSeries<double> workingCopy;

    if (hasExistingFile) {
        // Load fresh from file to edit
        workingCopy = timeSeries;
    } else if (hasTimeSeries() && isConstant) {
        // Start with current constant-generated time series
        workingCopy = timeSeries;
    }
    // Otherwise workingCopy is empty (creating new)

    // Open the dialog
    TimeSeriesDialog dialog(&workingCopy, this);

    if (hasExistingFile) {
        dialog.setWindowTitle(tr("Edit Time Series - %1").arg(QFileInfo(timeSeriesFilename).fileName()));
        dialog.setName(QString::fromStdString(workingCopy.name()));
    } else {
        dialog.setWindowTitle(tr("Create Time Series"));
    }

    dialog.setAxisLabels(tr("Time"), tr("Value"));

    if (dialog.exec() == QDialog::Accepted) {
        // Get the edited data
        TimeSeries<double> editedData = dialog.getTimeSeries();

        // Check if there's any data
        if (editedData.size() == 0) {
            QMessageBox::warning(this, tr("No Data"),
                                 tr("The time series has no data points. Operation cancelled."));
            return;
        }

        // Determine save path
        QString savePath;

        if (hasExistingFile) {
            // Ask if user wants to save to same file or new file
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                tr("Save Time Series"),
                tr("Save changes to the existing file?\n\n%1\n\n"
                   "Click 'No' to save to a new file.").arg(timeSeriesFilename),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
                );

            if (reply == QMessageBox::Cancel) {
                return;
            }

            if (reply == QMessageBox::Yes) {
                savePath = timeSeriesFilename;
            } else {
                // User wants new file
                savePath = QFileDialog::getSaveFileName(
                    this,
                    tr("Save Time Series As"),
                    QFileInfo(timeSeriesFilename).absolutePath(),
                    tr("Text Files (*.txt);;CSV Files (*.csv);;All Files (*)")
                    );

                if (savePath.isEmpty()) {
                    return;  // User cancelled
                }
            }
        } else {
            // No existing file - must save to new file
            savePath = QFileDialog::getSaveFileName(
                this,
                tr("Save Time Series"),
                QString(),
                tr("Text Files (*.txt);;CSV Files (*.csv);;All Files (*)")
                );

            if (savePath.isEmpty()) {
                QMessageBox::information(this, tr("Save Required"),
                                         tr("You must save the time series to a file to use it."));
                return;
            }
        }

        // Save the time series to file
        try {
            // Sort by time before saving
            TimeSeries<double> sortedData;
            QVector<QPair<double, double>> points;
            for (size_t i = 0; i < editedData.size(); ++i) {
                points.append({editedData.getTime(i), editedData.getValue(i)});
            }
            std::sort(points.begin(), points.end(),
                      [](const QPair<double, double>& a, const QPair<double, double>& b) {
                          return a.first < b.first;
                      });

            for (const auto& point : points) {
                sortedData.append(point.first, point.second);
            }

            sortedData.writefile(savePath.toStdString());

        } catch (const std::exception& e) {
            QMessageBox::critical(this, tr("Save Error"),
                                  tr("Failed to save time series:\n%1").arg(e.what()));
            return;
        }

        // Successfully saved - now load from that file to update widget
        setTimeSeriesFile(savePath);
    }
}

void ConstantOrTimeSeriesWidget::onViewButtonClicked()
{
    qDebug() << "onViewButtonClicked:";
    qDebug() << "  hasTimeSeries:" << hasTimeSeries();
    qDebug() << "  timeSeries.size():" << timeSeries.size();

    if (!hasTimeSeries()) {
        return;
    }

    // Create a TimeSeriesSet with single series for display
    TimeSeriesSet<double> dataSet(1);
    dataSet[0] = timeSeries;

    QString title = timeSeriesFilename.isEmpty()
                        ? "Time Series"
                        : QFileInfo(timeSeriesFilename).fileName();
    dataSet.setname(0, title.toStdString());

    // Show chart window
    ChartWindow::showChart(dataSet, title, this);
}

void ConstantOrTimeSeriesWidget::updateViewButtonState()
{
    viewButton->setEnabled(hasTimeSeries());
}

void ConstantOrTimeSeriesWidget::updateEditButtonState()
{
    // Edit button is always enabled - creates new if no data
    editButton->setEnabled(true);
}

void ConstantOrTimeSeriesWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!isConstant) {
        isConstant = true;
        timeSeriesFilename.clear();
        timeSeries = TimeSeries<double>();

        lineEdit->clear();
        lineEdit->setReadOnly(false);
        lineEdit->setEnabled(true);
        lineEdit->setPlaceholderText("Enter constant value");

        updateViewButtonState();
        updateEditButtonState();
        emit valueChanged();

        lineEdit->setFocus();

        event->accept();
    } else {
        QWidget::mouseDoubleClickEvent(event);
    }
}

void ConstantOrTimeSeriesWidget::setTimeSeries(const TimeSeries<double>& ts)
{
    isConstant = false;
    timeSeries = ts;

    qDebug() << "setTimeSeries called:";
    qDebug() << "  Size:" << timeSeries.size();
    qDebug() << "  Filename:" << QString::fromStdString(ts.getFilename());
    if (timeSeries.size() > 0) {
        qDebug() << "  First point: t=" << timeSeries.getTime(0) << ", v=" << timeSeries.getValue(0);
        qDebug() << "  Last point: t=" << timeSeries.getTime(timeSeries.size()-1)
                 << ", v=" << timeSeries.getValue(timeSeries.size()-1);
    }

    QString filename = QString::fromStdString(ts.getFilename());
    timeSeriesFilename = filename;

    lineEdit->blockSignals(true);
    lineEdit->setText(filename);
    lineEdit->blockSignals(false);

    lineEdit->setReadOnly(true);
    lineEdit->setEnabled(true);

    updateViewButtonState();
    updateEditButtonState();
    emit valueChanged();
}

bool ConstantOrTimeSeriesWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == lineEdit && event->type() == QEvent::MouseButtonDblClick) {
        if (!isConstant) {
            isConstant = true;
            timeSeriesFilename.clear();
            timeSeries = TimeSeries<double>();

            lineEdit->clear();
            lineEdit->setReadOnly(false);
            lineEdit->setEnabled(true);
            lineEdit->setPlaceholderText("Enter constant value");

            updateViewButtonState();
            updateEditButtonState();
            emit valueChanged();

            lineEdit->setFocus();

            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}
