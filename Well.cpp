#include "Well.h"
#include "Distribution.h"
#include "Utilities.h"
#include "Vector.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

// ============================================================================
// Constructors
// ============================================================================

CWell::CWell()
    : name_("")
    , distribution_type_("")
    , fraction_old_(0.0)
    , age_old_(0.0)
    , fraction_modern_(0.0)
    , vz_delay_(0.0)
    , histogram_bin_count_(0)
    , histogram_bin_size_(0.0)
{

}

CWell::CWell(const std::string& well_name)
    : CWell()
{
    name_ = well_name;

}

CWell::CWell(const CWell& other)
    : name_(other.name_)
    , distribution_type_(other.distribution_type_)
    , parameters_(other.parameters_)
    , young_age_distribution_(other.young_age_distribution_)
    , fraction_old_(other.fraction_old_)
    , age_old_(other.age_old_)
    , fraction_modern_(other.fraction_modern_)
    , vz_delay_(other.vz_delay_)
    , histogram_bin_count_(other.histogram_bin_count_)
    , histogram_bin_size_(other.histogram_bin_size_)
{

}

CWell& CWell::operator=(const CWell& other)
{
    if (this != &other) {
        name_ = other.name_;
        distribution_type_ = other.distribution_type_;
        parameters_ = other.parameters_;
        young_age_distribution_ = other.young_age_distribution_;
        fraction_old_ = other.fraction_old_;
        age_old_ = other.age_old_;
        fraction_modern_ = other.fraction_modern_;
        vz_delay_ = other.vz_delay_;
        histogram_bin_count_ = other.histogram_bin_count_;
        histogram_bin_size_ = other.histogram_bin_size_;

    }
    return *this;
}

// ============================================================================
// Property Setters
// ============================================================================

void CWell::setDistributionType(const std::string& type)
{
    // List of valid distribution types
    static const std::vector<std::string> validTypes = {
        "Piston",
        "Exponential",
        "Piston+Exponential",
        "Gamma",
        "Log-Normal",
        "Inverse-Gaussian",
        "Dispersion",
        "Histogram"
    };

    // Check if the type is valid (case-insensitive)
    bool isValid = false;
    std::string lowerType = type;
    std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);

    for (const auto& validType : validTypes) {
        std::string lowerValid = validType;
        std::transform(lowerValid.begin(), lowerValid.end(), lowerValid.begin(), ::tolower);

        if (lowerType == lowerValid) {
            isValid = true;
            break;
        }
    }

    if (!isValid) {
        std::cerr << "WARNING: Invalid distribution type '" << type << "' for well '"
                  << name_ << "'. Valid types are: ";
        for (size_t i = 0; i < validTypes.size(); ++i) {
            std::cerr << validTypes[i];
            if (i < validTypes.size() - 1) std::cerr << ", ";
        }
        std::cerr << std::endl;
    }

    distribution_type_ = type;

    // Resize parameters vector for this distribution type
    int param_count = getParameterCount(type, histogram_bin_count_);
    if (param_count > 0) {
        parameters_.resize(param_count, 0.0);
    }
}

// ============================================================================
// Parameter Setting
// ============================================================================

bool CWell::setParameter(const std::string& param_name, double value)
{
    std::vector<char> delimiters = {'[', ']', ':'};
    std::vector<std::string> parts = aquiutils::split(param_name, delimiters);

    if (parts.size() == 1) {
        // Simple parameter name
        std::string lower_name = aquiutils::tolower(aquiutils::trim(parts[0]));

        if (lower_name == "f") {
            setFractionOld(value);
            return true;
        }
        else if (lower_name == "fm") {
            setFractionModern(value);
            return true;
        }
        else if (lower_name == "vz_delay") {
            setVzDelay(value);
            return true;
        }
        else if (lower_name == "age_old") {
            setAgeOld(value);
            return true;
        }
        return false;
    }
    else if (parts.size() == 2) {
        // Array-style parameter like "param[0]"
        std::string lower_name = aquiutils::tolower(aquiutils::trim(parts[0]));

        if (lower_name == "param") {
            int index = std::atoi(parts[1].c_str());
            if (index >= 0 && index < static_cast<int>(parameters_.size())) {
                parameters_[index] = value;
                return true;
            }
        }
        return false;
    }

    return false;
}

int CWell::getParameterCount(const std::string& distribution_name, int n_bins)
{
    std::string lower_name = aquiutils::tolower(distribution_name);

    if (lower_name == "piston") return 1;
    if (lower_name == "gamma") return 2;
    if (lower_name == "inverse-gaussian") return 2;
    if (lower_name == "log-normal") return 2;
    if (lower_name == "histogram") return n_bins;
    if (lower_name == "exponential" || lower_name == "exponential") return 1;
    if (lower_name == "shifted exponential") return 2;

    return 0;
}

// ============================================================================
// Distribution Creation
// ============================================================================

void CWell::createDistribution(double oldest_time, int num_intervals, double multiplier)
{
    std::string lower_type = aquiutils::tolower(distribution_type_);

    if (lower_type == "piston") {
        young_age_distribution_ = createDiracDistribution(parameters_, oldest_time, num_intervals, multiplier);
    }
    else if (lower_type == "gamma") {
        young_age_distribution_ = createGammaDistribution(parameters_, oldest_time, num_intervals, multiplier);
    }
    else if (lower_type == "inverse-gaussian") {
        young_age_distribution_ = createInverseGaussianDistribution(parameters_, oldest_time, num_intervals, multiplier);
    }
    else if (lower_type == "log-normal") {
        young_age_distribution_ = createLogNormalDistribution(parameters_, oldest_time, num_intervals, multiplier);
    }
    else if (lower_type == "histogram") {
        young_age_distribution_ = createHistogramDistribution(parameters_, histogram_bin_count_,
                                                              histogram_bin_size_, oldest_time,
                                                              num_intervals, multiplier);
    }
    else if (lower_type == "exponential") {
        young_age_distribution_ = createExponentialDistribution(parameters_, oldest_time, num_intervals, multiplier);
    }
    else if (lower_type == "shifted exponential") {
        young_age_distribution_ = createShiftedExponentialDistribution(parameters_, oldest_time, num_intervals, multiplier);
    }

}

// ============================================================================
// Static Distribution Functions
// ============================================================================

TimeSeries<double> CWell::createDiracDistribution(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    TimeSeries<double> age_dist(num_intervals + 1);

    double dt0 = (multiplier != 0) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 0.0);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i));
    }

    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setValue(i, 0.0);
        double mid_age = 0.5 * (age_dist.getTime(i - 1) + age_dist.getTime(i));
        double next_mid = (i < num_intervals) ?
                              0.5 * (age_dist.getTime(i) + age_dist.getTime(i + 1)) : age_dist.getTime(i);

        if (mid_age < params[0] && next_mid > params[0]) {
            age_dist.setValue(i, 2.0 / (next_mid - age_dist.getTime(i - 1)));
        }
    }

    return age_dist;
}

// ============================================================================
// Serialization / Output
// ============================================================================

std::string CWell::parametersToString() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);

    oss << "Well: " << name_ << "\n";
    oss << "  Distribution type: " << distribution_type_ << "\n";
    oss << "  Distribution parameters: [";
    for (size_t i = 0; i < parameters_.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << parameters_[i];
    }
    oss << "]\n";

    oss << "  Fraction old water: " << fraction_old_ << "\n";
    oss << "  Age of old water: " << age_old_ << " years\n";
    oss << "  Fraction modern: " << fraction_modern_ << "\n";
    oss << "  VZ delay: " << vz_delay_ << " years\n";

    if (distribution_type_ == "hist" || distribution_type_ == "histogram") {
        oss << "  Histogram bins: " << histogram_bin_count_ << "\n";
        oss << "  Histogram bin size: " << histogram_bin_size_ << "\n";
    }

    if (young_age_distribution_.size() > 0) {
        oss << "  Age distribution computed: " << young_age_distribution_.size() << " points\n";
    }

    return oss.str();
}

void CWell::writeInfo(std::ostream& out) const
{
    out << parametersToString();
}

TimeSeries<double> CWell::createExponentialDistribution(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    TimeSeries<double> age_dist(num_intervals + 1);

    double dt0 = (multiplier != 0) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 0.0);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i));
    }

    for (int i = 0; i < num_intervals + 1; ++i) {
        //qDebug()<<i;
        //qDebug()<<-age_dist.getTime(i);
        //qDebug()<<-age_dist.getTime(i) / params[0];
        //qDebug()<<exp(-age_dist.getTime(i) / params[0]);
        //qDebug() <<(1.0 / params[0]) * std::exp(-age_dist.getTime(i) / params[0]);
        age_dist.setValue(i, (1.0 / params[0]) * std::exp(-age_dist.getTime(i) / params[0]));
    }

    return age_dist;
}

TimeSeries<double> CWell::createLogNormalDistribution(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    TimeSeries<double> age_dist(num_intervals + 1);

    double dt0 = (multiplier != 1) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 0.0);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i - 1));
    }

    age_dist.setValue(0, 0.0);
    double pi = 4.0 * std::atan(1.0);

    for (int i = 1; i < num_intervals + 1; ++i) {
        double t = age_dist.getTime(i);
        double value = 1.0 / (t * params[1] * std::sqrt(2.0 * pi)) *
                       std::exp(-std::pow(std::log(t) - std::log(params[0]), 2) /
                                (2.0 * std::pow(params[1], 2)));
        age_dist.setValue(i, value);
    }

    return age_dist;
}

TimeSeries<double> CWell::createInverseGaussianDistribution(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    double pi = 4.0 * std::atan(1.0);
    TimeSeries<double> age_dist(num_intervals + 1);

    double lambda = std::pow(params[0], 3) / std::pow(params[1], 2);
    double dt0 = (multiplier != 1) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 1e-12);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i));
    }

    for (int i = 0; i < num_intervals + 1; ++i) {
        double t = age_dist.getTime(i);
        double value = std::sqrt(lambda / (2.0 * pi * std::pow(t, 3))) *
                       std::exp(-lambda * std::pow(t - params[0], 2) /
                                (2.0 * std::pow(params[0], 2) * t));
        age_dist.setValue(i, value);
    }

    return age_dist;
}

TimeSeries<double> CWell::createInverseGaussianDistribution_MuLambda(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    double pi = 4.0 * std::atan(1.0);
    TimeSeries<double> age_dist(num_intervals + 1);

    double lambda = params[1];
    double dt0 = (multiplier != 1) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 1e-12);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i));
    }

    for (int i = 0; i < num_intervals + 1; ++i) {
        double t = age_dist.getTime(i);
        double value = std::sqrt(lambda / (2.0 * pi * std::pow(t, 3))) *
                       std::exp(-lambda * std::pow(t - params[0], 2) /
                                (2.0 * std::pow(params[0], 2) * t));
        age_dist.setValue(i, value);
    }

    return age_dist;
}

TimeSeries<double> CWell::createLevyDistribution(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    double pi = 4.0 * std::atan(1.0);
    TimeSeries<double> age_dist(num_intervals + 1);

    double c_levy = params[0];
    double dt0 = (multiplier != 1) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 1e-12);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i));
    }

    for (int i = 0; i < num_intervals + 1; ++i) {
        double t = age_dist.getTime(i);
        double value = std::sqrt(c_levy / (2.0 * pi)) *
                       std::exp(-c_levy / (2.0 * t)) / std::pow(t, 1.5);
        age_dist.setValue(i, value);
    }

    return age_dist;
}

TimeSeries<double> CWell::createShiftedLevyDistribution(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    double pi = 4.0 * std::atan(1.0);
    TimeSeries<double> age_dist(num_intervals + 1);

    double c_levy = params[0];
    double t_shift = params[1];
    double dt0 = (multiplier != 1) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 1e-12);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i));
    }

    for (int i = 0; i < num_intervals + 1; ++i) {
        double t = age_dist.getTime(i);
        if (t > t_shift) {
            double value = std::sqrt(c_levy / (2.0 * pi)) *
                           std::exp(-c_levy / (2.0 * (t - t_shift))) /
                           std::pow(t - t_shift, 1.5);
            age_dist.setValue(i, value);
        } else {
            age_dist.setValue(i, 0.0);
        }
    }

    return age_dist;
}

TimeSeries<double> CWell::createShiftedExponentialDistribution(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    TimeSeries<double> age_dist(num_intervals + 1);

    double lambda = params[0];
    double t_shift = params[1];
    double dt0 = (multiplier != 1) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 1e-12);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i));
    }

    for (int i = 0; i < num_intervals + 1; ++i) {
        double t = age_dist.getTime(i);
        if (t > t_shift) {
            age_dist.setValue(i, (1.0 / lambda) * std::exp(-(t - t_shift) / lambda));
        } else {
            age_dist.setValue(i, 0.0);
        }
    }

    return age_dist;
}

TimeSeries<double> CWell::createGeneralizedInverseGaussianDistribution(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    TimeSeries<double> age_dist(num_intervals + 1);

    double p = params[0];
    double a = params[1];
    double b = params[2];

    double dt0 = (multiplier != 1) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 1e-12);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i));
    }

    for (int i = 0; i < num_intervals + 1; ++i) {
        double t = age_dist.getTime(i);
        double value = std::pow(t, p - 1) * std::exp(-(a * t + b / t) / 2.0);
        age_dist.setValue(i, value);
    }

    double area = age_dist.integrate();
    return (1.0 / area) * age_dist;
}

void CWell::SetRealizations(const TimeSeriesSet<double>& real) {
    realizations = real;
}

void CWell::SetPercentile95(const TimeSeriesSet<double>& pct) {
    percentile95 = pct;
}

const TimeSeriesSet<double>& CWell::GetRealizations() const {
    return realizations;
}

const TimeSeriesSet<double>& CWell::GetPercentile95() const {
    return percentile95;
}

TimeSeries<double> CWell::createDispersionDistribution(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    double pi = 4.0 * std::atan(1.0);
    TimeSeries<double> age_dist(num_intervals + 1);

    double dt0 = (multiplier != 1) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 1e-12);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i));
    }

    for (int i = 0; i < num_intervals + 1; ++i) {
        double t = age_dist.getTime(i);
        double value = 1.0 / std::sqrt(2.0 * pi * params[1] * t) *
                       std::exp(-std::pow(t - params[0], 2) / (4.0 * params[1] * t));
        age_dist.setValue(i, value);
    }

    return age_dist;
}

TimeSeries<double> CWell::createHistogramDistribution(
    const std::vector<double>& params,
    int num_bins,
    double bin_size,
    double max_age,
    int num_intervals,
    double multiplier)
{
    TimeSeries<double> age_dist(num_intervals + 1);

    double dt0 = (multiplier != 0) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 0.0);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i - 1));
    }

    age_dist.setValue(0, 0.0);

    for (int i = 1; i < num_intervals + 1; ++i) {
        double t = age_dist.getTime(i);
        age_dist.setValue(i, 0.0);

        for (int j = 0; j < num_bins - 1; ++j) {
            if (t > j * bin_size && t <= (j + 1) * bin_size) {
                age_dist.setValue(i, params[j] / bin_size);
            }
        }

        if (t > (num_bins - 1) * bin_size && t <= num_bins * bin_size) {
            CVector param_vec(params);
            age_dist.setValue(i, (1.0 - param_vec.sum()) / bin_size);
        }
    }

    return age_dist;
}

TimeSeries<double> CWell::createGammaDistribution(
    const std::vector<double>& params,
    double max_age,
    int num_intervals,
    double multiplier)
{
    double k_gamma = params[0];
    double theta_gamma = params[1];

    TimeSeries<double> age_dist(num_intervals + 1);

    double dt0 = (multiplier != 1) ?
                     max_age / (1 + std::pow(1 + multiplier, num_intervals)) * multiplier :
                     max_age / num_intervals;

    age_dist.setTime(0, 0.0);
    for (int i = 1; i < num_intervals + 1; ++i) {
        age_dist.setTime(i, age_dist.getTime(i - 1) + dt0 * std::pow(1 + multiplier, i - 1));
    }

    age_dist.setValue(0, 0.0);
    for (int i = 1; i < num_intervals + 1; ++i) {
        double t = age_dist.getTime(i);
        age_dist.setValue(i, Gammapdf(t, k_gamma, theta_gamma));
    }

    return age_dist;
}
