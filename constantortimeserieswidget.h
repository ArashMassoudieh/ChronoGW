#ifndef CONSTANTORTIMESERIESWIDGET_H
#define CONSTANTORTIMESERIESWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QString>
#include "TimeSeries.h"

/**
 * @brief Widget for entering either a constant value or loading a time series
 *
 * Layout: [LineEdit][Load File Button][View Chart Button]
 *
 * - LineEdit: For entering constant values or showing loaded filename
 * - Load Button: Opens file dialog to load time series
 * - Chart Button: Shows chart of loaded time series (disabled if no data)
 */
class ConstantOrTimeSeriesWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit ConstantOrTimeSeriesWidget(QWidget *parent = nullptr);

    /**
     * @brief Set a constant value
     * @param value The constant value
     */
    void setConstantValue(double value);

    /**
     * @brief Set a time series from file
     * @param filename Path to time series file
     */
    void setTimeSeriesFile(const QString& filename);

    /**
     * @brief Set a time series directly
     * @param timeSeries The time series data
     */
    void setTimeSeries(const TimeSeries<double>& timeSeries);

    /**
     * @brief Check if widget is in constant mode
     * @return true if using constant value, false if using time series
     */
    bool isConstantMode() const;

    /**
     * @brief Get the constant value
     * @return The constant value (0.0 if in time series mode)
     */
    double getConstantValue() const;

    /**
     * @brief Get the time series filename
     * @return The filename (empty if in constant mode)
     */
    QString getTimeSeriesFilename() const;

    /**
     * @brief Get the loaded time series
     * @return The time series data
     */
    const TimeSeries<double>& getTimeSeries() const;

    /**
     * @brief Check if time series has been loaded
     * @return true if time series data exists
     */
    bool hasTimeSeries() const;

    /**
     * @brief Set placeholder text for the line edit
     * @param text Placeholder text
     */
    void setPlaceholderText(const QString& text);

    /**
     * @brief Enable or disable the widget
     * @param enabled true to enable, false to disable
     */
    void setEnabled(bool enabled);

protected:
    /**
     * @brief Handle double-click to switch back to constant mode
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    /**
     * @brief Emitted when the value or time series changes
     */
    void valueChanged();

    /**
     * @brief Emitted when a time series is loaded
     */
    void timeSeriesLoaded();

private slots:
    void onLineEditChanged();
    void onLoadButtonClicked();
    void onViewButtonClicked();

private:
    QLineEdit* lineEdit;
    QPushButton* loadButton;
    QPushButton* viewButton;
    QHBoxLayout* layout;

    bool isConstant;
    QString timeSeriesFilename;
    TimeSeries<double> timeSeries;

    void updateViewButtonState();
};

#endif // CONSTANTORTIMESERIESWIDGET_H
