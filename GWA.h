#pragma once

#include "Tracer.h"
#include "Well.h"
#include "TimeSeries.h"
#include "TimeSeriesSet.h"
#include <vector>
#include <string>
#include <map>
#include <memory>

/**
 * @brief Parameter range specification for inverse modeling
 */
struct ParameterRange
{
    std::string name;                    ///< Parameter name
    double low;                          ///< Lower bound
    double high;                         ///< Upper bound
    bool fixed;                          ///< Whether parameter is fixed
    bool logarithmic;                    ///< Use log-scale for this parameter
    bool apply_to_all;                   ///< Apply same value to all locations
    double temperature_correction = 1.0; ///< Temperature correction factor

    std::vector<std::string> locations;  ///< Wells or tracers this parameter applies to
    std::vector<std::string> quantities; ///< Parameter names at each location
    std::vector<int> location_types;     ///< 0=well, 1=tracer

    ParameterRange()
        : low(0.0), high(0.0), fixed(false)
        , logarithmic(false), apply_to_all(true) {}
};

/**
 * @brief Observed data characteristics for inverse modeling
 */
struct ObservedData
{
    std::string name;                    ///< Observable name
    int well_index;                      ///< Index into wells vector
    int tracer_index;                    ///< Index into tracers vector
    int std_parameter_index;             ///< Index for std deviation parameter
    int error_structure;                 ///< 0=normal, 1=lognormal

    std::string filename;                ///< Data filename
    TimeSeries<double> data;             ///< Observed time series

    bool has_detection_limit;            ///< Whether detection limit applies
    double detection_limit_value;        ///< Detection limit value

    int max_data_count;                  ///< Max data points among all observables at same well
    bool count_max;                      ///< Whether to normalize by max count

    ObservedData()
        : well_index(-1), tracer_index(-1), std_parameter_index(-1)
        , error_structure(0), has_detection_limit(false)
        , detection_limit_value(0.0), max_data_count(1), count_max(false) {}
};

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
    const ParameterRange& getParameter(size_t index) const;

    /**
     * @brief Get all parameter values
     */
    const std::vector<double>& getParameterValues() const { return parameter_values_; }

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
    void setAllParameterValues(const std::vector<double>& values);

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

    const CTracer& getTracer(size_t index) const { return tracers_[index]; }
    const CWell& getWell(size_t index) const { return wells_[index]; }
    const ObservedData& getObservation(size_t index) const { return observations_[index]; }

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
    void runForwardModel();

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
     */
    const std::vector<double>& getObservationStdDevs() const { return obs_std_devs_; }

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
     * @brief Setup automatic standard deviation parameters
     */
    void setupStdDevParameters();

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
    std::vector<ObservedData> observations_;

    // Parameters for inverse modeling
    std::vector<ParameterRange> parameters_;
    std::vector<double> parameter_values_;
    std::vector<double> obs_std_devs_;

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
