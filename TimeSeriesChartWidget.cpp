#include "TimeSeriesChartWidget.h"
#include <QVBoxLayout>
#include <cmath>
#include <limits>

TimeSeriesChartWidget::TimeSeriesChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setupChart();
}

void TimeSeriesChartWidget::setupChart()
{
    // Create chart
    chart_ = new QChart();
    chart_->setAnimationOptions(QChart::NoAnimation);
    chart_->legend()->hide();

    // Create line series
    lineSeries_ = new QLineSeries();
    lineSeries_->setName(tr("Data"));
    chart_->addSeries(lineSeries_);

    // Create scatter series for markers
    scatterSeries_ = new QScatterSeries();
    scatterSeries_->setName(tr("Points"));
    scatterSeries_->setMarkerSize(8);
    chart_->addSeries(scatterSeries_);

    // Create axes
    axisX_ = new QValueAxis();
    axisX_->setTitleText(tr("Time"));
    axisX_->setLabelFormat("%.2f");
    chart_->addAxis(axisX_, Qt::AlignBottom);
    lineSeries_->attachAxis(axisX_);
    scatterSeries_->attachAxis(axisX_);

    axisY_ = new QValueAxis();
    axisY_->setTitleText(tr("Value"));
    axisY_->setLabelFormat("%.4g");
    chart_->addAxis(axisY_, Qt::AlignLeft);
    lineSeries_->attachAxis(axisY_);
    scatterSeries_->attachAxis(axisY_);

    // Create chart view
    chartView_ = new QChartView(chart_);
    chartView_->setRenderHint(QPainter::Antialiasing);
    chartView_->setRubberBand(QChartView::RectangleRubberBand);

    // Layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView_);
}

void TimeSeriesChartWidget::setTimeSeries(const TimeSeries<double>* timeSeries)
{
    timeSeries_ = timeSeries;
    updateChart();
}

void TimeSeriesChartWidget::updateChart()
{
    lineSeries_->clear();
    scatterSeries_->clear();

    if (!timeSeries_ || timeSeries_->size() == 0) {
        // Set default axis ranges for empty data
        axisX_->setRange(0, 1);
        axisY_->setRange(0, 1);
        return;
    }

    // Collect and sort points by time for proper line drawing
    QVector<QPair<double, double>> points;
    for (size_t i = 0; i < timeSeries_->size(); ++i) {
        points.append({timeSeries_->getTime(i), timeSeries_->getValue(i)});
    }

    std::sort(points.begin(), points.end(),
              [](const QPair<double, double>& a, const QPair<double, double>& b) {
                  return a.first < b.first;
              });

    // Add points to series
    for (const auto& point : points) {
        lineSeries_->append(point.first, point.second);
        if (showMarkers_) {
            scatterSeries_->append(point.first, point.second);
        }
    }

    updateAxisRanges();
}

void TimeSeriesChartWidget::updateAxisRanges()
{
    if (!timeSeries_ || timeSeries_->size() == 0) {
        return;
    }

    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (size_t i = 0; i < timeSeries_->size(); ++i) {
        double t = timeSeries_->getTime(i);
        double v = timeSeries_->getValue(i);

        minX = std::min(minX, t);
        maxX = std::max(maxX, t);
        minY = std::min(minY, v);
        maxY = std::max(maxY, v);
    }

    // Add some padding
    double rangeX = maxX - minX;
    double rangeY = maxY - minY;

    if (rangeX < 1e-10) {
        rangeX = 1.0;
        minX -= 0.5;
        maxX += 0.5;
    } else {
        minX -= rangeX * 0.05;
        maxX += rangeX * 0.05;
    }

    if (rangeY < 1e-10) {
        rangeY = 1.0;
        minY -= 0.5;
        maxY += 0.5;
    } else {
        minY -= rangeY * 0.05;
        maxY += rangeY * 0.05;
    }

    axisX_->setRange(minX, maxX);
    axisY_->setRange(minY, maxY);
}

void TimeSeriesChartWidget::setChartTitle(const QString& title)
{
    chart_->setTitle(title);
}

void TimeSeriesChartWidget::setAxisLabels(const QString& xLabel, const QString& yLabel)
{
    axisX_->setTitleText(xLabel);
    axisY_->setTitleText(yLabel);
}

void TimeSeriesChartWidget::setShowMarkers(bool show)
{
    showMarkers_ = show;
    scatterSeries_->setVisible(show);
    updateChart();
}

void TimeSeriesChartWidget::resetZoom()
{
    chart_->zoomReset();
    updateAxisRanges();
}

void TimeSeriesChartWidget::onDataChanged()
{
    updateChart();
}
