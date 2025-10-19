#ifndef VALUEORPARAMETERWIDGET_H
#define VALUEORPARAMETERWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include "GWA.h"

/**
 * @brief Widget for entering either a numeric value OR selecting a parameter
 *
 * The user can either:
 * - Type a number directly in the line edit
 * - Select a parameter from the combo box
 *
 * The widget intelligently determines which mode based on user interaction.
 */
class ValueOrParameterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ValueOrParameterWidget(CGWA* gwa, QWidget *parent = nullptr);

    /**
     * @brief Set to value mode with specified value
     * @param value Numeric value
     */
    void setValue(double value);

    /**
     * @brief Set to parameter mode with specified parameter
     * @param paramName Parameter name
     */
    void setParameter(const QString& paramName);

    /**
     * @brief Check if widget is in parameter mode
     * @return true if a parameter is selected
     */
    bool isParameter() const;

    /**
     * @brief Get the current value
     * @return Value (either from line edit or from selected parameter)
     */
    double getValue() const;

    /**
     * @brief Get the selected parameter name
     * @return Parameter name if in parameter mode, empty string otherwise
     */
    QString getParameter() const;

    /**
     * @brief Set placeholder text for the line edit
     */
    void setPlaceholder(const QString& text);

    /**
     * @brief Set suffix for display (e.g., " years", " m/day")
     */
    void setSuffix(const QString& suffix);

signals:
    void valueChanged();
    void parameterChanged();

private slots:
    void onLineEditChanged();
    void onComboBoxChanged(int index);

private:
    void setupUI();
    void populateParameters();

    CGWA* gwa_;
    QLineEdit* lineEdit_;
    QComboBox* paramCombo_;
    QString suffix_;

    bool updatingInternally_; // Flag to prevent infinite recursion
};

#endif // VALUEORPARAMETERWIDGET_H
