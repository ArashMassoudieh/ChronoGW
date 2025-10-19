#ifndef PARAMETERORVALUEWIDGET_H
#define PARAMETERORVALUEWIDGET_H

#include <QWidget>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QRadioButton>
#include <QStackedWidget>
#include "GWA.h"

/**
 * @brief Widget for entering a value OR selecting a parameter
 *
 * This widget provides a dual-mode input:
 * - Direct value entry via QDoubleSpinBox
 * - Parameter selection via QComboBox (populated from GWA model)
 *
 * The user selects the mode with radio buttons, and the appropriate
 * input widget is shown.
 */
class ParameterOrValueWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterOrValueWidget(CGWA* gwa, const QString& label, QWidget* parent = nullptr);

    /**
     * @brief Set widget to value mode with specified value
     * @param value The numeric value
     */
    void setValue(double value);

    /**
     * @brief Set widget to parameter mode with specified parameter
     * @param paramName The parameter name to select
     */
    void setParameter(const QString& paramName);

    /**
     * @brief Check if widget is in parameter mode
     * @return true if using parameter, false if using direct value
     */
    bool isParameter() const { return useParameter; }

    /**
     * @brief Get the current value (either direct or from selected parameter)
     * @return The numeric value
     */
    double getValue() const;

    /**
     * @brief Get the selected parameter name
     * @return Parameter name if in parameter mode, empty string otherwise
     */
    QString getParameter() const;

    // Public access to spinbox for customization
    QDoubleSpinBox* valueSpin;
    QComboBox* paramCombo;

signals:
    /**
     * @brief Emitted when value or parameter selection changes
     */
    void valueChanged();

private:
    void setupUI(const QString& label);
    void updateMode();

    CGWA* gwa_;
    bool useParameter;

    QRadioButton* valueRadio;
    QRadioButton* paramRadio;
    QStackedWidget* inputStack;
};

#endif // PARAMETERORVALUEWIDGET_H
