#ifndef TIMESERIESCHARTWIDGET_H
#define TIMESERIESCHARTWIDGET_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include "TimeSeries.h"


/**
 * @brief Widget for displaying TimeSeries data as a chart
 *
 * Shows time series data as a line chart with optional scatter points.
 * Supports zooming, panning, and automatic axis scaling.
 */
class TimeSeriesChartWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Construct chart widget
     * @param parent Parent widget
     */
    explicit TimeSeriesChartWidget(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~TimeSeriesChartWidget() override = default;

    /**
     * @brief Set the TimeSeries data to display
     * @param timeSeries Pointer to TimeSeries data
     */
    void setTimeSeries(const TimeSeries<double>* timeSeries);

    /**
     * @brief Update the chart with current data
     */
    void updateChart();

    /**
     * @brief Set chart title
     * @param title Chart title string
     */
    void setChartTitle(const QString& title);

    /**
     * @brief Set axis labels
     * @param xLabel X-axis label
     * @param yLabel Y-axis label
     */
    void setAxisLabels(const QString& xLabel, const QString& yLabel);

    /**
     * @brief Enable/disable showing data points as markers
     * @param show true to show markers
     */
    void setShowMarkers(bool show);

    /**
     * @brief Reset zoom to show all data
     */
    void resetZoom();

    /**
     * @brief Get the underlying chart view
     * @return Pointer to QChartView
     */
    QChartView* chartView() const { return chartView_; }

public slots:
    /**
     * @brief Slot to refresh chart when data changes
     */
    void onDataChanged();

private:
    void setupChart();
    void updateAxisRanges();

    QChartView* chartView_ = nullptr;
    QChart* chart_ = nullptr;
    QLineSeries* lineSeries_ = nullptr;
    QScatterSeries* scatterSeries_ = nullptr;
    QValueAxis* axisX_ = nullptr;
    QValueAxis* axisY_ = nullptr;

    const TimeSeries<double>* timeSeries_ = nullptr;
    bool showMarkers_ = true;
};

#endif // TIMESERIESCHARTWIDGET_H
