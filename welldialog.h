#ifndef WELLDIALOG_H
#define WELLDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QVector>
#include <QString>
#include <QLabel>

class CGWA;
class CWell;
class ParameterValueWidget;

/**
 * @brief Dialog for editing well properties
 */
class WellDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param gwa GWA model (for parameter list)
     * @param well Well to edit (nullptr for new well)
     * @param parent Parent widget
     */
    WellDialog(CGWA* gwa, CWell* well = nullptr, QWidget* parent = nullptr);

    /**
     * @brief Get the well with user modifications
     * @return Modified well object
     */
    CWell getWell() const;

    /**
     * @brief Get parameter linkages for this well
     * @return Map of property names to parameter names
     */
    QMap<QString, QString> getParameterLinkages() const;

private slots:
    void onDistributionTypeChanged(const QString& type);
    void onAccepted();
    void onPlotDistribution();
    /**
     * @brief Plot MCMC realizations for this well
     */
    void onPlotRealizations();

    /**
     * @brief Plot MCMC prediction intervals (percentiles)
     */
    void onPlotPercentiles();

private:
    void setupUI();
    void loadWellData(const CWell* well);
    void updateParameterWidgets();
    void createDistributionParamWidgets(int count);
    QString findLinkedParameter(const std::string& wellName, const std::string& quantity);


    // Model reference
    CGWA* gwa_;
    CWell* well_;

    // Available distribution types
    QStringList distributionTypes;

    // UI Widgets
    QLineEdit* nameEdit;
    QComboBox* distributionTypeCombo;
    QGroupBox* distributionParamsGroup;
    QFormLayout* distributionParamsLayout;
    QVector<ParameterValueWidget*> distributionParamWidgets;
    QPushButton* plotDistributionButton_;
    QPushButton* plotRealizationsButton_;
    QPushButton* plotPercentilesButton_;

    ParameterValueWidget* fractionOldWidget;
    ParameterValueWidget* ageOldWidget;
    ParameterValueWidget* fractionMineralWidget;
    ParameterValueWidget* vzDelayWidget;

    QLabel* distributionInfoLabel;
};

#endif // WELLDIALOG_H
