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
    void SetParameterValue(size_t index, double value);

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
    CTracer& getTracer(size_t index) { return tracers_[index]; }
    const CWell& getWell(size_t index) const { return wells_[index]; }
    CWell& getWell(size_t index) { return wells_[index]; }
    const Observation& getObservation(size_t index) const { return observations_[index]; }
    Observation& getObservation(size_t index) { return observations_[index]; }
    Observation* observation(size_t index) {return &observations_[index];}
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
    double GetObjectiveFunctionValue() {return calculateLogLikelihood(); }

    bool GetSolutionFailed() {return false; }
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

    double GetSimulationDuration() const {return 0; }

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


    /**
     * @brief Remove a specific parameter linkage for a well or tracer property
     * @param locationName Name of the well or tracer
     * @param quantity Property name (e.g., "f", "age_old", "param[0]")
     * @param locationType "well" or "tracer"
     * @return true if any linkage was removed
     */
    bool removeParameterLinkage(const std::string& locationName,
                                const std::string& quantity,
                                const std::string& locationType);

    /**
     * @brief Remove all parameter linkages for a specific well or tracer
     * @param locationName Name of the well or tracer
     * @param locationType "well" or "tracer"
     * @return Number of linkages removed
     */
    int clearParameterLinkages(const std::string& locationName,
                               const std::string& locationType);

    /**
     * @brief Update parameter linkages for a well
     * @param wellName Name of the well
     * @param linkages Map of quantity -> parameter name
     *
     * This method clears existing linkages for the specified quantities
     * and creates new ones based on the provided map.
     */
    void updateWellParameterLinkages(const std::string& wellName,
                                     const std::map<std::string, std::string>& linkages);

    /**
     * @brief Update parameter linkages for a tracer
     * @param tracerName Name of the tracer
     * @param linkages Map of quantity -> parameter name
     */
    void updateTracerParameterLinkages(const std::string& tracerName,
                                       const std::map<std::string, std::string>& linkages);

    /**
     * @brief Find which parameter is linked to a specific location and quantity
     * @param locationName Name of well or tracer
     * @param quantity Property name
     * @param locationType "well" or "tracer"
     * @return Parameter name if found, empty string otherwise
     */
    std::string findLinkedParameter(const std::string& locationName,
                                    const std::string& quantity,
                                    const std::string& locationType) const;


    /**
     * @brief Remove a well by index
     * @param index Well index
     * @return true if successful
     */
    bool removeWell(size_t index);

    /**
     * @brief Remove a tracer by index
     * @param index Tracer index
     * @return true if successful
     */
    bool removeTracer(size_t index);

    /**
     * @brief Remove a parameter by index
     * @param index Parameter index
     * @return true if successful
     */
    bool removeParameter(size_t index);

    /**
     * @brief Remove an observation by index
     * @param index Observation index
     * @return true if successful
     */
    bool removeObservation(size_t index);

    /**
 * @brief Add a new well
 * @param well Well to add
 */
    void addWell(const CWell& well) { wells_.push_back(well); }

    /**
 * @brief Add a new tracer
 * @param tracer Tracer to add
 */
    void addTracer(const CTracer& tracer) {
        tracers_.push_back(tracer);
        linkSourceTracers();  // Re-link in case this is a source tracer
    }

    /**
 * @brief Add a new parameter
 * @param param Parameter to add
 */
    void addParameter(const Parameter& param) {
        parameters_.AddParameter(param);
        inverse_enabled_ = true;
    }

    /**
 * @brief Add a new observation
 * @param obs Observation to add
 */
    void addObservation(const Observation& obs) { observations_.push_back(obs); }

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
