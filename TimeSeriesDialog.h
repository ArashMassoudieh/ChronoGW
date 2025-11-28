#ifndef TIMESERIESDIALOG_H
#define TIMESERIESDIALOG_H

#include <QDialog>
#include "TimeSeries.h"

class QLineEdit;
class QTableView;
class QPushButton;
class QSplitter;
class TimeSeriesTableModel;
class TimeSeriesChartWidget;

/**
 * @brief Dialog for creating, editing, and visualizing TimeSeries data
 *
 * Provides:
 * - Table view for editing time/value pairs
 * - Chart visualization
 * - Add/Edit/Delete point operations
 * - Export to CSV
 * - Name editing
 */
class TimeSeriesDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Construct dialog for creating new TimeSeries
     * @param parent Parent widget
     */
    explicit TimeSeriesDialog(QWidget* parent = nullptr);

    /**
     * @brief Construct dialog for editing existing TimeSeries
     * @param timeSeries TimeSeries to edit (will be modified in place)
     * @param parent Parent widget
     */
    explicit TimeSeriesDialog(TimeSeries<double>* timeSeries, QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~TimeSeriesDialog() override;

    /**
     * @brief Get the edited/created TimeSeries
     * @return Copy of the TimeSeries data
     */
    TimeSeries<double> getTimeSeries() const;

    /**
     * @brief Get the TimeSeries name
     * @return Name string
     */
    QString getName() const;

    /**
     * @brief Set the TimeSeries name
     * @param name Name string
     */
    void setName(const QString& name);

    /**
     * @brief Set axis labels for the chart
     * @param xLabel X-axis label
     * @param yLabel Y-axis label
     */
    void setAxisLabels(const QString& xLabel, const QString& yLabel);

    /**
     * @brief Get pointer to the table model
     * @return Pointer to TimeSeriesTableModel
     */
    TimeSeriesTableModel* tableModel() const { return tableModel_; }

    /**
     * @brief Get pointer to the chart widget
     * @return Pointer to TimeSeriesChartWidget
     */
    TimeSeriesChartWidget* chartWidget() const { return chartWidget_; }

    /**
     * @brief Get pointer to the table view
     * @return Pointer to QTableView
     */
    QTableView* tableView() const { return tableView_; }

signals:
    /**
     * @brief Emitted when data is modified
     */
    void dataModified();

public slots:
    /**
     * @brief Add a new data point
     */
    void addPoint();

    /**
     * @brief Edit selected data point
     */
    void editSelectedPoint();

    /**
     * @brief Delete selected data points
     */
    void deleteSelectedPoints();

    /**
     * @brief Clear all data points
     */
    void clearAllPoints();

    /**
     * @brief Sort data by time
     */
    void sortByTime();

    /**
     * @brief Export data to CSV file
     */
    void exportToCsv();

    /**
     * @brief Reset chart zoom
     */
    void resetChartZoom();

protected:
    void setupUi();
    void setupConnections();
    void updateButtonStates();

    // Called when selection changes
    void onSelectionChanged();

private:
    // UI Components
    QLineEdit* nameEdit_ = nullptr;
    QTableView* tableView_ = nullptr;
    TimeSeriesChartWidget* chartWidget_ = nullptr;
    QSplitter* splitter_ = nullptr;

    // Buttons
    QPushButton* addButton_ = nullptr;
    QPushButton* editButton_ = nullptr;
    QPushButton* deleteButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;
    QPushButton* sortButton_ = nullptr;
    QPushButton* exportButton_ = nullptr;
    QPushButton* resetZoomButton_ = nullptr;

    // Model
    TimeSeriesTableModel* tableModel_ = nullptr;

    // Data
    TimeSeries<double>* timeSeries_ = nullptr;
    TimeSeries<double> internalTimeSeries_;  // Used when creating new
    bool ownsTimeSeries_ = false;
};

#endif // TIMESERIESDIALOG_H
