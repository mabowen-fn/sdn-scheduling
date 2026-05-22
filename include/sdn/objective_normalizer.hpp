#pragma once

#include "types.hpp"
#include "population.hpp"
#include <map>
#include <vector>

namespace sdn {

// ============================================================================
// OBJECTIVE NORMALIZER CLASS
// ============================================================================

/**
 * Implements objective normalization for NSGA-III.
 * 
 * Normalizes multi-objective function values to comparable scale [0, 1].
 * 
 * Uses reference points and extreme points method:
 * - Ideal point = minimum on each objective
 * - Extreme points = solutions with worst value on other objectives
 * - Normalized = (value - ideal) / (extreme - ideal)
 */
class ObjectiveNormalizer {
public:
    /**
     * Construct objective normalizer.
     */
    ObjectiveNormalizer() = default;
    
    ~ObjectiveNormalizer() = default;
    
    // Delete copy/move (stateless utility)
    ObjectiveNormalizer(const ObjectiveNormalizer&) = delete;
    ObjectiveNormalizer& operator=(const ObjectiveNormalizer&) = delete;
    ObjectiveNormalizer(ObjectiveNormalizer&&) = delete;
    ObjectiveNormalizer& operator=(ObjectiveNormalizer&&) = delete;
    
    // ========================================================================
    // REFERENCE POINT CALCULATION
    // ========================================================================
    
    struct ObjectiveValues {
        TimeMs completion_time = 0;
        EnergyJ energy_consumption = 0.0;
        double success_rate = 0.0;
    };
    
    /**
     * Calculate ideal point (best value on each objective).
     * 
     * Ideal point = minimum on completion_time and energy, maximum on success_rate
     * 
     * @param population Population to analyze
     * @return Ideal objective values
     * @throws CalculationException if population empty or fitness not available
     */
    ObjectiveValues calculate_ideal_point(const Population& population) const;
    
    /**
     * Calculate extreme points (worst value on each individual objective).
     * 
     * Extreme point for objective i = solution minimizing on all other objectives
     * while maximizing objective i.
     * 
     * @param population Population to analyze
     * @return Vector of 3 extreme points (one per objective)
     * @throws CalculationException if fitness not available
     */
    std::vector<ObjectiveValues> calculate_extreme_points(const Population& population) const;
    
    /**
     * Calculate nadir point (worst value on each objective).
     * 
     * Nadir point = maximum on completion_time and energy, minimum on success_rate
     * 
     * @param population Population to analyze
     * @return Nadir objective values
     * @throws CalculationException if population empty or fitness not available
     */
    ObjectiveValues calculate_nadir_point(const Population& population) const;
    
    // ========================================================================
    // NORMALIZATION
    // ========================================================================
    
    /**
     * Normalize a single chromosome's objectives.
     * 
     * Normalizes to [0, 1] range relative to ideal and extreme points.
     * 
     * @param chromosome The chromosome to normalize
     * @param ideal_point The ideal point (minimum)
     * @param extreme_points The extreme points (boundaries)
     * @return Normalized objective values [0, 1]
     */
    ObjectiveValues normalize_chromosome(
        const Chromosome& chromosome,
        const ObjectiveValues& ideal_point,
        const std::vector<ObjectiveValues>& extreme_points) const;
    
    /**
     * Normalize entire population.
     * 
     * @param population Population to normalize (modified in-place)
     * @param ideal_point Ideal point for normalization
     * @param extreme_points Extreme points for normalization
     */
    void normalize_population(
        Population& population,
        const ObjectiveValues& ideal_point,
        const std::vector<ObjectiveValues>& extreme_points) const;
    
    // ========================================================================
    // SINGLE OBJECTIVE NORMALIZATION
    // ========================================================================
    
    /**
     * Normalize completion time value.
     * 
     * @param value Completion time to normalize
     * @param ideal Ideal (minimum) value
     * @param extreme Extreme (maximum) value
     * @return Normalized value [0, 1]
     */
    double normalize_time(TimeMs value, TimeMs ideal, TimeMs extreme) const;
    
    /**
     * Normalize energy consumption value.
     * 
     * @param value Energy to normalize
     * @param ideal Ideal (minimum) value
     * @param extreme Extreme (maximum) value
     * @return Normalized value [0, 1]
     */
    double normalize_energy(EnergyJ value, EnergyJ ideal, EnergyJ extreme) const;
    
    /**
     * Normalize success rate value (already in [0, 1], but can rescale).
     * 
     * Success rate is maximized, so normalization inverts:
     * normalized = 1 - (success_rate - ideal) / (extreme - ideal)
     * 
     * @param value Success rate to normalize
     * @param ideal Ideal (maximum) value
     * @param extreme Extreme (minimum) value
     * @return Normalized value [0, 1]
     */
    double normalize_success_rate(double value, double ideal, double extreme) const;
    
    // ========================================================================
    // STATISTICS
    // ========================================================================
    
    /**
     * Calculate normalization range (difference between ideal and extreme).
     * 
     * Used to assess diversity of solutions across objective space.
     * Larger range = more diverse population.
     * 
     * @param ideal_point Ideal point
     * @param extreme_points Extreme points
     * @return Vector of 3 ranges (one per objective)
     */
    std::vector<double> calculate_normalization_ranges(
        const ObjectiveValues& ideal_point,
        const std::vector<ObjectiveValues>& extreme_points) const;
    
    /**
     * Calculate condition number (spread ratio).
     * 
     * Indicates aspect ratio of objective space.
     * High condition number = population concentrated in one region.
     * 
     * @param ranges Normalization ranges
     * @return Condition number (>= 1)
     */
    double calculate_condition_number(const std::vector<double>& ranges) const;

private:
    // Helper methods for objective comparisons
    static constexpr double EPSILON = 1e-10;  // For floating point comparisons
};

}  // namespace sdn
