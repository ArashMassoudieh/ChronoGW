#pragma once

#include "TimeSeries.h"
#include <string>
#include <vector>

/**
 * @brief Represents a groundwater well with age distribution characteristics
 *
 * This class manages:
 * - Age distribution type and parameters
 * - Young/old water mixing fractions
 * - Vadose zone delay
 * - Fraction modern calculation
 */
class CWell
{
public:
    // ========================================================================
    // Constructors
    // ========================================================================

    CWell();
    explicit CWell(const std::string& name);
    CWell(const CWell& other);
    CWell& operator=(const CWell& other);
    ~CWell() = default;

    // ========================================================================
    // Basic Properties
    // ========================================================================

    std::string getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    std::string getDistributionType() const { return distribution_type_; }
    void setDistributionType(const std::string& type);

    const std::vector<double>& getParameters() const { return parameters_; }
    void setParameters(const std::vector<double>& params) { parameters_ = params; }

    // ========================================================================
    // Mixing Parameters
    // ========================================================================

    double getFractionOld() const { return fraction_old_; }
    void setFractionOld(double fraction) { fraction_old_ = fraction; }

    double getAgeOld() const { return age_old_; }
    void setAgeOld(double age) { age_old_ = age; }

    double getFractionModern() const { return fraction_modern_; }
    void setFractionModern(double fm) { fraction_modern_ = fm; }

    double getVzDelay() const { return vz_delay_; }
    void setVzDelay(double delay) { vz_delay_ = delay; }

    // ========================================================================
    // Histogram Distribution Parameters (if using "hist" distribution)
    // ========================================================================

    int getHistogramBinCount() const { return histogram_bin_count_; }
    void setHistogramBinCount(int count) { histogram_bin_count_ = count; }

    double getHistogramBinSize() const { return histogram_bin_size_; }
    void setHistogramBinSize(double size) { histogram_bin_size_ = size; }

    // ========================================================================
    // Age Distribution
    // ========================================================================

    /**
     * @brief Get the computed young age distribution
     */
    const TimeSeries<double>& getYoungAgeDistribution() const {
        return young_age_distribution_;
    }

    /**
     * @brief Create/update the age distribution based on current parameters
     * @param oldest_time Maximum age to consider
     * @param num_intervals Number of age intervals
     * @param multiplier Spacing multiplier for non-uniform grids
     */
    void createDistribution(double oldest_time, int num_intervals = 1000, double multiplier = 0.02);

    // ========================================================================
    // Parameter Setting (backward compatibility)
    // ========================================================================

    /**
     * @brief Set a parameter by name
     * @param name Parameter name (e.g., "f", "param[0]", "vz_delay")
     * @param value Parameter value
     * @return true if parameter was recognized and set
     */
    bool setParameter(const std::string& name, double value);

    /**
     * @brief Get number of parameters required for a distribution type
     * @param distribution_name Distribution type name
     * @param n_bins Number of bins (for histogram distribution)
     * @return Number of parameters needed
     */
    static int getParameterCount(const std::string& distribution_name, int n_bins = 0);

    // ========================================================================
    // Serialization / Output
    // ========================================================================

    /**
     * @brief Get parameter summary as string
     * @return String representation of well parameters
     */
    std::string parametersToString() const;

    /**
     * @brief Write well information to output stream
     */
    void writeInfo(std::ostream& out) const;

    // ========================================================================
    // Static Distribution Creation Functions
    // ========================================================================

    /**
     * @brief Create Dirac delta (piston flow) age distribution
     */
    static TimeSeries<double> createDiracDistribution(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    /**
     * @brief Create exponential age distribution
     */
    static TimeSeries<double> createExponentialDistribution(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    /**
     * @brief Create log-normal age distribution
     */
    static TimeSeries<double> createLogNormalDistribution(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    /**
     * @brief Create inverse Gaussian age distribution
     */
    static TimeSeries<double> createInverseGaussianDistribution(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    /**
     * @brief Create inverse Gaussian distribution with mu/lambda parameterization
     */
    static TimeSeries<double> createInverseGaussianDistribution_MuLambda(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    // Backward compatibility aliases
    static TimeSeries<double> creat_age_dist_InvG_mu_lambda(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier)
    {
        return createInverseGaussianDistribution_MuLambda(params, max_age, num_intervals, multiplier);
    }

    /**
     * @brief Create dispersion model age distribution
     */
    static TimeSeries<double> createDispersionDistribution(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    /**
     * @brief Create histogram-based age distribution
     */
    static TimeSeries<double> createHistogramDistribution(
        const std::vector<double>& params,
        int num_bins,
        double bin_size,
        double max_age,
        int num_intervals,
        double multiplier);

    /**
     * @brief Create Gamma age distribution
     */
    static TimeSeries<double> createGammaDistribution(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    /**
     * @brief Create Levy age distribution
     */
    static TimeSeries<double> createLevyDistribution(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    /**
     * @brief Create shifted Levy age distribution
     */
    static TimeSeries<double> createShiftedLevyDistribution(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    /**
     * @brief Create shifted exponential age distribution
     */
    static TimeSeries<double> createShiftedExponentialDistribution(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    /**
     * @brief Create generalized inverse Gaussian age distribution
     */
    static TimeSeries<double> createGeneralizedInverseGaussianDistribution(
        const std::vector<double>& params,
        double max_age,
        int num_intervals,
        double multiplier);

    // For backward compatibility - public access to some members
    std::string name;                  // Direct access for now
    std::string distribution;          // Direct access for now
    std::vector<double> params;        // Direct access for now
    TimeSeries<double> young_age_distribution;  // Direct access for now
    double fraction_old;               // Direct access for now
    double age_old;                    // Direct access for now
    double vz_delay;                   // Direct access for now
    double fm;                         // Direct access for now
    int Histogram_bin_num;             // Direct access for now
    double Histogram_binsize;          // Direct access for now

private:
    // ========================================================================
    // Member Variables (will eventually replace public ones above)
    // ========================================================================

    std::string name_;                      ///< Well identifier
    std::string distribution_type_;         ///< Type of age distribution
    std::vector<double> parameters_;        ///< Distribution parameters

    TimeSeries<double> young_age_distribution_;  ///< Computed age distribution

    // Mixing parameters
    double fraction_old_;                   ///< Fraction of old water (0-1)
    double age_old_;                        ///< Age of old water component
    double fraction_modern_;                ///< Fraction of modern carbon (0-1)
    double vz_delay_;                       ///< Vadose zone delay time

    // Histogram-specific parameters
    int histogram_bin_count_;               ///< Number of histogram bins
    double histogram_bin_size_;             ///< Size of each histogram bin
};
