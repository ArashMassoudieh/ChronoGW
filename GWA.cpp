#include "GWA.h"
#include "Utilities.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>

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

    for (size_t i = 0; i < config_data_.keywords.size(); ++i) {
        if (aquiutils::tolower(config_data_.keywords[i]) == "parameter") {
            inverse_enabled_ = true;

            Parameter param;
            param.SetName(config_data_.values[i]);

            // Parse parameter properties
            for (size_t j = 0; j < config_data_.param_names[i].size(); ++j) {
                std::string pname = aquiutils::tolower(config_data_.param_names[i][j]);
                const std::string& pval_str = config_data_.param_values[i][j];
                double pval = std::atof(pval_str.c_str());

                if (pname == "low") {
                    param.SetLow(pval);
                }
                else if (pname == "high") {
                    param.SetHigh(pval);
                }
                else if (pname == "log") {
                    // Backward compatibility: support old numeric format
                    if (pval == 1) {
                        param.SetPriorDistribution("log-normal");
                    } else if (pval == 0) {
                        param.SetPriorDistribution("normal");
                    } else {
                        param.SetPriorDistribution("uniform");
                    }
                }
                else if (pname == "prior_distribution") {
                    // New string format
                    std::string dist = aquiutils::tolower(pval_str);
                    if (dist == "log-normal" || dist == "lognormal") {
                        param.SetPriorDistribution("log-normal");
                    } else if (dist == "normal") {
                        param.SetPriorDistribution("normal");
                    } else if (dist == "uniform") {
                        param.SetPriorDistribution("uniform");
                    } else {
                        param.SetPriorDistribution(dist);
                    }
                }
                else if (pname == "value") {
                    param.SetValue(pval);
                }
            }

            parameters_.AddParameter(param);

            // Initial value: midpoint (or geometric mean if log-scale)
            double initial_value;
            if (param.GetPriorDistribution() != "log-normal") {
                initial_value = 0.5 * (param.GetRange().low + param.GetRange().high);
            } else {
                initial_value = std::sqrt(std::abs(param.GetRange().low * param.GetRange().high));
                if (param.GetRange().low < 0) initial_value = -initial_value;
            }
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
                    std::string file_path = pval_str;

                    // Check if it's a full path (contains '/' or '\' or ':' for Windows drive)
                    bool is_full_path = (file_path.find('/') != std::string::npos) ||
                                        (file_path.find('\\') != std::string::npos) ||
                                        (file_path.find(':') != std::string::npos);

                    if (!is_full_path) {
                        // Relative path - prepend input path
                        file_path = settings_.input_path + pval_str;
                    }

                    TimeSeries<double> input(file_path);
                    std::cout<<"Input file path: " << input.getFilename()<<std::endl;
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
                        parameters_[param_idx]->AddLocation(tracer.getName(),pname, "tracer");
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
            well.setName(config_data_.values[i]);
            well.setVzDelay(0.0);

            // Find distribution type first
            for (size_t j = 0; j < config_data_.param_names[i].size(); ++j) {
                if (aquiutils::tolower(config_data_.param_names[i][j]) == "distribution") {
                    well.setDistributionType(config_data_.param_values[i][j]);
                    break;
                }
            }

            // Parse well properties
            for (size_t j = 0; j < config_data_.param_names[i].size(); ++j) {
                std::string pname = config_data_.param_names[i][j];
                std::string pvalue_str = config_data_.param_values[i][j];
                const std::string& est_param = config_data_.est_param_names[i][j];

                double pval;

                // If linked to a parameter, get the value from that parameter
                if (!est_param.empty()) {
                    int param_idx = findParameter(est_param);
                    if (param_idx >= 0) {
                        pval = parameters_[param_idx]->GetValue();
                        // Link this property to the parameter
                        parameters_[param_idx]->AddLocation(well.getName(), pname, "well");
                    } else {
                        // Parameter not found, use the string value as fallback
                        pval = std::atof(pvalue_str.c_str());
                    }
                } else {
                    // Direct value
                    pval = std::atof(pvalue_str.c_str());
                }

                // Set the parameter value in the well
                well.setParameter(pname, pval);
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
            Observation obs;
            obs.SetName(config_data_.values[i]);

            // Parse observation properties
            for (size_t j = 0; j < config_data_.param_names[i].size(); ++j) {
                std::string pname = aquiutils::tolower(config_data_.param_names[i][j]);
                const std::string& pval_str = config_data_.param_values[i][j];

                if (pname == "well") {
                    obs.SetLocation(pval_str);  // Store well name directly
                }
                else if (pname == "tracer") {
                    obs.SetQuantity(pval_str);  // Store tracer name directly
                }
                else if (pname == "std_param" || pname == "std") {
                    // Reference to a parameter name (e.g., "std_3H", "std_tracers")
                    obs.SetStdParameterName(pval_str);
                }
                else if (pname == "error_structure") {
                    // Support both string and numeric formats
                    std::string error_str = aquiutils::tolower(pval_str);
                    obs.SetErrorStructure(error_str);
                }
                else if (pname == "observed_data") {
                    std::string file_path = pval_str;

                    // Check if it's a full path (contains '/' or '\' or ':' for Windows drive)
                    bool is_full_path = (file_path.find('/') != std::string::npos) ||
                                        (file_path.find('\\') != std::string::npos) ||
                                        (file_path.find(':') != std::string::npos);

                    if (!is_full_path) {
                        // Relative path - prepend input path
                        file_path = settings_.input_path + pval_str;
                    }

                    TimeSeries<double> data(file_path);
                    obs.SetObservedTimeSeries(data);
                }
                else if (pname == "detect_limit_value") {
                    obs.SetHasDetectionLimit(true);
                    obs.SetDetectionLimitValue(std::atof(pval_str.c_str()));
                }
                else if (pname == "count_max") {
                    obs.SetCountMax(std::atoi(pval_str.c_str()) != 0);
                }
            }

            observations_.push_back(obs);
        }
    }

    // Calculate max data count per well
    std::map<std::string, int> max_counts;
    for (const auto& obs : observations_) {
        const std::string& location = obs.GetLocation();
        max_counts[location] = std::max(max_counts[location],
                                        static_cast<int>(obs.GetObservedData().size()));
    }

    for (auto& obs : observations_) {
        const std::string& location = obs.GetLocation();
        obs.SetCountMax(max_counts[location]);
    }
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
        if (aquiutils::tolower(name) == aquiutils::tolower(parameters_[i]->GetName())) {
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
        if (aquiutils::tolower(name) == aquiutils::tolower(wells_[i].getName())) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ============================================================================
// Parameter Management
// ============================================================================

Parameter* CGWA::getParameter(size_t index)
{
    return parameters_[static_cast<int>(index)];
}

const Parameter* CGWA::getParameter(size_t index) const
{
    return parameters_[static_cast<int>(index)];
}

void CGWA::SetParameterValue(size_t index, double value)
{
    Parameter* param = parameters_[static_cast<int>(index)];
    if (!param) {
        throw std::out_of_range("Parameter index out of range");
    }

    param->SetValue(value);
    applyParameterToModel(index, value);
}

void CGWA::setAllParameterValues(const std::vector<double>& values)
{
    if (values.size() == 0)
    {
        // Apply current values from parameters
        for (int i = 0; i < parameters_.size(); ++i) {
            Parameter* param = parameters_[i];
            if (param) {
                applyParameterToModel(i, param->GetValue());
            }
        }
    }
    else
    {
        if (values.size() != static_cast<size_t>(parameters_.size())) {
            throw std::invalid_argument("Parameter value count mismatch");
        }

        for (size_t i = 0; i < values.size(); ++i) {
            Parameter* param = parameters_[static_cast<int>(i)];
            if (param) {
                param->SetValue(values[i]);
                applyParameterToModel(i, values[i]);
            }
        }
    }
}

void CGWA::applyParameterToModel(size_t param_index, double value)
{
    Parameter* param = parameters_[static_cast<int>(param_index)];
    if (!param) {
        return;
    }

    // Apply to wells and tracers
    const std::vector<std::string>& locations = param->GetLocations();
    const std::vector<std::string>& quantities = param->GetQuantities();
    const std::vector<std::string>& location_types = param->GetLocationTypes();

    for (size_t i = 0; i < locations.size(); ++i) {
        if (location_types[i] == "tracer" || location_types[i] == "1") {
            // Apply to tracer
            int tracer_idx = findTracer(locations[i]);
            if (tracer_idx >= 0) {
                tracers_[tracer_idx].setParameter(quantities[i], value);
            }
        }
        else if (location_types[i] == "well" || location_types[i] == "0") {
            // Apply to well
            int well_idx = findWell(locations[i]);
            if (well_idx >= 0) {
                wells_[well_idx].setParameter(quantities[i], value);
            }
        }
    }

    // Apply standard deviation to observations that reference this parameter
    for (auto& obs : observations_) {
        if (obs.GetStdParameterName() == param->GetName()) {
            obs.SetErrorStdDev(value);
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

void CGWA::runForwardModel(bool applyparameters)
{
    if (applyparameters)
        this->setAllParameterValues();

    modeled_data_ = TimeSeriesSet<double>(observations_.size());

    double oldest_time = getOldestInputTime();

    // Create age distributions for all wells
    for (auto& well : wells_) {
        well.createDistribution(oldest_time, 1000, 0.02);
    }

    // Calculate concentrations at observation times
    for (size_t i = 0; i < observations_.size(); ++i) {
        Observation& obs = observations_[i];

        int well_idx = findWell(obs.GetLocation());
        int tracer_idx = findTracer(obs.GetQuantity());

        if (well_idx < 0 || well_idx >= static_cast<int>(wells_.size()) ||
            tracer_idx < 0 || tracer_idx >= static_cast<int>(tracers_.size())) {
            continue;
        }

        const CWell& well = wells_[well_idx];
        const CTracer& tracer = tracers_[tracer_idx];

        modeled_data_.setname(i, obs.GetName());
        TimeSeries<double> modeled;

        const TimeSeries<double>& observed = obs.GetObservedData();
        for (size_t j = 0; j < observed.size(); ++j) {
            double time = observed.getTime(j);
            double conc = tracer.calculateConcentration(
                time,
                well.getYoungAgeDistribution(),
                well.getFractionOld(),
                well.getVzDelay(),
                settings_.fixed_old_tracer,
                well.getAgeOld(),
                well.getFractionMineral()
                );

            modeled.append(time, conc);
        }

        obs.SetModeledTimeSeries(modeled);
        modeled_data_[i] = modeled;
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

            projected_data_.setname(index, well.getName() + "_" + tracer.getName());

            for (double t = settings_.project_start;
                 t < settings_.project_finish;
                 t += settings_.project_interval) {

                double conc = tracer.calculateConcentration(
                    t,
                    well.getYoungAgeDistribution(),
                    well.getFractionOld(),
                    well.getVzDelay(),
                    settings_.fixed_old_tracer,
                    well.getAgeOld(),
                    well.getFractionMineral()
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

    const Observation& obs = observations_[obs_index];

    double std_dev = obs.GetErrorStdDev();
    if (std_dev <= 0.0) {
        return 0.0;  // Invalid std dev
    }

    double variance = std_dev * std_dev;

    const TimeSeries<double>& observed = obs.GetObservedData();
    const TimeSeries<double>& modeled = modeled_data_[obs_index];

    // Data ratio for normalization
    double data_ratio = 1.0;
    if (obs.GetCountMax()) {
        data_ratio = 1.0 / static_cast<double>(observed.size());
    }

    double log_p = 0.0;

    if (obs.HasDetectionLimit()) {
        // Handle detection limit
        TimeSeries<double> obs_clamped = max(observed, obs.GetDetectionLimitValue());
        TimeSeries<double> mod_clamped = max(modeled, obs.GetDetectionLimitValue());

        if (obs.GetErrorStructure() == "normal") {
            // Normal error structure
            log_p = -norm2(obs_clamped > mod_clamped) / (2.0 * variance) -
                    std::log(std_dev) * modeled.size();
        }
        else if (obs.GetErrorStructure() == "log-normal") {
            // Log-normal error structure
            TimeSeries<double> obs_log = obs_clamped.log(obs.GetDetectionLimitValue());
            TimeSeries<double> mod_log = mod_clamped.log(obs.GetDetectionLimitValue());
            log_p = -norm2(mod_log > obs_log) / (2.0 * variance) -
                    std::log(std_dev) * modeled.size();
        }
    }
    else {
        // No detection limit
        if (obs.GetErrorStructure() == "normal") {
            // Normal error structure
            log_p = data_ratio * (-norm2(modeled > observed) / (2.0 * variance) -
                                  std::log(std_dev) * modeled.size());
        }
        else if (obs.GetErrorStructure() == "log-normal") {
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
        for (int i = 0; i < parameters_.size(); ++i) {
            const Parameter* param = parameters_[i];
            if (!param) continue;

            oss << "  " << param->GetName() << ":\n"
                << "    Value: " << param->GetValue() << "\n"
                << "    Range: [" << param->GetRange().low << ", " << param->GetRange().high << "]\n"
                << "    Prior distribution: " << param->GetPriorDistribution() << "\n";

            // Show locations/quantities/types if any
            const std::vector<std::string>& locations = param->GetLocations();
            const std::vector<std::string>& quantities = param->GetQuantities();
            const std::vector<std::string>& location_types = param->GetLocationTypes();

            if (!locations.empty()) {
                oss << "    Applies to:\n";
                for (size_t j = 0; j < locations.size(); ++j) {
                    oss << "      - Location: " << locations[j]
                        << ", Quantity: " << quantities[j]
                        << ", Type: " << location_types[j] << "\n";
                }
            }
        }
    }

    // Observations
    oss << "\nObservations (" << observations_.size() << "):\n";
    for (size_t i = 0; i < observations_.size(); ++i) {
        const Observation& obs = observations_[i];
        oss << "  " << obs.GetName() << ": "
            << "location=" << obs.GetLocation() << ", "
            << "quantity=" << obs.GetQuantity() << ", "
            << "data_points=" << obs.GetObservedData().size() << "\n";
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

    for (int i = 0; i < parameters_.size(); ++i) {
        const Parameter* param = parameters_[i];
        if (param) {
            out << param->GetName() << "\t" << param->GetValue() << "\n";
        }
    }
}

const std::vector<double> CGWA::getParameterValues() const
{
    std::vector<double> values;
    values.reserve(parameters_.size());

    for (int i = 0; i < parameters_.size(); ++i) {
        const Parameter* param = parameters_[i];
        if (param) {
            values.push_back(param->GetValue());
        }
    }

    return values;
}

std::vector<double> CGWA::getObservationStdDevs() const
{
    std::vector<double> std_devs;
    std_devs.reserve(observations_.size());

    for (const auto& obs : observations_) {
        int param_idx = findParameter(obs.GetStdParameterName());
        if (param_idx >= 0) {
            std_devs.push_back(parameters_[param_idx]->GetValue());
        } else {
            std_devs.push_back(0.0); // Default if parameter not found
        }
    }

    return std_devs;
}

bool CGWA::exportToFile(const std::string& filename) const
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file for writing: " << filename << std::endl;
        return false;
    }

    file << std::fixed << std::setprecision(10);

    // Write settings
    file << "path=" << settings_.input_path << "\n";
    file << "outpath=" << settings_.output_path << "\n";
    file << "\n";

    // Write wells
    for (const auto& well : wells_) {
        file << "well=" << well.getName() << ", {";

        // Find parameters that apply to this well
        std::map<std::string, std::string> well_params;
        for (int i = 0; i < parameters_.size(); ++i) {
            const Parameter* param = parameters_[i];
            if (!param) continue;

            const std::vector<std::string>& locations = param->GetLocations();
            const std::vector<std::string>& quantities = param->GetQuantities();
            const std::vector<std::string>& location_types = param->GetLocationTypes();

            for (size_t j = 0; j < locations.size(); ++j) {
                if ((location_types[j] == "well" || location_types[j] == "0") &&
                    locations[j] == well.getName()) {
                    well_params[quantities[j]] = "p[" + param->GetName() + "]";
                }
            }
        }

        // Write well parameters
        bool first = true;
        if (well_params.find("f") != well_params.end()) {
            file << "f=" << well_params["f"];
            first = false;
        }

        // Write distribution parameters
        const std::vector<double>& distParams = well.getParameters();
        for (size_t i = 0; i < distParams.size(); ++i) {
            if (!first) file << ";";

            std::string key = "param[" + std::to_string(i) + "]";

            // Check if this parameter is linked to a calibration parameter
            if (well_params.find(key) != well_params.end()) {
                file << key << "=" << well_params[key];
            } else {
                // Use the actual value
                file << key << "=" << distParams[i];
            }
            first = false;
        }

        if (!first) file << ";";
        file << "distribution=" << well.getDistributionType();

        if (well_params.find("age_old") != well_params.end()) {
            file << "; age_old=" << well_params["age_old"];
        }
        if (well_params.find("fm") != well_params.end()) {
            file << ";fm=" << well_params["fm"];
        }

        file << "}\n";
    }
    file << "\n";

    // Write tracers
    for (const auto& tracer : tracers_) {
        file << "tracer=" << tracer.getName() << ", {";

        bool first = true;

        // Find parameters that apply to this tracer
        std::map<std::string, std::string> tracer_params;
        for (int i = 0; i < parameters_.size(); ++i) {
            const Parameter* param = parameters_[i];
            if (!param) continue;

            const std::vector<std::string>& locations = param->GetLocations();
            const std::vector<std::string>& quantities = param->GetQuantities();
            const std::vector<std::string>& location_types = param->GetLocationTypes();

            for (size_t j = 0; j < locations.size(); ++j) {
                if ((location_types[j] == "tracer" || location_types[j] == "1") &&
                    locations[j] == tracer.getName()) {
                    tracer_params[quantities[j]] = "p[" + param->GetName() + "]";
                }
            }
        }

        // Write source tracer first if it exists
        if (!tracer.getSourceTracerName().empty()) {
            file << "source=" << tracer.getSourceTracerName();
            first = false;
        }

        // Write decay rate
        if (tracer.getDecayRate() != 0.0 || tracer_params.find("decay") != tracer_params.end()) {
            if (!first) file << "; ";
            if (tracer_params.find("decay") != tracer_params.end()) {
                file << "decay=" << tracer_params["decay"];
            } else {
                file << "decay=" << tracer.getDecayRate();
            }
            first = false;
        }

        // Write fm
        if (tracer.getMaxFractionModern() != 0.0) {
            if (!first) file << ";";
            file << "fm=" << tracer.getMaxFractionModern();
            first = false;
        }

        // Write input file (only if not from source tracer and has file)
        if (tracer.getSourceTracerName().empty() && tracer.hasInputFile()) {
            if (!first) file << ";";

            std::string input_file = tracer.getInputFilename();

            // Check if the file is in the input_path directory
            if (!settings_.input_path.empty() && input_file.find(settings_.input_path) == 0) {
                // File is in input_path, strip the path to make it relative
                input_file = input_file.substr(settings_.input_path.length());
            }
            // Otherwise keep the full path (file is somewhere else)

            file << "input=" << input_file;
            first = false;
        }

        // Write constant input
        if (tracer.hasConstantInput()) {
            if (!first) file << ";";
            if (tracer_params.find("constant_input") != tracer_params.end()) {
                file << "constant_input=" << tracer_params["constant_input"];
            } else {
                file << "constant_input=" << tracer.getConstantInputValue();
            }
            first = false;
        }

        // Write co parameter
        if (tracer_params.find("co") != tracer_params.end()) {
            if (!first) file << ";";
            file << "co=" << tracer_params["co"];
            first = false;
        }

        // Write cm parameter
        if (tracer_params.find("cm") != tracer_params.end()) {
            if (!first) file << ";";
            file << "cm=" << tracer_params["cm"];
            first = false;
        }

        // Write linear production flag
        if (tracer.hasLinearProduction()) {
            if (!first) file << ";";
            file << "linear_prod=1";
            first = false;
        }

        file << "}\n";
    }
    file << "\n";

    // Write observations
    for (const auto& obs : observations_) {
        file << "observed=" << obs.GetName() << ", {";
        file << "well=" << obs.GetLocation();
        file << "; tracer=" << obs.GetQuantity();
        file << "; std_param=" << obs.GetStdParameterName();
        file << "; error_structure=" << obs.GetErrorStructure();


        // Get filename from observed data TimeSeries
        const TimeSeries<double>& observed_data = obs.GetObservedData();
        if (observed_data.size() > 0 && !observed_data.getFilename().empty()) {
            std::string data_file = observed_data.getFilename();

            // Check if the file is in the input_path directory
            if (!settings_.input_path.empty() && data_file.find(settings_.input_path) == 0) {
                // File is in input_path, strip the path to make it relative
                data_file = data_file.substr(settings_.input_path.length());
            }
            // Otherwise keep the full path (file is somewhere else)

            file << "; observed_data=" << data_file;
        }

        if (obs.HasDetectionLimit()) {
            file << "; detect_limit_value=" << obs.GetDetectionLimitValue();
        }

        if (obs.GetCountMax()) {
            file << "; count_max=1";
        }

        file << "}\n";
    }
    file << "\n";

    // Write project settings
    if (settings_.project_enabled) {
        file << "project_start=" << settings_.project_start << "\n";
        file << "project_end=" << settings_.project_finish << "\n";
        file << "\n";
    }

    // Write output file settings
    if (!settings_.pe_info_filename.empty()) {
        file << "pe_info=" << settings_.pe_info_filename << "\n";
    }
    if (!settings_.det_output_filename.empty()) {
        file << "detout=" << settings_.det_output_filename << "\n";
    }
    if (!settings_.realized_param_filename.empty()) {
        file << "realizedparam=" << settings_.realized_param_filename << "\n";
    }
    file << "\n";

    // Write parameters
    for (int i = 0; i < parameters_.size(); ++i) {
        const Parameter* param = parameters_[i];
        if (!param) continue;

        file << "parameter=" << param->GetName() << ", {";
        file << "low=" << param->GetRange().low;
        file << ";high=" << param->GetRange().high;

        // Write prior distribution as string
        file << "; prior_distribution=" << param->GetPriorDistribution();

        file << "; value=" << param->GetValue();
        file << "}\n";
    }

    file.close();
    return true;
}

bool CGWA::removeParameterLinkage(const std::string& locationName,
                                  const std::string& quantity,
                                  const std::string& locationType)
{
    bool removed = false;

    // Normalize location type ("0" -> "well", "1" -> "tracer")
    std::string normalizedType = locationType;
    if (locationType == "0") normalizedType = "well";
    if (locationType == "1") normalizedType = "tracer";

    // Search through all parameters
    for (size_t i = 0; i < parameters_.size(); ++i) {
        Parameter* param = parameters_[i];
        if (!param) continue;

        // Try to remove with both the given type and normalized type
        if (param->RemoveLocation(locationName, quantity, locationType)) {
            removed = true;
        }
        if (locationType != normalizedType) {
            if (param->RemoveLocation(locationName, quantity, normalizedType)) {
                removed = true;
            }
        }
    }

    return removed;
}

int CGWA::clearParameterLinkages(const std::string& locationName,
                                 const std::string& locationType)
{
    int totalRemoved = 0;

    // Normalize location type
    std::string normalizedType = locationType;
    if (locationType == "0") normalizedType = "well";
    if (locationType == "1") normalizedType = "tracer";

    // Remove from all parameters
    for (size_t i = 0; i < parameters_.size(); ++i) {
        Parameter* param = parameters_[i];
        if (!param) continue;

        totalRemoved += param->RemoveAllLocations(locationName, locationType);
        if (locationType != normalizedType) {
            totalRemoved += param->RemoveAllLocations(locationName, normalizedType);
        }
    }

    return totalRemoved;
}

void CGWA::updateWellParameterLinkages(const std::string& wellName,
                                       const std::map<std::string, std::string>& linkages)
{
    // Define all possible well quantities that could have parameter linkages
    std::vector<std::string> possibleQuantities = {
        "f", "age_old", "fm", "vz_delay"
    };

    // Add distribution parameters (param[0] through param[9])
    for (int i = 0; i < 10; ++i) {
        possibleQuantities.push_back("param[" + std::to_string(i) + "]");
    }

    // Remove existing linkages for all possible quantities
    for (const auto& quantity : possibleQuantities) {
        removeParameterLinkage(wellName, quantity, "well");
    }

    // Add new linkages
    for (const auto& pair : linkages) {
        const std::string& quantity = pair.first;
        const std::string& paramName = pair.second;

        // Find the parameter
        int paramIdx = findParameter(paramName);
        if (paramIdx >= 0) {
            Parameter* param = parameters_[paramIdx];
            if (param) {
                param->AddLocation(wellName, quantity, "well");
            }
        }
    }
}

void CGWA::updateTracerParameterLinkages(const std::string& tracerName,
                                         const std::map<std::string, std::string>& linkages)
{
    // Define all possible tracer quantities
    std::vector<std::string> possibleQuantities = {
        "decay_rate", "retardation", "c_old", "c_modern",
        "fm_max", "input_multiplier"
    };

    // Remove existing linkages
    for (const auto& quantity : possibleQuantities) {
        removeParameterLinkage(tracerName, quantity, "tracer");
    }

    // Add new linkages
    for (const auto& pair : linkages) {
        const std::string& quantity = pair.first;
        const std::string& paramName = pair.second;

        int paramIdx = findParameter(paramName);
        if (paramIdx >= 0) {
            Parameter* param = parameters_[paramIdx];
            if (param) {
                param->AddLocation(tracerName, quantity, "tracer");
            }
        }
    }
}

std::string CGWA::findLinkedParameter(const std::string& locationName,
                                      const std::string& quantity,
                                      const std::string& locationType) const
{
    // Normalize location type
    std::string normalizedType = locationType;
    if (locationType == "0") normalizedType = "well";
    if (locationType == "1") normalizedType = "tracer";

    // Search through all parameters
    for (size_t i = 0; i < parameters_.size(); ++i) {
        const Parameter* param = parameters_[i];
        if (!param) continue;

        const std::vector<std::string>& locations = param->GetLocations();
        const std::vector<std::string>& quantities = param->GetQuantities();
        const std::vector<std::string>& locationTypes = param->GetLocationTypes();

        for (size_t j = 0; j < locations.size(); ++j) {
            // Check both original and normalized type
            bool typeMatch = (locationTypes[j] == locationType ||
                              locationTypes[j] == normalizedType);

            if (typeMatch &&
                locations[j] == locationName &&
                quantities[j] == quantity) {
                return param->GetName();
            }
        }
    }

    return ""; // Not found
}

bool CGWA::removeWell(size_t index)
{
    if (index >= wells_.size()) {
        return false;
    }

    std::string wellName = wells_[index].getName();

    // Remove all parameter linkages for this well
    clearParameterLinkages(wellName, "well");

    // Remove observations associated with this well
    observations_.erase(
        std::remove_if(observations_.begin(), observations_.end(),
                       [&wellName](const Observation& obs) {
                           return obs.GetLocation() == wellName;
                       }),
        observations_.end()
        );

    // Remove the well
    wells_.erase(wells_.begin() + index);

    return true;
}

bool CGWA::removeTracer(size_t index)
{
    if (index >= tracers_.size()) {
        return false;
    }

    std::string tracerName = tracers_[index].getName();

    // Remove all parameter linkages for this tracer
    clearParameterLinkages(tracerName, "tracer");

    // Clear source tracer references in other tracers
    for (auto& tracer : tracers_) {
        if (tracer.getSourceTracerName() == tracerName) {
            tracer.setSourceTracerName("");
            tracer.setSourceTracer(nullptr);
        }
    }

    // Remove observations associated with this tracer
    observations_.erase(
        std::remove_if(observations_.begin(), observations_.end(),
                       [&tracerName](const Observation& obs) {
                           return obs.GetQuantity() == tracerName;
                       }),
        observations_.end()
        );

    // Remove the tracer
    tracers_.erase(tracers_.begin() + index);

    return true;
}

bool CGWA::removeParameter(size_t index)
{
    if (index >= static_cast<size_t>(parameters_.size())) {
        return false;
    }

    // Get parameter name before removing
    std::string paramName = parameters_[index]->GetName();

    // Remove observations that reference this parameter as std_param
    observations_.erase(
        std::remove_if(observations_.begin(), observations_.end(),
                       [&paramName](const Observation& obs) {
                           return obs.GetStdParameterName() == paramName;
                       }),
        observations_.end()
        );

    // Remove the parameter
    parameters_.RemoveParameter(index);

    return true;
}

bool CGWA::removeObservation(size_t index)
{
    if (index >= observations_.size()) {
        return false;
    }

    observations_.erase(observations_.begin() + index);

    // Resize modeled data if it exists
    if (modeled_data_.size() > index) {
        // Note: TimeSeriesSet may need a method to remove an element
        // For now, we'll just note this needs to be refreshed
        modeled_data_ = TimeSeriesSet<double>();
    }

    return true;
}

TimeSeries<double> CGWA::calculateSingleObservation(size_t obs_index)
{
    if (obs_index >= observations_.size()) {
        return TimeSeries<double>();
    }

    // Apply current parameter values to model
    setAllParameterValues();

    const Observation& obs = observations_[obs_index];

    int well_idx = findWell(obs.GetLocation());
    int tracer_idx = findTracer(obs.GetQuantity());

    if (well_idx < 0 || well_idx >= static_cast<int>(wells_.size()) ||
        tracer_idx < 0 || tracer_idx >= static_cast<int>(tracers_.size())) {
        return TimeSeries<double>();
    }

    CWell& well = wells_[well_idx];
    const CTracer& tracer = tracers_[tracer_idx];

    // Get oldest input time for creating age distribution
    double oldest_time = getOldestInputTime();

    // Create age distribution for this well
    well.createDistribution(oldest_time, 1000, 0.02);

    // Calculate concentrations at observation times
    TimeSeries<double> modeled;
    const TimeSeries<double>& observed = obs.GetObservedData();

    for (size_t j = 0; j < observed.size(); ++j) {
        double time = observed.getTime(j);
        double conc = tracer.calculateConcentration(
            time,
            &well,
            settings_.fixed_old_tracer
            );

        modeled.append(time, conc);
    }

    return modeled;
}

TimeSeries<double> CGWA::calculateSingleObservation(const std::string& obs_name)
{
    // Find observation by name
    for (size_t i = 0; i < observations_.size(); ++i) {
        if (observations_[i].GetName() == obs_name) {
            return calculateSingleObservation(i);
        }
    }

    // Observation not found
    return TimeSeries<double>();
}
