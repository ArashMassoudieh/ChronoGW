#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>

class CGWA;
class Parameter;

/**
 * @brief Dialog for editing parameter properties
 */
class ParameterDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param gwa GWA model (for checking duplicate names)
     * @param param Parameter to edit (nullptr for new parameter)
     * @param parent Parent widget
     */
    ParameterDialog(CGWA* gwa, Parameter* param = nullptr, QWidget* parent = nullptr);

    /**
     * @brief Get the parameter with user modifications
     * @return Modified parameter object
     */
    Parameter getParameter() const;

private slots:
    void onAccepted();
    void onPriorDistributionChanged(const QString& dist);
    void onPlotPriorDistribution();

private:
    void setupUI();
    void loadParameterData(const Parameter* param);

    // Model reference
    CGWA* gwa_;
    Parameter* param_;

    // UI Widgets
    QLineEdit* nameEdit_;
    QDoubleSpinBox* lowSpinBox_;
    QDoubleSpinBox* highSpinBox_;
    QDoubleSpinBox* valueSpinBox_;
    QComboBox* priorDistributionCombo_;
    QDoubleSpinBox* priorMeanSpinBox_;
    QDoubleSpinBox* priorStdSpinBox_;
    QPushButton* plotPriorButton_;

    QGroupBox* priorParamsGroup_;
};

#endif // PARAMETERDIALOG_H
