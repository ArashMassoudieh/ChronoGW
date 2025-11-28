#ifndef TIMESERIESTABLEMODEL_H
#define TIMESERIESTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QPair>
#include "TimeSeries.h"

/**
 * @brief Table model for displaying and editing TimeSeries<double> data
 *
 * Provides a two-column model (Time, Value) that can be used with QTableView.
 * Changes to the model are reflected in the underlying TimeSeries object.
 */
class TimeSeriesTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        TimeColumn = 0,
        ValueColumn = 1,
        ColumnCount = 2
    };

    /**
     * @brief Construct model with optional TimeSeries data
     * @param parent Parent object
     */
    explicit TimeSeriesTableModel(QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~TimeSeriesTableModel() override = default;

    // ========================================================================
    // QAbstractTableModel interface
    // ========================================================================

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override;

    // ========================================================================
    // Data management
    // ========================================================================

    /**
     * @brief Set the TimeSeries data to display/edit
     * @param timeSeries Pointer to TimeSeries (model does not take ownership)
     */
    void setTimeSeries(TimeSeries<double>* timeSeries);

    /**
     * @brief Get the current TimeSeries pointer
     * @return Pointer to TimeSeries, or nullptr if not set
     */
    TimeSeries<double>* getTimeSeries() const { return timeSeries_; }

    /**
     * @brief Add a new data point
     * @param time Time value
     * @param value Data value
     * @return true if successful, false if time already exists
     */
    bool addPoint(double time, double value);

    /**
     * @brief Remove a data point by row index
     * @param row Row index to remove
     * @return true if successful
     */
    bool removePoint(int row);

    /**
     * @brief Remove multiple rows
     * @param rows List of row indices to remove (will be sorted descending)
     * @return Number of rows removed
     */
    int removePoints(const QList<int>& rows);

    /**
     * @brief Clear all data points
     */
    void clearAll();

    /**
     * @brief Get time value at row
     * @param row Row index
     * @return Time value, or 0.0 if invalid row
     */
    double getTime(int row) const;

    /**
     * @brief Get data value at row
     * @param row Row index
     * @return Data value, or 0.0 if invalid row
     */
    double getValue(int row) const;

    /**
     * @brief Check if a time value already exists
     * @param time Time to check
     * @param excludeRow Row to exclude from check (-1 for none)
     * @return true if time exists
     */
    bool timeExists(double time, int excludeRow = -1) const;

    /**
     * @brief Sort data by time (ascending)
     */
    void sortByTime();

    /**
     * @brief Refresh the model from the underlying TimeSeries
     */
    void refresh();

signals:
    /**
     * @brief Emitted when data changes (for chart updates)
     */
    void timeSeriesDataChanged();

private:
    TimeSeries<double>* timeSeries_ = nullptr;
};

#endif // TIMESERIESTABLEMODEL_H
