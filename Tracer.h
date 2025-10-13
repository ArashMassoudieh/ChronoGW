#pragma once
#include "TimeSeries.h"
#include <string>
#include <memory>
#include <optional>

/**
 * @brief Represents a tracer in groundwater with transport and transformation properties
 *
 * This class models tracer behavior including:
 * - Input time series
 * - Radioactive decay
 * - Retardation
 * - Production from parent tracers
 * - Mixing with old water
 */
class CTracer
{
public:
    // ========================================================================
    // Constructors
    // ========================================================================

    CTracer();
    CTracer(const std::string& name);
    CTracer(const CTracer& other);
    CTracer& operator=(const CTracer& other);
    ~CTracer() = default;

    // ========================================================================
    // Core Tracer Properties - Getters
    // ========================================================================

    const std::string& getName() const { return name_; }
    double getInputMultiplier() const { return input_multiplier_; }
    double getDecayRate() const { return decay_rate_; }
    double getRetardation() const { return retardation_; }
    double getOldWaterConcentration() const { return c_old_; }
    double getModernWaterConcentration() const { return c_modern_; }
    double getMaxFractionModern() const { return fm_max_; }
    bool hasVzDelay() const { return vz_delay_; }
    bool hasLinearProduction() const { return linear_production_; }

    const TimeSeries<double>& getInput() const { return input_; }
    const std::string& getSourceTracerName() const { return source_tracer_name_; }

    // ========================================================================
    // Core Tracer Properties - Setters
    // ========================================================================

    void setName(const std::string& name) { name_ = name; }
    void setInputMultiplier(double multiplier) { input_multiplier_ = multiplier; }
    void setDecayRate(double rate) { decay_rate_ = rate; }
    void setRetardation(double retard) { retardation_ = retard; }
    void setOldWaterConcentration(double conc) { c_old_ = conc; }
    void setModernWaterConcentration(double conc) { c_modern_ = conc; }
    void setMaxFractionModern(double fm) { fm_max_ = fm; }
    void setVzDelay(bool enabled) { vz_delay_ = enabled; }
    void setLinearProduction(bool enabled) { linear_production_ = enabled; }

    void setInput(const TimeSeries<double>& input) { input_ = input; }
    void setSourceTracerName(const std::string& source) { source_tracer_name_ = source; }

    // For backward compatibility with string-based setting
    bool setParameter(const std::string& param_name, double value);

    // ========================================================================
    // Serialization / Output
    // ========================================================================

    /**
     * @brief Get parameter summary as string
     * @return String representation of tracer parameters
     */
    std::string parametersToString() const;

    /**
     * @brief Write tracer information to output stream
     */
    void writeInfo(std::ostream& out) const;

    // ========================================================================
    // Constant Input Handling
    // ========================================================================

    void setConstantInput(double value);
    void clearConstantInput();
    bool hasConstantInput() const { return constant_input_; }
    double getConstantInputValue() const { return constant_input_value_; }

    // ========================================================================
    // Source Tracer Handling
    // ========================================================================

    void setSourceTracer(CTracer* source) { source_tracer_ = source; }
    CTracer* getSourceTracer() const { return source_tracer_; }
    bool hasSourceTracer() const { return source_tracer_ != nullptr; }

    // ========================================================================
    // Concentration Calculation
    // ========================================================================

    /**
     * @brief Calculate tracer concentration at a given time
     * @param time Time at which to calculate concentration
     * @param age_distribution Young water age distribution
     * @param fraction_old Fraction of old water in mixture
     * @param vz_delay Vadose zone delay time
     * @param fixed_old_conc Whether to use fixed old water concentration
     * @param age_old Age of old water component
     * @param fraction_modern Fraction of modern carbon
     * @return Calculated concentration
     */
    double calculateConcentration(
        double time,
        const TimeSeries<double>& age_distribution,
        double fraction_old,
        double vz_delay = 0.0,
        bool fixed_old_conc = false,
        double age_old = 100000.0,
        double fraction_modern = 0.0) const;

private:
    // ========================================================================
    // Private Helper Methods
    // ========================================================================

    /**
     * @brief Calculate young water component concentration
     */
    double calculateYoungWaterComponent(
        double time,
        const TimeSeries<double>& age_distribution,
        double vz_delay,
        double fraction_modern) const;

    /**
     * @brief Calculate concentration from parent tracer decay
     */
    double calculateFromParentDecay(
        double time,
        const TimeSeries<double>& age_distribution,
        double vz_delay,
        double fraction_modern) const;

    /**
     * @brief Calculate old water component concentration
     */
    double calculateOldWaterComponent(
        double time,
        double fraction_old,
        double vz_delay,
        double age_old,
        double fraction_modern,
        bool fixed_old_conc) const;

    // ========================================================================
    // Member Variables
    // ========================================================================

    std::string name_;                    ///< Tracer name/identifier
    TimeSeries<double> input_;            ///< Input concentration time series

    // Transport and transformation properties
    double input_multiplier_ = 1.0;       ///< Multiplier for input concentrations
    double decay_rate_ = 0.0;             ///< First-order decay rate (1/time)
    double retardation_ = 1.0;            ///< Retardation factor (>=1)

    // Concentration in different water components
    double c_old_ = 0.0;                  ///< Concentration in old water
    double c_modern_ = 0.0;               ///< Concentration in modern water
    double fm_max_ = 0.0;                 ///< Maximum fraction modern

    // Flags
    bool vz_delay_ = false;               ///< Include vadose zone delay
    bool linear_production_ = false;      ///< Use linear production model
    bool constant_input_ = false;         ///< Use constant input value
    double constant_input_value_ = 0.0;   ///< Constant input concentration

    // Source tracer for decay chain calculations
    std::string source_tracer_name_;      ///< Name of parent tracer
    CTracer* source_tracer_ = nullptr;    ///< Pointer to parent tracer
};

// ============================================================================
// Inline Implementations
// ============================================================================

inline void CTracer::setConstantInput(double value) {
    constant_input_ = true;
    constant_input_value_ = value;

    // Create simple two-point time series
    input_.clear();
    input_.append(0.0, value);
    input_.append(3000.0, value);
}

inline void CTracer::clearConstantInput() {
    constant_input_ = false;
    constant_input_value_ = 0.0;
}
