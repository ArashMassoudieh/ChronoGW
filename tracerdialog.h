#ifndef TRACERDIALOG_H
#define TRACERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QString>

class CGWA;
class CTracer;
class ParameterValueWidget;
class ConstantOrTimeSeriesWidget;

/**
 * @brief Dialog for editing tracer properties
 */
class TracerDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param gwa GWA model (for parameter list and source tracers)
     * @param tracer Tracer to edit (nullptr for new tracer)
     * @param parent Parent widget
     */
    TracerDialog(CGWA* gwa, CTracer* tracer = nullptr, QWidget* parent = nullptr);

    /**
     * @brief Get the tracer with user modifications
     * @return Modified tracer object
     */
    CTracer getTracer() const;

    /**
     * @brief Get parameter linkages for this tracer
     * @return Map of property names to parameter names
     */
    QMap<QString, QString> getParameterLinkages() const;

private slots:
    void onAccepted();

private:
    void setupUI();
    void loadTracerData(const CTracer* tracer);
    void updateParameterWidgets();
    QString findLinkedParameter(const std::string& tracerName, const std::string& quantity);
    void removeParameterLinkage(const std::string& tracerName, const std::string& quantity);
    // Model reference
    CGWA* gwa_;
    CTracer* tracer_;

    // UI Widgets - Basic Properties
    QLineEdit* nameEdit;
    ConstantOrTimeSeriesWidget* inputWidget;

    // UI Widgets - Transport Properties
    ParameterValueWidget* inputMultiplierWidget;
    ParameterValueWidget* decayRateWidget;
    ParameterValueWidget* retardationWidget;

    // UI Widgets - Water Component Concentrations
    ParameterValueWidget* oldWaterConcWidget;
    ParameterValueWidget* modernWaterConcWidget;
    ParameterValueWidget* maxFractionMineralWidget;

    // UI Widgets - Options
    QCheckBox* vzDelayCheckBox;
    QCheckBox* linearProductionCheckBox;

    // UI Widgets - Source Tracer
    QComboBox* sourceTracerCombo;
};

#endif // TRACERDIALOG_H
