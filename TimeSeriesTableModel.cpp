#include "TimeSeriesTableModel.h"
#include <algorithm>

TimeSeriesTableModel::TimeSeriesTableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , timeSeries_(nullptr)
{
}

int TimeSeriesTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !timeSeries_) {
        return 0;
    }
    return static_cast<int>(timeSeries_->size());
}

int TimeSeriesTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant TimeSeriesTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !timeSeries_) {
        return QVariant();
    }

    const int row = index.row();
    const int col = index.column();

    if (row < 0 || row >= static_cast<int>(timeSeries_->size())) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (col) {
        case TimeColumn:
            return timeSeries_->getTime(row);
        case ValueColumn:
            return timeSeries_->getValue(row);
        default:
            return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        return QVariant::fromValue(Qt::AlignRight | Qt::AlignVCenter);
    }

    return QVariant();
}

QVariant TimeSeriesTableModel::headerData(int section, Qt::Orientation orientation,
                                          int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case TimeColumn:
            return tr("Time");
        case ValueColumn:
            return tr("Value");
        default:
            return QVariant();
        }
    }

    // Vertical header: row numbers (1-based)
    return section + 1;
}

Qt::ItemFlags TimeSeriesTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool TimeSeriesTableModel::setData(const QModelIndex& index, const QVariant& value,
                                   int role)
{
    if (!index.isValid() || role != Qt::EditRole || !timeSeries_) {
        return false;
    }

    const int row = index.row();
    const int col = index.column();

    if (row < 0 || row >= static_cast<int>(timeSeries_->size())) {
        return false;
    }

    bool ok = false;
    const double newValue = value.toDouble(&ok);
    if (!ok) {
        return false;
    }

    switch (col) {
    case TimeColumn:
        // Check for duplicate time (excluding current row)
        if (timeExists(newValue, row)) {
            return false;
        }
        timeSeries_->setTime(row, newValue);
        break;
    case ValueColumn:
        timeSeries_->setValue(row, newValue);
        break;
    default:
        return false;
    }

    emit dataChanged(index, index, {role});
    emit timeSeriesDataChanged();
    return true;
}

void TimeSeriesTableModel::setTimeSeries(TimeSeries<double>* timeSeries)
{
    beginResetModel();
    timeSeries_ = timeSeries;
    endResetModel();
    emit timeSeriesDataChanged();
}

bool TimeSeriesTableModel::addPoint(double time, double value)
{
    if (!timeSeries_) {
        return false;
    }

    // Check for duplicate time
    if (timeExists(time)) {
        return false;
    }

    const int newRow = static_cast<int>(timeSeries_->size());
    beginInsertRows(QModelIndex(), newRow, newRow);
    timeSeries_->append(time, value);
    endInsertRows();

    emit timeSeriesDataChanged();
    return true;
}

bool TimeSeriesTableModel::removePoint(int row)
{
    if (!timeSeries_ || row < 0 || row >= static_cast<int>(timeSeries_->size())) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row);

    // Create new TimeSeries without the removed point
    TimeSeries<double> newSeries;
    for (size_t i = 0; i < timeSeries_->size(); ++i) {
        if (static_cast<int>(i) != row) {
            newSeries.append(timeSeries_->getTime(i), timeSeries_->getValue(i));
        }
    }

    // Copy back
    timeSeries_->clear();
    for (size_t i = 0; i < newSeries.size(); ++i) {
        timeSeries_->append(newSeries.getTime(i), newSeries.getValue(i));
    }

    endRemoveRows();
    emit timeSeriesDataChanged();
    return true;
}

int TimeSeriesTableModel::removePoints(const QList<int>& rows)
{
    if (!timeSeries_ || rows.isEmpty()) {
        return 0;
    }

    // Sort rows in descending order to remove from end first
    QList<int> sortedRows = rows;
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

    int removed = 0;
    for (int row : sortedRows) {
        if (removePoint(row)) {
            ++removed;
        }
    }

    return removed;
}

void TimeSeriesTableModel::clearAll()
{
    if (!timeSeries_ || timeSeries_->size() == 0) {
        return;
    }

    beginResetModel();
    timeSeries_->clear();
    endResetModel();
    emit timeSeriesDataChanged();
}

double TimeSeriesTableModel::getTime(int row) const
{
    if (!timeSeries_ || row < 0 || row >= static_cast<int>(timeSeries_->size())) {
        return 0.0;
    }
    return timeSeries_->getTime(row);
}

double TimeSeriesTableModel::getValue(int row) const
{
    if (!timeSeries_ || row < 0 || row >= static_cast<int>(timeSeries_->size())) {
        return 0.0;
    }
    return timeSeries_->getValue(row);
}

bool TimeSeriesTableModel::timeExists(double time, int excludeRow) const
{
    if (!timeSeries_) {
        return false;
    }

    const double tolerance = 1e-10;
    for (size_t i = 0; i < timeSeries_->size(); ++i) {
        if (static_cast<int>(i) == excludeRow) {
            continue;
        }
        if (std::abs(timeSeries_->getTime(i) - time) < tolerance) {
            return true;
        }
    }
    return false;
}

void TimeSeriesTableModel::sortByTime()
{
    if (!timeSeries_ || timeSeries_->size() <= 1) {
        return;
    }

    beginResetModel();

    // Collect all points
    QVector<QPair<double, double>> points;
    for (size_t i = 0; i < timeSeries_->size(); ++i) {
        points.append({timeSeries_->getTime(i), timeSeries_->getValue(i)});
    }

    // Sort by time
    std::sort(points.begin(), points.end(),
              [](const QPair<double, double>& a, const QPair<double, double>& b) {
                  return a.first < b.first;
              });

    // Rebuild TimeSeries
    timeSeries_->clear();
    for (const auto& point : points) {
        timeSeries_->append(point.first, point.second);
    }

    endResetModel();
    emit timeSeriesDataChanged();
}

void TimeSeriesTableModel::refresh()
{
    beginResetModel();
    endResetModel();
    emit timeSeriesDataChanged();
}
