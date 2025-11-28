#ifndef TIMESERIESPOINTDIALOG_H
#define TIMESERIESPOINTDIALOG_H

#include <QDialog>

class QDoubleSpinBox;
class QLabel;

/**
 * @brief Dialog for adding or editing a single time series point
 *
 * Provides input fields for time and value with validation.
 */
class TimeSeriesPointDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Construct dialog for adding/editing a point
     * @param parent Parent widget
     */
    explicit TimeSeriesPointDialog(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~TimeSeriesPointDialog() override = default;

    /**
     * @brief Set dialog for edit mode with existing values
     * @param time Current time value
     * @param value Current data value
     */
    void setEditMode(double time, double value);

    /**
     * @brief Set dialog for add mode
     * @param suggestedTime Suggested time value (optional)
     */
    void setAddMode(double suggestedTime = 0.0);

    /**
     * @brief Get the entered time value
     * @return Time value
     */
    double getTime() const;

    /**
     * @brief Get the entered data value
     * @return Data value
     */
    double getValue() const;

    /**
     * @brief Set the valid time range
     * @param min Minimum time value
     * @param max Maximum time value
     */
    void setTimeRange(double min, double max);

    /**
     * @brief Set the valid value range
     * @param min Minimum value
     * @param max Maximum value
     */
    void setValueRange(double min, double max);

    /**
     * @brief Set number of decimal places for display
     * @param timeDecimals Decimals for time field
     * @param valueDecimals Decimals for value field
     */
    void setDecimals(int timeDecimals, int valueDecimals);

private:
    void setupUi();

    QDoubleSpinBox* timeSpinBox_ = nullptr;
    QDoubleSpinBox* valueSpinBox_ = nullptr;
    bool editMode_ = false;
};

#endif // TIMESERIESPOINTDIALOG_H
