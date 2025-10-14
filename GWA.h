#pragma once

#include "Tracer.h"
#include "Well.h"
#include "TimeSeries.h"
#include "TimeSeriesSet.h"
#include <vector>
#include <string>
#include <map>
#include <memory>
#include "parameter_set.h"
#include "observation.h"



/**
 * @brief Model configuration and settings
 */
struct ModelSettings
{
    std::string input_path;              ///< Input file directory
    std::string output_path;             ///< Output file directory
    std::string pe_info_filename;        ///< Parameter estimation info file
    std::string det_output_filename;     ///< Deterministic output file
    std::string realized_param_filename; ///< Realized parameters file

    bool single_vz_delay;                ///< Use single vadose zone delay for all tracers
    bool fixed_old_tracer;               ///< Use fixed old tracer concentration

    bool project_enabled;                ///< Whether to do forward projection
    double project_start;                ///< Projection start time
    double project_finish;               ///< Projection end time
    double project_interval;             ///< Projection time step

    ModelSettings()
        : single_vz_delay(false), fixed_old_tracer(false)
        , project_enabled(false), project_start(2020.0)
        , project_finish(2040.0), project_interval(1.0) {}
};

/**
 * @brief Main groundwater age modeling class
 *
 * This class manages:
 * - Tracers and wells
 * - Parameters for inverse modeling
 * - Observed data
 * - Forward modeling and projections
 * - Log-likelihood calculation for MCMC/optimization
 */
class CGWA
{
public:
    // ========================================================================
    // Constructors
    // ========================================================================

    CGWA();
    explicit CGWA(const std::string& config_filename);
    CGWA(const CGWA& other);
    CGWA& operator=(const CGWA& other);
    ~CGWA() = default;

    // ========================================================================
    // Configuration Loading
    // ========================================================================

    /**
     * @brief Load model from configuration file
     * @param filename Path to configuration file
     * @return true if successful
     */
    bool loadFromFile(const std::string& filename);

    // ========================================================================
    // Parameter Management
    // ========================================================================

    /**
     * @brief Get number of parameters
     */
    size_t getParameterCount() const { return parameters_.size(); }

    /**
     * @brief Get parameter by index
     */
    const Parameter* getParameter(size_t index) const;
    Parameter* getParameter(size_t index);

    /**
     * @brief Get all parameter values
     */
    const std::vector<double> getParameterValues() const;

    /**
     * @brief Get reference to parameter set
     * @return Reference to Parameter_Set
     */
    Parameter_Set& Parameters() { return parameters_; }

    /**
     * @brief Get const reference to parameter set
     * @return Const reference to Parameter_Set
     */
    const Parameter_Set& Parameters() const { return parameters_; }

    /**
     * @brief Set parameter value by index
     * @param index Parameter index
     * @param value New value
     */
    void setParameterValue(size_t index, double value);

    /**
     * @brief Set all parameter values at once
     * @param values Vector of parameter values
     */
    void setAllParameterValues(const std::vector<double>& values = vector<double>());

    /**
     * @brief Find parameter index by name
     * @return Index, or -1 if not found
     */
    int findParameter(const std::string& name) const;

    // ========================================================================
    // Model Components Access
    // ========================================================================

    size_t getTracerCount() const { return tracers_.size(); }
    size_t getWellCount() const { return wells_.size(); }
    size_t getObservationCount() const { return observations_.size(); }
    size_t ObservationsCount() const { return observations_.size(); }

    const CTracer& getTracer(size_t index) const { return tracers_[index]; }
    const CWell& getWell(size_t index) const { return wells_[index]; }
    const Observation& getObservation(size_t index) const { return observations_[index]; }

    /**
     * @brief Get pointer to observations vector as base Observation type
     * @return Pointer to vector of Observation
     */
    std::vector<Observation>* Observations() {
        return reinterpret_cast<std::vector<Observation>*>(&observations_);
    }

    /**
     * @brief Get const pointer to observations vector as base Observation type
     * @return Const pointer to vector of Observation
     */
    const std::vector<Observation>* Observations() const {
        return reinterpret_cast<const std::vector<Observation>*>(&observations_);
    }

    CTracer& getTracerMutable(size_t index) { return tracers_[index]; }
    CWell& getWellMutable(size_t index) { return wells_[index]; }

    int findTracer(const std::string& name) const;
    int findWell(const std::string& name) const;

    // ========================================================================
    // Forward Modeling
    // ========================================================================

    /**
     * @brief Run forward model to calculate concentrations at observation times
     */
    void runForwardModel(bool applyparameters = true);

    /**
     * @brief Get modeled concentrations (after calling runForwardModel)
     */
    const TimeSeriesSet<double>& getModeledData() const { return modeled_data_; }

    /**
     * @brief Run forward projection over specified time range
     */
    TimeSeriesSet<double> runProjection();

    /**
     * @brief Get projected concentrations (after calling runProjection)
     */
    const TimeSeriesSet<double>& getProjectedData() const { return projected_data_; }

    // ========================================================================
    // Inverse Modeling / Optimization
    // ========================================================================

    /**
     * @brief Calculate log-likelihood for current parameter values
     * @return Log-likelihood value
     *
     * Used by MCMC and optimization algorithms
     */
    double calculateLogLikelihood();

    /**
    * @brief Get observation standard deviations
    * @return Vector of std dev values corresponding to each observation
    */
    std::vector<double> getObservationStdDevs() const;

    /**
     * @brief Whether inverse modeling is enabled
     */
    bool isInverseModeEnabled() const { return inverse_enabled_; }

    // ========================================================================
    // Serialization / Output
    // ========================================================================

    /**
     * @brief Write all model parameters to string
     * @return String representation of all parameters
     */
    std::string parametersToString() const;

    /**
     * @brief Write model summary to output stream
     */
    void writeModelSummary(std::ostream& out) const;

    /**
     * @brief Write current parameter values to output stream
     */
    void writeParameterValues(std::ostream& out) const;

    // ========================================================================
    // Settings Access
    // ========================================================================

    const ModelSettings& getSettings() const { return settings_; }
    ModelSettings& getSettingsMutable() { return settings_; }

    /**
     * @brief Export model configuration to file
     * @param filename Output file path
     * @return true if successful
     */
    bool exportToFile(const std::string& filename) const;

    /**
     * @brief Get input path
     * @return Input file directory path
     */
    std::string InputPath() const { return settings_.input_path; }

    /**
     * @brief Set input path
     * @param path Input file directory path
     */
    void SetInputPath(const std::string& path) { settings_.input_path = path; }

    /**
     * @brief Get output path
     * @return Output file directory path
     */
    std::string OutputPath() const { return settings_.output_path; }

    /**
     * @brief Set output path
     * @param path Output file directory path
     */
    void SetOutputPath(const std::string& path) { settings_.output_path = path; }

private:
    // ========================================================================
    // Private Configuration Loading Methods
    // ========================================================================

    /**
     * @brief Parse configuration file into internal structure
     */
    bool parseConfigFile(const std::string& filename);

    /**
     * @brief Parse a parameter block from config file
     */
    void parseParameterBlock(const std::vector<std::string>& params);

    /**
     * @brief Load model settings from parsed config
     */
    void loadSettings();

    /**
     * @brief Load parameters from parsed config
     */
    void loadParameters();

    /**
     * @brief Load tracers from parsed config
     */
    void loadTracers();

    /**
     * @brief Load wells from parsed config
     */
    void loadWells();

    /**
     * @brief Load observed data from parsed config
     */
    void loadObservedData();

    /**
     * @brief Link tracers to their source tracers by name
     */
    void linkSourceTracers();

    // ========================================================================
    // Private Helper Methods
    // ========================================================================

    /**
     * @brief Apply parameter changes to wells and tracers
     */
    void applyParameterToModel(size_t param_index, double value);

    /**
     * @brief Update constant inputs for all tracers
     */
    void updateConstantInputs();

    /**
     * @brief Get oldest time from all tracer inputs
     */
    double getOldestInputTime() const;

    /**
     * @brief Calculate likelihood contribution from one observation
     */
    double calculateObservationLikelihood(size_t obs_index) const;

    // ========================================================================
    // Member Variables
    // ========================================================================

    // Model components
    std::vector<CTracer> tracers_;
    std::vector<CWell> wells_;
    std::vector<Observation> observations_;

    // Parameters for inverse modeling
    Parameter_Set parameters_;

    // Model results
    TimeSeriesSet<double> modeled_data_;
    TimeSeriesSet<double> projected_data_;

    // Settings
    ModelSettings settings_;
    bool inverse_enabled_;

    // Configuration file parser state (temporary during loading)
    struct ConfigData {
        std::vector<std::string> keywords;
        std::vector<std::string> values;
        std::vector<std::vector<std::string>> param_names;
        std::vector<std::vector<std::string>> param_values;
        std::vector<std::vector<std::string>> est_param_names;
    };
    ConfigData config_data_;
};
