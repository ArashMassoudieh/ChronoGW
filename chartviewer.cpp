#include "chartviewer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QLegendMarker>
#include <QPixmap>
#include <QDebug>

ChartViewer::ChartViewer(QWidget *parent)
    : QWidget(parent)
    , chart_(new QChart())
    , chartView_(new QChartView(chart_, this))
    , xAxis_(nullptr)
    , yAxis_(nullptr)
    , xAxisLog_(false)
    , yAxisLog_(false)
    , plotMode_(Lines)
    , xAxisLabel_("Time")
    , yAxisLabel_("Value")
{
    setupUI();
    initializeColorPalette();
}

ChartViewer::~ChartViewer()
{
    // QChart takes ownership of series and axes, so they're automatically deleted
}

void ChartViewer::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);

    // Setup toolbar
    setupToolbar();
    mainLayout_->addWidget(toolbar_);

    // Setup chart view
    chartView_->setRenderHint(QPainter::Antialiasing);
    chartView_->setRubberBand(QChartView::RectangleRubberBand);
    chart_->legend()->setVisible(true);
    chart_->legend()->setAlignment(Qt::AlignRight);

    // Legend marker clicks will be connected after series are created

    mainLayout_->addWidget(chartView_);

    setLayout(mainLayout_);
}

void ChartViewer::setupToolbar()
{
    toolbar_ = new QToolBar(this);
    toolbar_->setIconSize(QSize(24, 24));

    // Copy action
    copyAction_ = toolbar_->addAction(QIcon::fromTheme("edit-copy"), "Copy Data");
    copyAction_->setToolTip("Copy chart data to clipboard");
    connect(copyAction_, &QAction::triggered, this, &ChartViewer::onCopy);

    // Paste action
    pasteAction_ = toolbar_->addAction(QIcon::fromTheme("edit-paste"), "Paste Data");
    pasteAction_->setToolTip("Paste chart data from clipboard");
    connect(pasteAction_, &QAction::triggered, this, &ChartViewer::onPaste);

    toolbar_->addSeparator();

    // Export PNG action
    exportPngAction_ = toolbar_->addAction(QIcon::fromTheme("image-x-generic"), "Export PNG");
    exportPngAction_->setToolTip("Export chart to PNG image");
    connect(exportPngAction_, &QAction::triggered, this, &ChartViewer::onExportPng);

    // Export CSV action
    exportCsvAction_ = toolbar_->addAction(QIcon::fromTheme("text-csv"), "Export CSV");
    exportCsvAction_->setToolTip("Export data to CSV file");
    connect(exportCsvAction_, &QAction::triggered, this, &ChartViewer::onExportCsv);

    toolbar_->addSeparator();

    // X-axis log toggle
    xLogAction_ = toolbar_->addAction("X: Linear");
    xLogAction_->setToolTip("Toggle X-axis between linear and logarithmic scale");
    xLogAction_->setCheckable(true);
    connect(xLogAction_, &QAction::triggered, this, &ChartViewer::onToggleXLog);

    // Y-axis log toggle
    yLogAction_ = toolbar_->addAction("Y: Linear");
    yLogAction_->setToolTip("Toggle Y-axis between linear and logarithmic scale");
    yLogAction_->setCheckable(true);
    connect(yLogAction_, &QAction::triggered, this, &ChartViewer::onToggleYLog);

    toolbar_->addSeparator();

    // Plot mode toggle
    plotModeAction_ = toolbar_->addAction("Lines");
    plotModeAction_->setToolTip("Toggle between lines, symbols, or both");
    connect(plotModeAction_, &QAction::triggered, this, &ChartViewer::onTogglePlotMode);
}

void ChartViewer::initializeColorPalette()
{
    // Define a nice color palette for series
    colorPalette_ << QColor(31, 119, 180)   // Blue
                  << QColor(255, 127, 14)   // Orange
                  << QColor(44, 160, 44)    // Green
                  << QColor(214, 39, 40)    // Red
                  << QColor(148, 103, 189)  // Purple
                  << QColor(140, 86, 75)    // Brown
                  << QColor(227, 119, 194)  // Pink
                  << QColor(127, 127, 127)  // Gray
                  << QColor(188, 189, 34)   // Olive
                  << QColor(23, 190, 207);  // Cyan
}

void ChartViewer::setData(const TimeSeriesSet<double>& data)
{
    timeSeriesData_ = data;
    updateChart();
}

void ChartViewer::setTitle(const QString& title)
{
    chart_->setTitle(title);
}

void ChartViewer::setAxisLabels(const QString& xLabel, const QString& yLabel)
{
    xAxisLabel_ = xLabel;
    yAxisLabel_ = yLabel;

    if (xAxis_) xAxis_->setTitleText(xLabel);
    if (yAxis_) yAxis_->setTitleText(yLabel);
}

void ChartViewer::setPlotMode(PlotMode mode)
{
    plotMode_ = mode;
    updateChart();

    // Update button text
    switch (plotMode_) {
    case Lines:
        plotModeAction_->setText("Lines");
        break;
    case Symbols:
        plotModeAction_->setText("Symbols");
        break;
    case LinesAndSymbols:
        plotModeAction_->setText("Lines+Symbols");
        break;
    }
}

void ChartViewer::updateChart()
{
    // Clear existing series
    chart_->removeAllSeries();
    lineSeries_.clear();
    scatterSeries_.clear();

    if (timeSeriesData_.empty()) {
        return;
    }

    // Create series
    createSeries();

    // Setup axes
    setupAxes();

    // Update ranges
    updateAxesRanges();

    // Apply colors
    applySeriesColors();
}

void ChartViewer::createSeries()
{
    for (size_t i = 0; i < timeSeriesData_.size(); ++i) {
        const TimeSeries<double>& ts = timeSeriesData_[i];
        QString seriesName = QString::fromStdString(ts.name());

        if (seriesName.isEmpty()) {
            seriesName = QString("Series %1").arg(i);
        }

        // Initialize visibility (default: visible)
        if (!seriesVisibility_.contains(seriesName)) {
            seriesVisibility_[seriesName] = true;
        }

        bool visible = seriesVisibility_[seriesName];

        // Create line series if needed
        if (plotMode_ == Lines || plotMode_ == LinesAndSymbols) {
            QLineSeries* lineSeries = new QLineSeries();
            lineSeries->setName(seriesName);

            for (size_t j = 0; j < ts.size(); ++j) {
                double x = ts.getTime(j);
                double y = ts.getValue(j);

                // Skip invalid values for log scales
                if ((xAxisLog_ && x <= 0) || (yAxisLog_ && y <= 0)) {
                    continue;
                }

                lineSeries->append(x, y);
            }

            lineSeries->setVisible(visible);
            chart_->addSeries(lineSeries);
            lineSeries_[seriesName] = lineSeries;
        }

        // Create scatter series if needed
        if (plotMode_ == Symbols || plotMode_ == LinesAndSymbols) {
            QScatterSeries* scatterSeries = new QScatterSeries();
            scatterSeries->setName(seriesName + " (points)");
            scatterSeries->setMarkerSize(8.0);

            for (size_t j = 0; j < ts.size(); ++j) {
                double x = ts.getTime(j);
                double y = ts.getValue(j);

                // Skip invalid values for log scales
                if ((xAxisLog_ && x <= 0) || (yAxisLog_ && y <= 0)) {
                    continue;
                }

                scatterSeries->append(x, y);
            }

            scatterSeries->setVisible(visible);
            chart_->addSeries(scatterSeries);
            scatterSeries_[seriesName] = scatterSeries;
        }
    }
}

void ChartViewer::setupAxes()
{
    // Remove old axes
    if (xAxis_) {
        chart_->removeAxis(xAxis_);
        delete xAxis_;
    }
    if (yAxis_) {
        chart_->removeAxis(yAxis_);
        delete yAxis_;
    }

    // Create X-axis
    if (xAxisLog_) {
        QLogValueAxis* logAxis = new QLogValueAxis();
        logAxis->setLabelFormat("%g");
        logAxis->setBase(10.0);
        xAxis_ = logAxis;
    } else {
        QValueAxis* valueAxis = new QValueAxis();
        valueAxis->setLabelFormat("%g");
        xAxis_ = valueAxis;
    }
    xAxis_->setTitleText(xAxisLabel_);

    // Create Y-axis
    if (yAxisLog_) {
        QLogValueAxis* logAxis = new QLogValueAxis();
        logAxis->setLabelFormat("%g");
        logAxis->setBase(10.0);
        yAxis_ = logAxis;
    } else {
        QValueAxis* valueAxis = new QValueAxis();
        valueAxis->setLabelFormat("%g");
        yAxis_ = valueAxis;
    }
    yAxis_->setTitleText(yAxisLabel_);

    // Add axes to chart
    chart_->addAxis(xAxis_, Qt::AlignBottom);
    chart_->addAxis(yAxis_, Qt::AlignLeft);

    // Attach series to axes
    for (QLineSeries* series : lineSeries_.values()) {
        series->attachAxis(xAxis_);
        series->attachAxis(yAxis_);
    }
    for (QScatterSeries* series : scatterSeries_.values()) {
        series->attachAxis(xAxis_);
        series->attachAxis(yAxis_);
    }
}

void ChartViewer::updateAxesRanges()
{
    if (timeSeriesData_.empty()) return;

    double xMin = 1e100, xMax = -1e100;
    double yMin = 1e100, yMax = -1e100;

    for (size_t i = 0; i < timeSeriesData_.size(); ++i) {
        const TimeSeries<double>& ts = timeSeriesData_[i];

        for (size_t j = 0; j < ts.size(); ++j) {
            double x = ts.getTime(j);
            double y = ts.getValue(j);

            // Skip invalid values for log scales
            if (xAxisLog_ && x <= 0) continue;
            if (yAxisLog_ && y <= 0) continue;

            xMin = std::min(xMin, x);
            xMax = std::max(xMax, x);
            yMin = std::min(yMin, y);
            yMax = std::max(yMax, y);
        }
    }

    // Add some padding
    double xRange = xMax - xMin;
    double yRange = yMax - yMin;

    if (xAxisLog_) {
        xMin *= 0.9;
        xMax *= 1.1;
    } else {
        xMin -= xRange * 0.05;
        xMax += xRange * 0.05;
    }

    if (yAxisLog_) {
        yMin *= 0.9;
        yMax *= 1.1;
    } else {
        yMin -= yRange * 0.05;
        yMax += yRange * 0.05;
    }

    // Set ranges
    if (QValueAxis* axis = qobject_cast<QValueAxis*>(xAxis_)) {
        axis->setRange(xMin, xMax);
    } else if (QLogValueAxis* axis = qobject_cast<QLogValueAxis*>(xAxis_)) {
        axis->setRange(xMin, xMax);
    }

    if (QValueAxis* axis = qobject_cast<QValueAxis*>(yAxis_)) {
        axis->setRange(yMin, yMax);
    } else if (QLogValueAxis* axis = qobject_cast<QLogValueAxis*>(yAxis_)) {
        axis->setRange(yMin, yMax);
    }
}

void ChartViewer::applySeriesColors()
{
    int colorIndex = 0;

    for (auto it = lineSeries_.begin(); it != lineSeries_.end(); ++it) {
        QColor color = colorPalette_[colorIndex % colorPalette_.size()];
        it.value()->setColor(color);

        // If scatter series exists with same name, use same color
        if (scatterSeries_.contains(it.key())) {
            scatterSeries_[it.key()]->setColor(color);
            scatterSeries_[it.key()]->setBorderColor(color.darker(120));
        }

        colorIndex++;
    }

    // Connect legend marker signals after series are added
    connectLegendMarkers();
}

void ChartViewer::connectLegendMarkers()
{
    // Connect click signal for each legend marker
    const auto markers = chart_->legend()->markers();
    for (QLegendMarker* marker : markers) {
        connect(marker, &QLegendMarker::clicked,
                this, &ChartViewer::onLegendMarkerClicked);
    }
}

void ChartViewer::setXAxisLog(bool useLog)
{
    xAxisLog_ = useLog;
    updateChart();
    xLogAction_->setChecked(useLog);
    xLogAction_->setText(useLog ? "X: Log" : "X: Linear");
}

void ChartViewer::setYAxisLog(bool useLog)
{
    yAxisLog_ = useLog;
    updateChart();
    yLogAction_->setChecked(useLog);
    yLogAction_->setText(useLog ? "Y: Log" : "Y: Linear");
}

void ChartViewer::setSeriesVisible(const QString& seriesName, bool visible)
{
    seriesVisibility_[seriesName] = visible;

    if (lineSeries_.contains(seriesName)) {
        lineSeries_[seriesName]->setVisible(visible);
    }
    if (scatterSeries_.contains(seriesName)) {
        scatterSeries_[seriesName]->setVisible(visible);
    }

    emit seriesVisibilityChanged(seriesName, visible);
}

void ChartViewer::exportToPng(const QString& filename)
{
    QPixmap pixmap = chartView_->grab();
    if (!pixmap.save(filename, "PNG")) {
        QMessageBox::warning(this, "Export Failed",
                             "Failed to export chart to PNG file.");
    }
}

void ChartViewer::exportToCsv(const QString& filename)
{
    if (timeSeriesData_.empty()) {
        QMessageBox::warning(this, "Export Failed",
                             "No data to export.");
        return;
    }

    // Use TimeSeriesSet's write method
    timeSeriesData_.write(filename.toStdString(), ",");

    QMessageBox::information(this, "Export Successful",
                             QString("Exported %1 time series to CSV file:\n%2")
                                 .arg(timeSeriesData_.size())
                                 .arg(filename));
}

void ChartViewer::onCopy()
{
    clipboard_ = timeSeriesData_;
    QMessageBox::information(this, "Copy",
                             QString("Copied %1 time series to clipboard").arg(clipboard_.size()));
}

void ChartViewer::onPaste()
{
    if (clipboard_.empty()) {
        QMessageBox::warning(this, "Paste", "Clipboard is empty");
        return;
    }

    setData(clipboard_);
    QMessageBox::information(this, "Paste",
                             QString("Pasted %1 time series from clipboard").arg(clipboard_.size()));
}

void ChartViewer::onExportPng()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        tr("Export Chart to PNG"),
        QString(),
        tr("PNG Images (*.png);;All Files (*)")
        );

    if (!filename.isEmpty()) {
        exportToPng(filename);
    }
}

void ChartViewer::onExportCsv()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        tr("Export Data to CSV"),
        QString(),
        tr("CSV Files (*.csv);;Text Files (*.txt);;All Files (*)")
        );

    if (!filename.isEmpty()) {
        exportToCsv(filename);
    }
}

void ChartViewer::onToggleXLog()
{
    setXAxisLog(!xAxisLog_);
}

void ChartViewer::onToggleYLog()
{
    setYAxisLog(!yAxisLog_);
}

void ChartViewer::onTogglePlotMode()
{
    // Cycle through modes: Lines -> Symbols -> LinesAndSymbols -> Lines
    switch (plotMode_) {
    case Lines:
        setPlotMode(Symbols);
        break;
    case Symbols:
        setPlotMode(LinesAndSymbols);
        break;
    case LinesAndSymbols:
        setPlotMode(Lines);
        break;
    }
}

void ChartViewer::onLegendMarkerClicked()
{
    QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
    if (!marker) return;

    QString seriesName = marker->series()->name();

    // Toggle visibility
    bool currentlyVisible = marker->series()->isVisible();
    bool newVisible = !currentlyVisible;

    marker->series()->setVisible(newVisible);
    seriesVisibility_[seriesName] = newVisible;

    // Update marker appearance (dim when hidden)
    if (newVisible) {
        marker->setVisible(true);
        QFont font = marker->font();
        font.setStrikeOut(false);
        marker->setFont(font);
    } else {
        QFont font = marker->font();
        font.setStrikeOut(true);
        marker->setFont(font);
    }

    emit seriesVisibilityChanged(seriesName, newVisible);
}
