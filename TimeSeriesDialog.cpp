#include "TimeSeriesDialog.h"
#include "TimeSeriesTableModel.h"
#include "TimeSeriesChartWidget.h"
#include "TimeSeriesPointDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QSplitter>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGroupBox>
#include <fstream>

TimeSeriesDialog::TimeSeriesDialog(QWidget* parent)
    : QDialog(parent)
    , ownsTimeSeries_(true)
{
    timeSeries_ = &internalTimeSeries_;
    setupUi();
    setupConnections();
}

TimeSeriesDialog::TimeSeriesDialog(TimeSeries<double>* timeSeries, QWidget* parent)
    : QDialog(parent)
    , timeSeries_(timeSeries)
    , ownsTimeSeries_(false)
{
    if (!timeSeries_) {
        timeSeries_ = &internalTimeSeries_;
        ownsTimeSeries_ = true;
    }
    setupUi();
    setupConnections();

    // Set name from TimeSeries if available
    if (!timeSeries_->name().empty()) {
        nameEdit_->setText(QString::fromStdString(timeSeries_->name()));
    }
}

TimeSeriesDialog::~TimeSeriesDialog()
{
    // internalTimeSeries_ is a member, no need to delete
}

void TimeSeriesDialog::setupUi()
{
    setWindowTitle(tr("Time Series Editor"));
    setMinimumSize(800, 600);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Name row
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel(tr("Name:")));
    nameEdit_ = new QLineEdit();
    nameEdit_->setPlaceholderText(tr("Enter time series name..."));
    nameLayout->addWidget(nameEdit_);
    mainLayout->addLayout(nameLayout);

    // Splitter for table and chart
    splitter_ = new QSplitter(Qt::Vertical);

    // Chart widget (top)
    chartWidget_ = new TimeSeriesChartWidget();
    chartWidget_->setMinimumHeight(200);
    splitter_->addWidget(chartWidget_);

    // Table section (bottom)
    QWidget* tableSection = new QWidget();
    QVBoxLayout* tableSectionLayout = new QVBoxLayout(tableSection);
    tableSectionLayout->setContentsMargins(0, 0, 0, 0);

    // Table view
    tableView_ = new QTableView();
    tableModel_ = new TimeSeriesTableModel(this);
    tableModel_->setTimeSeries(timeSeries_);
    tableView_->setModel(tableModel_);

    // Table configuration
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tableView_->setAlternatingRowColors(true);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView_->verticalHeader()->setDefaultSectionSize(24);
    tableView_->setEditTriggers(QAbstractItemView::DoubleClicked | 
                                 QAbstractItemView::EditKeyPressed);

    tableSectionLayout->addWidget(tableView_);

    // Button row
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    addButton_ = new QPushButton(tr("Add Point"));
    addButton_->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    buttonLayout->addWidget(addButton_);

    editButton_ = new QPushButton(tr("Edit"));
    editButton_->setEnabled(false);
    buttonLayout->addWidget(editButton_);

    deleteButton_ = new QPushButton(tr("Delete"));
    deleteButton_->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    deleteButton_->setEnabled(false);
    buttonLayout->addWidget(deleteButton_);

    buttonLayout->addSpacing(20);

    sortButton_ = new QPushButton(tr("Sort by Time"));
    buttonLayout->addWidget(sortButton_);

    clearButton_ = new QPushButton(tr("Clear All"));
    buttonLayout->addWidget(clearButton_);

    buttonLayout->addStretch();

    exportButton_ = new QPushButton(tr("Export CSV..."));
    exportButton_->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    buttonLayout->addWidget(exportButton_);

    resetZoomButton_ = new QPushButton(tr("Reset Zoom"));
    buttonLayout->addWidget(resetZoomButton_);

    tableSectionLayout->addLayout(buttonLayout);

    splitter_->addWidget(tableSection);

    // Set initial splitter sizes (60% chart, 40% table)
    splitter_->setSizes({360, 240});

    mainLayout->addWidget(splitter_);

    // Dialog buttons
    QDialogButtonBox* dialogButtons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(dialogButtons);

    connect(dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Initialize chart
    chartWidget_->setTimeSeries(timeSeries_);
}

void TimeSeriesDialog::setupConnections()
{
    // Button connections
    connect(addButton_, &QPushButton::clicked, this, &TimeSeriesDialog::addPoint);
    connect(editButton_, &QPushButton::clicked, this, &TimeSeriesDialog::editSelectedPoint);
    connect(deleteButton_, &QPushButton::clicked, this, &TimeSeriesDialog::deleteSelectedPoints);
    connect(clearButton_, &QPushButton::clicked, this, &TimeSeriesDialog::clearAllPoints);
    connect(sortButton_, &QPushButton::clicked, this, &TimeSeriesDialog::sortByTime);
    connect(exportButton_, &QPushButton::clicked, this, &TimeSeriesDialog::exportToCsv);
    connect(resetZoomButton_, &QPushButton::clicked, this, &TimeSeriesDialog::resetChartZoom);

    // Model-chart connection
    connect(tableModel_, &TimeSeriesTableModel::timeSeriesDataChanged,
            chartWidget_, &TimeSeriesChartWidget::onDataChanged);

    // Selection change
    connect(tableView_->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &TimeSeriesDialog::onSelectionChanged);

    // Data modified signal
    connect(tableModel_, &TimeSeriesTableModel::timeSeriesDataChanged,
            this, &TimeSeriesDialog::dataModified);

    // Double-click to edit
    connect(tableView_, &QTableView::doubleClicked, this, &TimeSeriesDialog::editSelectedPoint);

    // Name change
    connect(nameEdit_, &QLineEdit::textChanged, [this](const QString& text) {
        if (timeSeries_) {
            timeSeries_->setName(text.toStdString());
        }
    });
}

void TimeSeriesDialog::onSelectionChanged()
{
    updateButtonStates();
}

void TimeSeriesDialog::updateButtonStates()
{
    bool hasSelection = tableView_->selectionModel()->hasSelection();
    int selectedCount = tableView_->selectionModel()->selectedRows().count();

    editButton_->setEnabled(selectedCount == 1);
    deleteButton_->setEnabled(hasSelection);
}

void TimeSeriesDialog::addPoint()
{
    TimeSeriesPointDialog dialog(this);

    // Suggest next time value
    double suggestedTime = 0.0;
    if (timeSeries_ && timeSeries_->size() > 0) {
        // Find max time and add 1
        double maxTime = timeSeries_->getTime(0);
        for (size_t i = 1; i < timeSeries_->size(); ++i) {
            maxTime = std::max(maxTime, timeSeries_->getTime(i));
        }
        suggestedTime = maxTime + 1.0;
    }
    dialog.setAddMode(suggestedTime);

    if (dialog.exec() == QDialog::Accepted) {
        double time = dialog.getTime();
        double value = dialog.getValue();

        if (tableModel_->timeExists(time)) {
            QMessageBox::warning(this, tr("Duplicate Time"),
                                 tr("A point with time %1 already exists.").arg(time));
            return;
        }

        tableModel_->addPoint(time, value);
    }
}

void TimeSeriesDialog::editSelectedPoint()
{
    QModelIndexList selected = tableView_->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }

    int row = selected.first().row();
    double currentTime = tableModel_->getTime(row);
    double currentValue = tableModel_->getValue(row);

    TimeSeriesPointDialog dialog(this);
    dialog.setEditMode(currentTime, currentValue);

    if (dialog.exec() == QDialog::Accepted) {
        double newTime = dialog.getTime();
        double newValue = dialog.getValue();

        // Check for duplicate time (excluding current row)
        if (tableModel_->timeExists(newTime, row)) {
            QMessageBox::warning(this, tr("Duplicate Time"),
                                 tr("A point with time %1 already exists.").arg(newTime));
            return;
        }

        // Update via model
        QModelIndex timeIndex = tableModel_->index(row, TimeSeriesTableModel::TimeColumn);
        QModelIndex valueIndex = tableModel_->index(row, TimeSeriesTableModel::ValueColumn);

        tableModel_->setData(timeIndex, newTime);
        tableModel_->setData(valueIndex, newValue);
    }
}

void TimeSeriesDialog::deleteSelectedPoints()
{
    QModelIndexList selected = tableView_->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }

    int count = selected.count();
    QString message = (count == 1) 
        ? tr("Delete the selected point?")
        : tr("Delete %1 selected points?").arg(count);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Confirm Delete"), message,
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QList<int> rows;
        for (const QModelIndex& index : selected) {
            rows.append(index.row());
        }
        tableModel_->removePoints(rows);
    }
}

void TimeSeriesDialog::clearAllPoints()
{
    if (!timeSeries_ || timeSeries_->size() == 0) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Confirm Clear"),
        tr("Delete all %1 data points?").arg(timeSeries_->size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        tableModel_->clearAll();
    }
}

void TimeSeriesDialog::sortByTime()
{
    tableModel_->sortByTime();
}

void TimeSeriesDialog::exportToCsv()
{
    if (!timeSeries_ || timeSeries_->size() == 0) {
        QMessageBox::information(this, tr("Export"),
                                 tr("No data to export."));
        return;
    }

    QString defaultName = nameEdit_->text().isEmpty() 
        ? "timeseries.csv" 
        : nameEdit_->text() + ".csv";

    QString filename = QFileDialog::getSaveFileName(
        this, tr("Export to CSV"), defaultName,
        tr("CSV Files (*.csv);;All Files (*)"));

    if (filename.isEmpty()) {
        return;
    }

    std::ofstream file(filename.toStdString());
    if (!file.is_open()) {
        QMessageBox::critical(this, tr("Export Error"),
                              tr("Could not open file for writing."));
        return;
    }

    // Write header
    file << "Time,Value\n";

    // Write data (sorted by time)
    QVector<QPair<double, double>> points;
    for (size_t i = 0; i < timeSeries_->size(); ++i) {
        points.append({timeSeries_->getTime(i), timeSeries_->getValue(i)});
    }
    std::sort(points.begin(), points.end(),
              [](const QPair<double, double>& a, const QPair<double, double>& b) {
                  return a.first < b.first;
              });

    file << std::fixed;
    for (const auto& point : points) {
        file << point.first << "," << point.second << "\n";
    }

    file.close();

    QMessageBox::information(this, tr("Export Complete"),
                             tr("Exported %1 points to %2")
                                 .arg(timeSeries_->size())
                                 .arg(filename));
}

void TimeSeriesDialog::resetChartZoom()
{
    chartWidget_->resetZoom();
}

TimeSeries<double> TimeSeriesDialog::getTimeSeries() const
{
    if (timeSeries_) {
        return *timeSeries_;
    }
    return TimeSeries<double>();
}

QString TimeSeriesDialog::getName() const
{
    return nameEdit_->text();
}

void TimeSeriesDialog::setName(const QString& name)
{
    nameEdit_->setText(name);
    if (timeSeries_) {
        timeSeries_->setName(name.toStdString());
    }
}

void TimeSeriesDialog::setAxisLabels(const QString& xLabel, const QString& yLabel)
{
    chartWidget_->setAxisLabels(xLabel, yLabel);
}
