#include "Tracer.h"
#include "Utilities.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include "Well.h"

// ============================================================================
// Constructors
// ============================================================================

CTracer::CTracer()
    : name_("")
    , input_multiplier_(1.0)
    , decay_rate_(0.0)
    , retardation_(1.0)
    , c_old_(0.0)
    , c_modern_(0.0)
    , fm_max_(0.0)
    , vz_delay_(false)
    , linear_production_(false)
    , constant_input_(false)
    , constant_input_value_(0.0)
    , source_tracer_name_("")
    , source_tracer_(nullptr)
{
}

CTracer::CTracer(const std::string& name)
    : CTracer()
{
    name_ = name;
}

CTracer::CTracer(const CTracer& other)
    : name_(other.name_)
    , input_(other.input_)
    , input_multiplier_(other.input_multiplier_)
    , decay_rate_(other.decay_rate_)
    , retardation_(other.retardation_)
    , c_old_(other.c_old_)
    , c_modern_(other.c_modern_)
    , fm_max_(other.fm_max_)
    , vz_delay_(other.vz_delay_)
    , linear_production_(other.linear_production_)
    , constant_input_(other.constant_input_)
    , constant_input_value_(other.constant_input_value_)
    , source_tracer_name_(other.source_tracer_name_)
    , source_tracer_(other.source_tracer_)
{
}

CTracer& CTracer::operator=(const CTracer& other)
{
    if (this != &other) {
        name_ = other.name_;
        input_ = other.input_;
        input_multiplier_ = other.input_multiplier_;
        decay_rate_ = other.decay_rate_;
        retardation_ = other.retardation_;
        c_old_ = other.c_old_;
        c_modern_ = other.c_modern_;
        fm_max_ = other.fm_max_;
        vz_delay_ = other.vz_delay_;
        linear_production_ = other.linear_production_;
        constant_input_ = other.constant_input_;
        constant_input_value_ = other.constant_input_value_;
        source_tracer_name_ = other.source_tracer_name_;
        source_tracer_ = other.source_tracer_;
    }
    return *this;
}

// ============================================================================
// Parameter Setting (backward compatibility)
// ============================================================================

bool CTracer::setParameter(const std::string& param_name, double value)
{
    std::string lower_name = aquiutils::tolower(param_name);

    if (lower_name == "co") {
        c_old_ = value;
    }
    else if (lower_name == "cm") {
        c_modern_ = value;
    }
    else if (lower_name == "decay") {
        decay_rate_ = value;
    }
    else if (lower_name == "fm") {
        fm_max_ = value;
    }
    else if (lower_name == "retard") {
        retardation_ = value;
    }
    else if (lower_name == "input_multiplier") {
        input_multiplier_ = value;
    }
    else if (lower_name == "vz_delay") {
        vz_delay_ = (value != 0.0);
    }
    else if (lower_name == "constant_input") {
        setConstantInput(value);
    }
    else {
        return false; // Unknown parameter
    }

    return true;
}

// ============================================================================
// Concentration Calculation - Main Method
// ============================================================================

double CTracer::calculateConcentration(
    double time,
    const TimeSeries<double>& age_distribution,
    double fraction_old,
    double vz_delay,
    bool fixed_old_conc,
    double age_old,
    double fraction_modern) const
{
    // Calculate young water component
    double young_component = 0.0;

    if (!hasSourceTracer()) {
        // Direct input from atmosphere/surface
        young_component = calculateYoungWaterComponent(
            time, age_distribution, vz_delay, fraction_modern);
    }
    else {
        // Production from parent tracer decay
        young_component = calculateFromParentDecay(
            time, age_distribution, vz_delay, fraction_modern);
    }

    // Calculate old water component
    double old_component = calculateOldWaterComponent(
        time, fraction_old, vz_delay, age_old, fraction_modern, fixed_old_conc);

    // Mix young and old water
    return young_component * (1.0 - fraction_old) + old_component * fraction_old;
}

double CTracer::calculateConcentration(double time, CWell *well, bool fixed_old_conc) const
{
    // Calculate young water component
    double young_component = 0.0;

    if (!hasSourceTracer()) {
        // Direct input from atmosphere/surface
        young_component = calculateYoungWaterComponent(
            time, well->getYoungAgeDistribution(), well->getVzDelay(), well->getFractionOld());
    }
    else {
        // Production from parent tracer decay
        young_component = calculateFromParentDecay(
            time, well->getYoungAgeDistribution(), well->getVzDelay(), well->getFractionOld());
    }

    // Calculate old water component
    double old_component = calculateOldWaterComponent(
        time, well->getFractionOld(), well->getVzDelay(), well->getAgeOld(), well->getFractionMineral(), fixed_old_conc);


    // Mix young and old water
    return young_component * (1.0 - well->getFractionOld()) + old_component * well->getFractionOld();
}

// ============================================================================
// Concentration Calculation - Helper Methods
// ============================================================================

double CTracer::calculateYoungWaterComponent(
    double time,
    const TimeSeries<double>& age_distribution,
    double vz_delay,
    double fraction_modern) const
{
    double sum = 0.0;
    double vz = vz_delay_ ? vz_delay : 0.0;

    // Integrate over age distribution
    for (size_t i = 1; i < age_distribution.size(); ++i) {
        double age1 = age_distribution.getTime(i - 1);
        double age2 = age_distribution.getTime(i);
        double pdf1 = age_distribution.getValue(i - 1);
        double pdf2 = age_distribution.getValue(i);

        double value1, value2;

        if (!linear_production_) {
            // Standard decay model
            double t1 = time - retardation_ * (age1 + vz);
            double t2 = time - retardation_ * (age2 + vz);

            value1 = input_multiplier_ * pdf1 * input_.interpol(t1) *
                     std::exp(-decay_rate_ * retardation_ * (age1 + vz));
            value2 = input_multiplier_ * pdf2 * input_.interpol(t2) *
                     std::exp(-decay_rate_ * retardation_ * (age2 + vz));
        }
        else {
            // Linear production model
            double t1 = time - retardation_ * (age1 + vz);
            double t2 = time - retardation_ * (age2 + vz);

            value1 = input_multiplier_ * pdf1 *
                     (input_.interpol(t1) + decay_rate_ * retardation_ * (age1 + vz));
            value2 = input_multiplier_ * pdf2 *
                     (input_.interpol(t2) + decay_rate_ * retardation_ * (age2 + vz));
        }

        // Trapezoidal integration
        sum += (1.0 - fraction_modern * fm_max_) * 0.5 * (value1 + value2) * (age2 - age1);
    }

    return sum;
}

double CTracer::calculateFromParentDecay(
    double time,
    const TimeSeries<double>& age_distribution,
    double vz_delay,
    double fraction_modern) const
{
    if (!source_tracer_) {
        return 0.0; // No parent tracer
    }

    double sum = 0.0;
    double vz = source_tracer_->vz_delay_ ? vz_delay : 0.0;

    // Integrate production from parent decay
    for (size_t i = 1; i < age_distribution.size(); ++i) {
        double age1 = age_distribution.getTime(i - 1);
        double age2 = age_distribution.getTime(i);
        double pdf1 = age_distribution.getValue(i - 1);
        double pdf2 = age_distribution.getValue(i);

        double t1 = time - source_tracer_->retardation_ * (age1 + vz);
        double t2 = time - source_tracer_->retardation_ * (age2 + vz);

        // Parent concentration times (1 - exp(-decay*age)) gives daughter production
        double value1 = source_tracer_->input_multiplier_ * pdf1 *
                        source_tracer_->input_.interpol(t1) *
                        (1.0 - std::exp(-source_tracer_->decay_rate_ *
                                        source_tracer_->retardation_ * age1)) *
                        std::exp(-source_tracer_->decay_rate_ *
                                 source_tracer_->retardation_ * vz);

        double value2 = source_tracer_->input_multiplier_ * pdf2 *
                        source_tracer_->input_.interpol(t2) *
                        (1.0 - std::exp(-source_tracer_->decay_rate_ *
                                        source_tracer_->retardation_ * age2)) *
                        std::exp(-source_tracer_->decay_rate_ *
                                 source_tracer_->retardation_ * vz);

        sum += (1.0 - fraction_modern * fm_max_) * 0.5 * (value1 + value2) * (age2 - age1);
    }

    return sum;
}

double CTracer::calculateOldWaterComponent(
    double time,
    double fraction_old,
    double vz_delay,
    double age_old,
    double fraction_modern,
    bool fixed_old_conc) const
{
    if (fraction_old == 0.0) {
        return 0.0; // No old water
    }

    double vz = vz_delay_ ? vz_delay : 0.0;

    if (fixed_old_conc) {
        // Use fixed concentration for old water
        return (1.0 - fraction_modern * fm_max_) * c_old_ +
               fraction_modern * fm_max_ * c_modern_;
    }
    else {
        // Calculate old water concentration from input
        double old_conc;

        if (!linear_production_) {
            old_conc = input_multiplier_ *
                       input_.interpol(time - retardation_ * (age_old + vz)) *
                       std::exp(-decay_rate_ * retardation_ * (age_old + vz));
        }
        else {
            old_conc = input_multiplier_ *
                       (input_.interpol(time - retardation_ * (age_old + vz)) +
                        decay_rate_ * retardation_ * (age_old + vz));
        }

        return (1.0 - fraction_modern * fm_max_) * old_conc +
               fraction_modern * fm_max_ * c_modern_;
    }
}

// ============================================================================
// Serialization / Output
// ============================================================================

std::string CTracer::parametersToString() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);

    oss << "Tracer: " << name_ << "\n";
    oss << "  Decay rate: " << decay_rate_ << "\n";
    oss << "  Retardation: " << retardation_ << "\n";
    oss << "  Input multiplier: " << input_multiplier_ << "\n";
    oss << "  Old water concentration: " << c_old_ << "\n";
    oss << "  Modern water concentration: " << c_modern_ << "\n";
    oss << "  Max fraction modern: " << fm_max_ << "\n";
    oss << "  VZ delay enabled: " << (vz_delay_ ? "Yes" : "No") << "\n";
    oss << "  Linear production: " << (linear_production_ ? "Yes" : "No") << "\n";

    if (constant_input_) {
        oss << "  Constant input: " << constant_input_value_ << "\n";
    } else {
        oss << "  Input time series: " << input_.size() << " points\n";
    }

    if (!source_tracer_name_.empty()) {
        oss << "  Source tracer: " << source_tracer_name_ << "\n";
    }

    return oss.str();
}

void CTracer::writeInfo(std::ostream& out) const
{
    out << parametersToString();
}
