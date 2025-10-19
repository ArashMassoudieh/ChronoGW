#ifndef WELLDIALOG_H
#define WELLDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QVector>
#include "Well.h"
#include "GWA.h"
#include "parameterorvaluewidget.h"

/**
 * @brief Dialog for editing CWell properties
 */
class WellDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WellDialog(CWell* well, CGWA* gwa, QWidget *parent = nullptr);

    /**
     * @brief Get the modified well data
     * @return CWell object with updated properties
     */
    CWell getWell() const;

    /**
     * @brief Get parameter linkages (which well properties are linked to which parameters)
     * @return Map of property name to parameter name
     */
    QMap<QString, QString> getParameterLinkages() const;

private slots:
    void onDistributionTypeChanged(const QString& type);
    void onAccepted();

private:
    void setupUI();
    void loadWellData();
    void createDistributionParameterInputs();

    /**
     * @brief Find which parameter (if any) is linked to a well property
     * @param wellName Name of the well
     * @param quantity Property name (e.g., "f", "age_old", "param[0]")
     * @return Parameter name if linked, empty string otherwise
     */
    QString findLinkedParameter(const std::string& wellName, const std::string& quantity);

    CWell* well_;
    CGWA* gwa_;

    // UI Elements - Basic Properties
    QLineEdit* nameEdit;
    QComboBox* distributionTypeCombo;

    // UI Elements - Mixing Parameters (now with parameter selection)
    ParameterOrValueWidget* fractionOldWidget;
    ParameterOrValueWidget* ageOldWidget;
    ParameterOrValueWidget* fractionMineralWidget;
    ParameterOrValueWidget* vzDelayWidget;

    // UI Elements - Distribution Parameters
    QGroupBox* distributionParamsGroup;
    QFormLayout* distributionParamsLayout;
    QVector<ParameterOrValueWidget*> distributionParamWidgets;

    // Available distribution types
    QStringList distributionTypes;
};

#endif // WELLDIALOG_H
