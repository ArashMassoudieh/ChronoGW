#ifndef PARAMETERVALUEWIDGET_H
#define PARAMETERVALUEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QString>
#include <QVector>

/**
 * @brief Widget combining a value input field and a parameter selector
 *
 * This widget provides a horizontal layout with:
 * - A QLineEdit for entering numeric values
 * - A QComboBox for selecting from available parameters
 *
 * It can operate in two modes:
 * 1. Value mode: User enters a direct numeric value
 * 2. Parameter mode: User selects a parameter name from the combo box
 */
class ParameterValueWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit ParameterValueWidget(QWidget *parent = nullptr);

    /**
     * @brief Set the available parameter names for the combo box
     * @param parameterNames List of parameter names
     */
    void setAvailableParameters(const QVector<QString>& parameterNames);

    /**
     * @brief Set a numeric value
     * @param value The value to set
     */
    void setValue(double value);

    /**
     * @brief Set a parameter name (selects it in combo box)
     * @param parameterName The parameter name to select
     */
    void setParameter(const QString& parameterName);

    /**
     * @brief Check if currently in parameter mode
     * @return true if a parameter is selected, false if using value
     */
    bool isParameterMode() const;

    /**
     * @brief Get the current value
     * @return The numeric value (0.0 if in parameter mode)
     */
    double getValue() const;

    /**
     * @brief Get the selected parameter name
     * @return The parameter name (empty string if in value mode)
     */
    QString getParameter() const;

    /**
     * @brief Enable or disable the widget
     * @param enabled true to enable, false to disable
     */
    void setEnabled(bool enabled);

    /**
     * @brief Set placeholder text for the line edit
     * @param text Placeholder text
     */
    void setPlaceholderText(const QString& text);

signals:
    /**
     * @brief Emitted when the value or parameter selection changes
     */
    void valueChanged();

private slots:
    void onLineEditChanged();
    void onComboBoxChanged(int index);

private:
    QLineEdit* lineEdit;
    QComboBox* comboBox;
    QHBoxLayout* layout;

    bool updatingInternally;  // Flag to prevent recursive updates
};

#endif // PARAMETERVALUEWIDGET_H
