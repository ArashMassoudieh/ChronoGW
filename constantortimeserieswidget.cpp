#include "constantortimeserieswidget.h"
#include "chartwindow.h"
#include <QFileDialog>
#include <QDoubleValidator>
#include <QFileInfo>
#include <QMessageBox>
#include <QDate>
#include <QMouseEvent>

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

    loadButton = new QPushButton(this);
    loadButton->setIcon(QIcon(":/icons/open.png"));
    loadButton->setToolTip("Load time series from file");
    loadButton->setMaximumWidth(30);
    loadButton->setIconSize(QSize(20, 20));  // Adjust size as needed

    // View button with custom icon
    viewButton = new QPushButton(this);
    viewButton->setIcon(QIcon(":/icons/observeddata.png"));
    viewButton->setToolTip("View time series chart");
    viewButton->setMaximumWidth(30);
    viewButton->setIconSize(QSize(20, 20));  // Adjust size as needed
    viewButton->setEnabled(false);

    // Create layout
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(lineEdit);
    layout->addWidget(loadButton);
    layout->addWidget(viewButton);

    // Connect signals
    connect(lineEdit, &QLineEdit::textChanged,
            this, &ConstantOrTimeSeriesWidget::onLineEditChanged);
    connect(loadButton, &QPushButton::clicked,
            this, &ConstantOrTimeSeriesWidget::onLoadButtonClicked);
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

    lineEdit->setReadOnly(false);  // Make sure it's editable
    lineEdit->setEnabled(true);    // And enabled

    updateViewButtonState();

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
    timeSeries.setFilename(filename.toStdString());  // Store filename in TimeSeries object

    lineEdit->blockSignals(true);
    lineEdit->setText(filename);
    lineEdit->blockSignals(false);
    lineEdit->setReadOnly(true);

    updateViewButtonState();

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
    viewButton->setEnabled(enabled && hasTimeSeries());
    QWidget::setEnabled(enabled);
}

void ConstantOrTimeSeriesWidget::onLineEditChanged()
{
    // Only process if in constant mode and user is actually typing
    // (not if we're programmatically setting the text)
    if (isConstant && !lineEdit->text().isEmpty()) {
        // Update the time series with new constant value
        double value = lineEdit->text().toDouble();
        QDate currentDate = QDate::currentDate();
        int currentYear = currentDate.year();

        timeSeries = TimeSeries<double>();
        timeSeries.append(1960.0, value);
        timeSeries.append(static_cast<double>(currentYear), value);

        updateViewButtonState();
        emit valueChanged();
    }
    // Don't do anything if isConstant is false - we're in time series mode
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

void ConstantOrTimeSeriesWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    // If we're in time series mode and user double-clicks anywhere on the widget
    if (!isConstant) {
        isConstant = true;
        timeSeriesFilename.clear();
        timeSeries = TimeSeries<double>();

        lineEdit->clear();
        lineEdit->setReadOnly(false);  // Make it editable
        lineEdit->setEnabled(true);
        lineEdit->setPlaceholderText("Enter constant value");

        updateViewButtonState();
        emit valueChanged();

        // Set focus so user can start typing immediately
        lineEdit->setFocus();

        event->accept();
    } else {
        QWidget::mouseDoubleClickEvent(event);
    }
}

void ConstantOrTimeSeriesWidget::setTimeSeries(const TimeSeries<double>& ts)
{
    isConstant = false;
    timeSeries = ts;  // Copy the actual data

    // DEBUG: Check what we actually received
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
    emit valueChanged();
}

bool ConstantOrTimeSeriesWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == lineEdit && event->type() == QEvent::MouseButtonDblClick) {
        // Catch double-clicks on the lineEdit (even when read-only)
        if (!isConstant) {
            // Switch to constant mode
            isConstant = true;
            timeSeriesFilename.clear();
            timeSeries = TimeSeries<double>();

            lineEdit->clear();
            lineEdit->setReadOnly(false);  // Make it editable
            lineEdit->setEnabled(true);
            lineEdit->setPlaceholderText("Enter constant value");

            updateViewButtonState();
            emit valueChanged();

            // Set focus so user can start typing immediately
            lineEdit->setFocus();

            return true;  // Event handled
        }
    }

    return QWidget::eventFilter(obj, event);
}
