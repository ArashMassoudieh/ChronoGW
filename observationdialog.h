#ifndef OBSERVATIONDIALOG_H
#define OBSERVATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>

class CGWA;
class Observation;

/**
 * @brief Dialog for editing observation properties
 */
class ObservationDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param gwa GWA model (for getting well/tracer/parameter lists)
     * @param obs Observation to edit (nullptr for new observation)
     * @param parent Parent widget
     */
    ObservationDialog(CGWA* gwa, Observation* obs = nullptr, QWidget* parent = nullptr);

    /**
     * @brief Get the observation with user modifications
     * @return Modified observation object
     */
    Observation getObservation() const;

private slots:
    void onAccepted();
    void onBrowseObservedData();
    void onDetectionLimitToggled(bool checked);
    void onPlotObservedData();

private:
    void setupUI();
    void loadObservationData(const Observation* obs);
    void updateParameterList();

    // Model reference
    CGWA* gwa_;
    Observation* obs_;

    // UI Widgets
    QLineEdit* nameEdit_;
    QComboBox* wellCombo_;
    QComboBox* tracerCombo_;
    QComboBox* stdParameterCombo_;
    QComboBox* errorStructureCombo_;

    QLabel* observedDataLabel_;
    QPushButton* browseDataButton_;
    QString observedDataPath_;

    QCheckBox* detectionLimitCheckBox_;
    QDoubleSpinBox* detectionLimitSpinBox_;
    QCheckBox* countMaxCheckBox_;
};

#endif // OBSERVATIONDIALOG_H
