#include "GWA.h"
#include "Utilities.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>

// ============================================================================
// Constructors
// ============================================================================

CGWA::CGWA()
    : inverse_enabled_(false)
{
}

CGWA::CGWA(const std::string& config_filename)
    : CGWA()
{
    loadFromFile(config_filename);
}

CGWA::CGWA(const CGWA& other)
    : tracers_(other.tracers_)
    , wells_(other.wells_)
    , observations_(other.observations_)
    , parameters_(other.parameters_)
    , parameter_values_(other.parameter_values_)
    , obs_std_devs_(other.obs_std_devs_)
    , modeled_data_(other.modeled_data_)
    , projected_data_(other.projected_data_)
    , settings_(other.settings_)
    , inverse_enabled_(other.inverse_enabled_)
{
    linkSourceTracers();
}

CGWA& CGWA::operator=(const CGWA& other)
{
    if (this != &other) {
        tracers_ = other.tracers_;
        wells_ = other.wells_;
        observations_ = other.observations_;
        parameters_ = other.parameters_;
        parameter_values_ = other.parameter_values_;
        obs_std_devs_ = other.obs_std_devs_;
        modeled_data_ = other.modeled_data_;
        projected_data_ = other.projected_data_;
        settings_ = other.settings_;
        inverse_enabled_ = other.inverse_enabled_;

        linkSourceTracers();
    }
    return *this;
}

// ============================================================================
// Configuration Loading
// ============================================================================

bool CGWA::loadFromFile(const std::string& filename)
{
    inverse_enabled_ = false;

    if (!parseConfigFile(filename)) {
        return false;
    }

    loadSettings();
    loadParameters();
    loadTracers();
    loadWells();
    loadObservedData();
    setupStdDevParameters();
    linkSourceTracers();
    updateConstantInputs();

    return true;
}

bool CGWA::parseConfigFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.good()) {
        std::cerr << "Cannot open config file: " << filename << std::endl;
        return false;
    }

    config_data_ = ConfigData(); // Clear previous data

    bool in_bracket_block = false;
    std::vector<std::string> current_params;

    while (!file.eof()) {
        std::vector<std::string> line = aquiutils::getline(file);
        if (line.empty()) continue;

        if (!in_bracket_block) {
            // Parse "keyword = value" line
            std::vector<std::string> parts = aquiutils::split(line[0], '=');
            if (parts.size() >= 2) {
                config_data_.keywords.push_back(aquiutils::trim(parts[0]));
                config_data_.values.push_back(aquiutils::trim(parts[1]));

                // Check for parameter block
                if (line.size() > 1) {
                    size_t brace_open = line[1].find('{');
                    size_t brace_close = line[1].find('}');

                    if (brace_open != std::string::npos) {
                        in_bracket_block = true;
                        current_params.clear();

                        // Parse any params on same line
                        std::string param_text = line[1].substr(brace_open + 1,
                                                                brace_close == std::string::npos ? std::string::npos : brace_close - brace_open - 1);
                        std::vector<std::string> params = aquiutils::split(param_text, ';');
                        for (const auto& p : params) {
                            if (!aquiutils::trim(p).empty()) {
                                current_params.push_back(aquiutils::trim(p));
                            }
                        }

                        if (brace_close != std::string::npos) {
                            in_bracket_block = false;
                            parseParameterBlock(current_params);
                        }
                    } else {
                        // No parameters for this keyword
                        parseParameterBlock(current_params);
                    }
                } else {
                    parseParameterBlock(current_params);
                }
            }
        }
        else {
            // Inside parameter block
            std::vector<std::string> params = aquiutils::split(line[0], ';');
            for (const auto& p : params) {
                std::string trimmed = aquiutils::trim(p);
                if (!trimmed.empty() && trimmed != "{" && trimmed != "}") {
                    current_params.push_back(trimmed);
                }
            }

            if (line[0].find('}') != std::string::npos) {
                in_bracket_block = false;
                parseParameterBlock(current_params);
            }
        }
    }

    return true;
}

void CGWA::parseParameterBlock(const std::vector<std::string>& params)
{
    std::vector<std::string> names, values, est_params;

    for (const auto& param : params) {
        std::vector<std::string> parts = aquiutils::split(param, '=');
        if (parts.size() >= 2) {
            names.push_back(aquiutils::trim(parts[0]));

            std::string value_part = aquiutils::trim(parts[1]);

            // Check if it's a parameter reference like p[0]
            if (value_part.size() >= 4 && value_part.substr(0, 2) == "p[") {
                size_t bracket_close = value_part.find(']');
                if (bracket_close != std::string::npos) {
                    est_params.push_back(value_part.substr(2, bracket_close - 2));
                    values.push_back("0"); // Placeholder
                } else {
                    est_params.push_back("");
                    values.push_back(value_part);
                }
            } else {
                est_params.push_back("");
                values.push_back(value_part);
            }
        }
    }

    config_data_.param_names.push_back(names);
    config_data_.param_values.push_back(values);
    config_data_.est_param_names.push_back(est_params);
}

// ============================================================================
// Settings Loading
// ============================================================================

void CGWA::loadSettings()
{
    settings_ = ModelSettings();

    for (size_t i = 0; i < config_data_.keywords.size(); ++i) {
        std::string key = aquiutils::tolower(config_data_.keywords[i]);
        const std::string& val = config_data_.values[i];

        if (key == "path") {
            settings_.input_path = val;
        }
        else if (key == "outpath") {
            settings_.output_path = val;
        }
        else if (key == "pe_info") {
            settings_.pe_info_filename = val;
        }
        else if (key == "detout") {
            settings_.det_output_filename = val;
        }
        else if (key == "realizedparam") {
            settings_.realized_param_filename = val;
        }
        else if (key == "single_vz_delay") {
            settings_.single_vz_delay = (std::atoi(val.c_str()) != 0);
        }
        else if (key == "fixed_old_tracer") {
            settings_.fixed_old_tracer = (std::atoi(val.c_str()) != 0);
        }
        else if (key == "project_start") {
            settings_.project_enabled = true;
            settings_.project_start = std::atof(val.c_str());
        }
        else if (key == "project_finish") {
            settings_.project_finish = std::atof(val.c_str());
        }
        else if (key == "project_interval") {
            settings_.project_interval = std::atof(val.c_str());
        }
    }

    if (settings_.output_path.empty()) {
        settings_.output_path = settings_.input_path;
    }
}

// ============================================================================
// Parameter Loading
// ============================================================================

void CGWA::loadParameters()
{
    parameters_.clear();
    parameter_values_.clear();

    for (size_t i = 0; i < config_data_.keywords.size(); ++i) {
        if (aquiutils::tolower(config_data_.keywords[i]) == "parameter") {
            inverse_enabled_ = true;

            ParameterRange param;
            param.name = config_data_.values[i];

            // Parse parameter properties
            for (size_t j = 0; j < config_data_.param_names[i].size(); ++j) {
                std::string pname = aquiutils::tolower(config_data_.param_names[i][j]);
                double pval = std::atof(config_data_.param_values[i][j].c_str());

                if (pname == "low") param.low = pval;
                else if (pname == "high") param.high = pval;
                else if (pname == "fixed") param.fixed = (pval != 0.0);
                else if (pname == "log") param.logarithmic = (pval != 0.0);
                else if (pname == "applytoall") param.apply_to_all = (pval != 0.0);
            }

            parameters_.push_back(param);

            // Initial value: midpoint (or geometric mean if log-scale)
            double initial_value;
            if (!param.logarithmic) {
                initial_value = 0.5 * (param.low + param.high);
            } else {
                initial_value = std::sqrt(std::abs(param.low * param.high));
                if (param.low < 0) initial_value = -initial_value;
            }
            parameter_values_.push_back(initial_value);
        }
    }
}

// ============================================================================
// Tracers Loading
// ============================================================================

void CGWA::loadTracers()
{
    tracers_.clear();

    for (size_t i = 0; i < config_data_.keywords.size(); ++i) {
        if (aquiutils::tolower(config_data_.keywords[i]) == "tracer") {
            CTracer tracer(config_data_.values[i]);

            // Parse tracer properties
            for (size_t j = 0; j < config_data_.param_names[i].size(); ++j) {
                std::string pname = aquiutils::tolower(config_data_.param_names[i][j]);
                const std::string& pval_str = config_data_.param_values[i][j];
                double pval = std::atof(pval_str.c_str());

                if (pname == "input") {
                    TimeSeries<double> input(settings_.input_path + pval_str);
                    tracer.setInput(input);
                }
                else if (pname == "source") {
                    tracer.setSourceTracerName(pval_str);
                }
                else if (pname == "linear_prod") {
                    tracer.setLinearProduction(pval != 0.0);
                }
                else {
                    tracer.setParameter(pname, pval);
                }

                // Link to parameter if specified
                const std::string& est_param = config_data_.est_param_names[i][j];
                if (!est_param.empty()) {
                    int param_idx = findParameter(est_param);
                    if (param_idx >= 0) {
                        parameters_[param_idx].locations.push_back(tracer.getName());
                        parameters_[param_idx].quantities.push_back(pname);
                        parameters_[param_idx].location_types.push_back(1); // 1 = tracer
                    }
                }
            }

            tracers_.push_back(tracer);
        }
    }
}

// ============================================================================
// Wells Loading
// ============================================================================

void CGWA::loadWells()
{
    wells_.clear();

    for (size_t i = 0; i < config_data_.keywords.size(); ++i) {
        if (aquiutils::tolower(config_data_.keywords[i]) == "well") {
            CWell well;
            well.name = config_data_.values[i];
            well.vz_delay = 0.0;

            // Find distribution type
            for (size_t j = 0; j < config_data_.param_names[i].size(); ++j) {
                if (aquiutils::tolower(config_data_.param_names[i][j]) == "distribution") {
                    well.distribution = config_data_.param_values[i][j];
                    well.setDistributionType(config_data_.param_values[i][j]);
                    break;
                }
            }

            // Parse well properties
            for (size_t j = 0; j < config_data_.param_names[i].size(); ++j) {
                std::string pname = config_data_.param_names[i][j];
                double pval = std::atof(config_data_.param_values[i][j].c_str());

                well.setParameter(pname, pval);

                // Link to parameter if specified
                const std::string& est_param = config_data_.est_param_names[i][j];
                if (!est_param.empty()) {
                    int param_idx = findParameter(est_param);
                    if (param_idx >= 0) {
                        parameters_[param_idx].locations.push_back(well.name);
                        parameters_[param_idx].quantities.push_back(pname);
                        parameters_[param_idx].location_types.push_back(0); // 0 = well
                    }
                }
            }

            wells_.push_back(well);
        }
    }
}

// ============================================================================
// Observed Data Loading
// ============================================================================

void CGWA::loadObservedData()
{
    observations_.clear();

    for (size_t i = 0; i < config_data_.keywords.size(); ++i) {
        if (aquiutils::tolower(config_data_.keywords[i]) == "observed") {
            ObservedData obs;
            obs.name = config_data_.values[i];

            // Parse observation properties
            for (size_t j = 0; j < config_data_.param_names[i].size(); ++j) {
                std::string pname = aquiutils::tolower(config_data_.param_names[i][j]);
                const std::string& pval_str = config_data_.param_values[i][j];

                if (pname == "well") {
                    obs.well_index = findWell(pval_str);
                }
                else if (pname == "tracer") {
                    obs.tracer_index = findTracer(pval_str);
                }
                else if (pname == "std_no") {
                    obs.std_parameter_index = std::atoi(pval_str.c_str());
                }
                else if (pname == "error_structure") {
                    obs.error_structure = std::atoi(pval_str.c_str());
                }
                else if (pname == "observed_data") {
                    obs.filename = pval_str;
                    obs.data = TimeSeries<double>(settings_.input_path + pval_str);
                }
                else if (pname == "detect_limit_value") {
                    obs.has_detection_limit = true;
                    obs.detection_limit_value = std::atof(pval_str.c_str());
                }
                else if (pname == "count_max") {
                    obs.count_max = (std::atoi(pval_str.c_str()) != 0);
                }
            }

            observations_.push_back(obs);
        }
    }

    // Calculate max data count per well
    std::vector<int> max_counts(wells_.size(), 0);
    for (const auto& obs : observations_) {
        if (obs.well_index >= 0 && obs.well_index < static_cast<int>(max_counts.size())) {
            max_counts[obs.well_index] = std::max(max_counts[obs.well_index],
                                                  static_cast<int>(obs.data.size()));
        }
    }

    for (auto& obs : observations_) {
        if (obs.well_index >= 0 && obs.well_index < static_cast<int>(max_counts.size())) {
            obs.max_data_count = max_counts[obs.well_index];
        }
    }
}

// ============================================================================
// Standard Deviation Parameters Setup
// ============================================================================

void CGWA::setupStdDevParameters()
{
    std::vector<int> std_indices;

    // Find unique std_no values
    for (auto& obs : observations_) {
        if (std::find(std_indices.begin(), std_indices.end(),
                      obs.std_parameter_index) == std_indices.end()) {
            std_indices.push_back(obs.std_parameter_index);

            // Create parameter for this std deviation
            ParameterRange param;
            param.name = "std_" + std::to_string(obs.std_parameter_index);
            param.low = std::exp(-4.0);
            param.high = std::exp(4.0);
            param.fixed = false;
            param.logarithmic = true;
            param.apply_to_all = true;

            obs.std_parameter_index = parameters_.size();
            parameters_.push_back(param);
            parameter_values_.push_back(std::sqrt(param.low * param.high));
        }
    }

    obs_std_devs_.resize(std_indices.size());
}

// ============================================================================
// Source Tracer Linking
// ============================================================================

void CGWA::linkSourceTracers()
{
    for (auto& tracer : tracers_) {
        if (!tracer.getSourceTracerName().empty()) {
            int source_idx = findTracer(tracer.getSourceTracerName());
            if (source_idx >= 0) {
                tracer.setSourceTracer(&tracers_[source_idx]);
            }
        }
    }
}

// ============================================================================
// Lookup Methods
// ============================================================================

int CGWA::findParameter(const std::string& name) const
{
    for (size_t i = 0; i < parameters_.size(); ++i) {
        if (aquiutils::tolower(name) == aquiutils::tolower(parameters_[i].name)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int CGWA::findTracer(const std::string& name) const
{
    for (size_t i = 0; i < tracers_.size(); ++i) {
        if (aquiutils::tolower(name) == aquiutils::tolower(tracers_[i].getName())) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int CGWA::findWell(const std::string& name) const
{
    for (size_t i = 0; i < wells_.size(); ++i) {
        if (aquiutils::tolower(name) == aquiutils::tolower(wells_[i].name)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ============================================================================
// Parameter Management
// ============================================================================

const ParameterRange& CGWA::getParameter(size_t index) const
{
    if (index >= parameters_.size()) {
        throw std::out_of_range("Parameter index out of range");
    }
    return parameters_[index];
}

void CGWA::setParameterValue(size_t index, double value)
{
    if (index >= parameter_values_.size()) {
        throw std::out_of_range("Parameter index out of range");
    }

    parameter_values_[index] = value;
    applyParameterToModel(index, value);
}

void CGWA::setAllParameterValues(const std::vector<double>& values)
{
    if (values.size() != parameter_values_.size()) {
        throw std::invalid_argument("Parameter value count mismatch");
    }

    for (size_t i = 0; i < values.size(); ++i) {
        parameter_values_[i] = values[i];
        applyParameterToModel(i, values[i]);
    }
}

void CGWA::applyParameterToModel(size_t param_index, double value)
{
    if (param_index >= parameters_.size()) {
        return;
    }

    const ParameterRange& param = parameters_[param_index];

    // Apply to wells and tracers
    for (size_t i = 0; i < param.locations.size(); ++i) {
        if (param.location_types[i] == 1) {
            // Apply to tracer
            int tracer_idx = findTracer(param.locations[i]);
            if (tracer_idx >= 0) {
                tracers_[tracer_idx].setParameter(param.quantities[i], value);
            }
        }
        else if (param.location_types[i] == 0) {
            // Apply to well
            int well_idx = findWell(param.locations[i]);
            if (well_idx >= 0) {
                wells_[well_idx].setParameter(param.quantities[i], value);
            }
        }
    }

    // Check if this is a std deviation parameter
    for (auto& obs : observations_) {
        if (obs.std_parameter_index == static_cast<int>(param_index)) {
            if (obs.std_parameter_index < static_cast<int>(obs_std_devs_.size())) {
                obs_std_devs_[obs.std_parameter_index] = value;
            }
        }
    }

    updateConstantInputs();
}

void CGWA::updateConstantInputs()
{
    for (auto& tracer : tracers_) {
        if (tracer.hasConstantInput()) {
            tracer.setConstantInput(tracer.getConstantInputValue());
        }
    }
}

// ============================================================================
// Forward Modeling
// ============================================================================

void CGWA::runForwardModel()
{
    modeled_data_ = TimeSeriesSet<double>(observations_.size());

    double oldest_time = getOldestInputTime();

    // Create age distributions for all wells
    for (auto& well : wells_) {
        well.createDistribution(oldest_time, 1000, 0.02);
    }

    // Calculate concentrations at observation times
    for (size_t i = 0; i < observations_.size(); ++i) {
        const ObservedData& obs = observations_[i];

        if (obs.well_index < 0 || obs.well_index >= static_cast<int>(wells_.size()) ||
            obs.tracer_index < 0 || obs.tracer_index >= static_cast<int>(tracers_.size())) {
            continue;
        }

        const CWell& well = wells_[obs.well_index];
        const CTracer& tracer = tracers_[obs.tracer_index];

        modeled_data_.setname(i, obs.name);

        for (size_t j = 0; j < obs.data.size(); ++j) {
            double time = obs.data.getTime(j);
            double conc = tracer.calculateConcentration(
                time,
                well.young_age_distribution,
                well.fraction_old,
                well.vz_delay,
                settings_.fixed_old_tracer,
                well.age_old,
                well.fm
                );

            modeled_data_[i].append(time, conc);
        }
    }
}

TimeSeriesSet<double> CGWA::runProjection()
{
    if (!settings_.project_enabled) {
        return TimeSeriesSet<double>();
    }

    projected_data_ = TimeSeriesSet<double>(wells_.size() * tracers_.size());

    double oldest_time = getOldestInputTime();

    // Create age distributions for all wells
    for (auto& well : wells_) {
        well.createDistribution(oldest_time, 1000, 0.02);
    }

    // Project concentrations over time
    size_t index = 0;
    for (size_t i = 0; i < wells_.size(); ++i) {
        const CWell& well = wells_[i];

        for (size_t j = 0; j < tracers_.size(); ++j) {
            const CTracer& tracer = tracers_[j];

            projected_data_.setname(index, well.name + "_" + tracer.getName());

            for (double t = settings_.project_start;
                 t < settings_.project_finish;
                 t += settings_.project_interval) {

                double conc = tracer.calculateConcentration(
                    t,
                    well.young_age_distribution,
                    well.fraction_old,
                    well.vz_delay,
                    settings_.fixed_old_tracer,
                    well.age_old,
                    well.fm
                    );

                projected_data_[index].append(t, conc);
            }

            ++index;
        }
    }

    return projected_data_;
}

double CGWA::getOldestInputTime() const
{
    double oldest = -10000.0;

    for (const auto& tracer : tracers_) {
        const TimeSeries<double>& input = tracer.getInput();
        if (input.size() > 1) {
            double duration = input.getTime(input.size() - 1) - input.getTime(0);
            oldest = std::max(oldest, duration);
        }
    }

    return oldest;
}

// ============================================================================
// Log-Likelihood Calculation
// ============================================================================

double CGWA::calculateLogLikelihood()
{
    runForwardModel();

    double log_likelihood = 0.0;

    for (size_t i = 0; i < observations_.size(); ++i) {
        log_likelihood += calculateObservationLikelihood(i);
    }

    // Check for NaN
    if (std::isnan(log_likelihood)) {
        log_likelihood = -30000.0;
    }

    return log_likelihood;
}

double CGWA::calculateObservationLikelihood(size_t obs_index) const
{
    if (obs_index >= observations_.size()) {
        return 0.0;
    }

    const ObservedData& obs = observations_[obs_index];

    if (obs.std_parameter_index < 0 ||
        obs.std_parameter_index >= static_cast<int>(obs_std_devs_.size())) {
        return 0.0;
    }

    double std_dev = obs_std_devs_[obs.std_parameter_index];
    double variance = std_dev * std_dev;

    const TimeSeries<double>& observed = obs.data;
    const TimeSeries<double>& modeled = modeled_data_[obs_index];

    // Data ratio for normalization
    double data_ratio = 1.0;
    if (obs.count_max) {
        data_ratio = 1.0 / static_cast<double>(observed.size());
    }

    double log_p = 0.0;

    if (obs.has_detection_limit) {
        // Handle detection limit
        TimeSeries<double> obs_clamped = max(observed, obs.detection_limit_value);
        TimeSeries<double> mod_clamped = max(modeled, obs.detection_limit_value);

        if (obs.error_structure == 0) {
            // Normal error structure
            log_p = -norm2(obs_clamped > mod_clamped) / (2.0 * variance) -
                    std::log(std_dev) * modeled.size();
        }
        else if (obs.error_structure == 1) {
            // Log-normal error structure
            TimeSeries<double> obs_log = obs_clamped.log(obs.detection_limit_value);
            TimeSeries<double> mod_log = mod_clamped.log(obs.detection_limit_value);
            log_p = -norm2(mod_log > obs_log) / (2.0 * variance) -
                    std::log(std_dev) * modeled.size();
        }
    }
    else {
        // No detection limit
        if (obs.error_structure == 0) {
            // Normal error structure
            log_p = data_ratio * (-norm2(modeled > observed) / (2.0 * variance) -
                                  std::log(std_dev) * modeled.size());
        }
        else if (obs.error_structure == 1) {
            // Log-normal error structure
            TimeSeries<double> obs_log = observed.log(1e-8);
            TimeSeries<double> mod_log = modeled.log(1e-8);
            log_p = data_ratio * (-norm2(mod_log > obs_log) / (2.0 * variance) -
                                  std::log(std_dev) * modeled.size());
        }
    }

    return log_p;
}

// ============================================================================
// Serialization / Output
// ============================================================================

std::string CGWA::parametersToString() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);

    oss << "=== Model Configuration ===\n\n";

    // Wells
    oss << "Wells (" << wells_.size() << "):\n";
    for (size_t i = 0; i < wells_.size(); ++i) {
        oss << "\n" << wells_[i].parametersToString();
    }

    // Tracers
    oss << "\nTracers (" << tracers_.size() << "):\n";
    for (size_t i = 0; i < tracers_.size(); ++i) {
        oss << "\n" << tracers_[i].parametersToString();
    }

    // Parameters (if inverse modeling)
    if (inverse_enabled_) {
        oss << "\nInverse Modeling Parameters (" << parameters_.size() << "):\n";
        for (size_t i = 0; i < parameters_.size(); ++i) {
            const ParameterRange& param = parameters_[i];
            oss << "  " << param.name << ": "
                << "value=" << parameter_values_[i] << ", "
                << "range=[" << param.low << ", " << param.high << "], "
                << "log=" << (param.logarithmic ? "yes" : "no") << ", "
                << "fixed=" << (param.fixed ? "yes" : "no") << "\n";
        }
    }

    // Observations
    oss << "\nObservations (" << observations_.size() << "):\n";
    for (size_t i = 0; i < observations_.size(); ++i) {
        const ObservedData& obs = observations_[i];
        oss << "  " << obs.name << ": "
            << "well=" << obs.well_index << ", "
            << "tracer=" << obs.tracer_index << ", "
            << "data_points=" << obs.data.size() << "\n";
    }

    return oss.str();
}

void CGWA::writeModelSummary(std::ostream& out) const
{
    out << parametersToString();
}

void CGWA::writeParameterValues(std::ostream& out) const
{
    out << std::fixed << std::setprecision(8);

    for (size_t i = 0; i < parameters_.size(); ++i) {
        out << parameters_[i].name << "\t" << parameter_values_[i] << "\n";
    }
}
