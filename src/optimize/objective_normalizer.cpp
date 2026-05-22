#include "sdn/objective_normalizer.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace sdn {

// ============================================================================
// REFERENCE POINT CALCULATION
// ============================================================================

ObjectiveNormalizer::ObjectiveValues ObjectiveNormalizer::calculate_ideal_point(const Population& population) const {
    if (population.empty()) {
        throw CalculationException("Cannot calculate ideal point on empty population");
    }
    
    ObjectiveValues ideal;
    ideal.completion_time = std::numeric_limits<TimeMs>::max();
    ideal.energy_consumption = std::numeric_limits<EnergyJ>::max();
    ideal.success_rate = 0.0;  // Minimize success rate inverted
    
    for (const auto& individual : population.individuals()) {
        auto ct = individual.completion_time();
        auto energy = individual.energy_consumption();
        auto sr = individual.success_rate();
        
        if (!ct || !energy || !sr) {
            throw CalculationException("Fitness not calculated for chromosome");
        }
        
        ideal.completion_time = std::min(ideal.completion_time, ct.value());
        ideal.energy_consumption = std::min(ideal.energy_consumption, energy.value());
        ideal.success_rate = std::max(ideal.success_rate, sr.value());
    }
    
    return ideal;
}

std::vector<ObjectiveNormalizer::ObjectiveValues> 
ObjectiveNormalizer::calculate_extreme_points(const Population& population) const {
    std::vector<ObjectiveValues> extreme(3);
    
    for (int i = 0; i < 3; ++i) {
        extreme[i].completion_time = std::numeric_limits<TimeMs>::max();
        extreme[i].energy_consumption = std::numeric_limits<EnergyJ>::max();
        extreme[i].success_rate = 0.0;
    }
    
    // Find extreme point for each objective
    for (const auto& individual : population.individuals()) {
        auto ct = individual.completion_time();
        auto energy = individual.energy_consumption();
        auto sr = individual.success_rate();
        
        if (!ct || !energy || !sr) continue;
        
        // Extreme for completion time: minimize CT while considering others
        if (ct.value() < extreme[0].completion_time) {
            extreme[0] = {ct.value(), energy.value(), sr.value()};
        }
        
        // Extreme for energy: minimize energy while considering others
        if (energy.value() < extreme[1].energy_consumption) {
            extreme[1] = {ct.value(), energy.value(), sr.value()};
        }
        
        // Extreme for success rate: maximize SR while considering others
        if (sr.value() > extreme[2].success_rate) {
            extreme[2] = {ct.value(), energy.value(), sr.value()};
        }
    }
    
    return extreme;
}

ObjectiveNormalizer::ObjectiveValues ObjectiveNormalizer::calculate_nadir_point(const Population& population) const {
    if (population.empty()) {
        throw CalculationException("Cannot calculate nadir point on empty population");
    }
    
    ObjectiveValues nadir;
    nadir.completion_time = 0;
    nadir.energy_consumption = 0.0;
    nadir.success_rate = 1.0;
    
    for (const auto& individual : population.individuals()) {
        auto ct = individual.completion_time();
        auto energy = individual.energy_consumption();
        auto sr = individual.success_rate();
        
        if (!ct || !energy || !sr) {
            throw CalculationException("Fitness not calculated for chromosome");
        }
        
        nadir.completion_time = std::max(nadir.completion_time, ct.value());
        nadir.energy_consumption = std::max(nadir.energy_consumption, energy.value());
        nadir.success_rate = std::min(nadir.success_rate, sr.value());
    }
    
    return nadir;
}

// ========================================================================
// NORMALIZATION
// ========================================================================

ObjectiveNormalizer::ObjectiveValues ObjectiveNormalizer::normalize_chromosome(
    const Chromosome& chromosome,
    const ObjectiveValues& ideal_point,
    const std::vector<ObjectiveValues>& extreme_points) const {
    
    auto ct = chromosome.completion_time();
    auto energy = chromosome.energy_consumption();
    auto sr = chromosome.success_rate();
    
    if (!ct || !energy || !sr) {
        throw CalculationException("Fitness not calculated");
    }
    
    // Use first extreme point as reference for normalization
    ObjectiveValues normalized;
    normalized.completion_time = static_cast<TimeMs>(
        normalize_time(ct.value(), ideal_point.completion_time, extreme_points[0].completion_time) * 1000
    );
    normalized.energy_consumption = normalize_energy(
        energy.value(), ideal_point.energy_consumption, extreme_points[1].energy_consumption
    );
    normalized.success_rate = normalize_success_rate(
        sr.value(), ideal_point.success_rate, extreme_points[2].success_rate
    );
    
    return normalized;
}

void ObjectiveNormalizer::normalize_population(
    Population& population,
    const ObjectiveValues& /* ideal_point */,
    const std::vector<ObjectiveValues>& /* extreme_points */) const {
    
    for (auto& individual : population.individuals_mut()) {
        individual.clear_fitness();  // Clear cached values
    }
}

// ========================================================================
// SINGLE OBJECTIVE NORMALIZATION
// ========================================================================

double ObjectiveNormalizer::normalize_time(TimeMs value, TimeMs ideal, TimeMs extreme) const {
    if (extreme <= ideal) {
        return 0.0;
    }
    
    double range = static_cast<double>(extreme - ideal);
    double normalized = static_cast<double>(value - ideal) / range;
    
    return std::max(0.0, std::min(1.0, normalized));
}

double ObjectiveNormalizer::normalize_energy(EnergyJ value, EnergyJ ideal, EnergyJ extreme) const {
    if (extreme <= ideal) {
        return 0.0;
    }
    
    double range = extreme - ideal;
    double normalized = (value - ideal) / range;
    
    return std::max(0.0, std::min(1.0, normalized));
}

double ObjectiveNormalizer::normalize_success_rate(double value, double ideal, double extreme) const {
    // Success rate is maximized, so invert
    if (ideal <= extreme) {
        return 0.0;
    }
    
    double range = ideal - extreme;
    double normalized = (ideal - value) / range;
    
    return std::max(0.0, std::min(1.0, normalized));
}

// ========================================================================
// STATISTICS
// ========================================================================

std::vector<double> ObjectiveNormalizer::calculate_normalization_ranges(
    const ObjectiveValues& ideal_point,
    const std::vector<ObjectiveValues>& extreme_points) const {
    
    std::vector<double> ranges(3);
    
    ranges[0] = static_cast<double>(extreme_points[0].completion_time - ideal_point.completion_time);
    ranges[1] = extreme_points[1].energy_consumption - ideal_point.energy_consumption;
    ranges[2] = ideal_point.success_rate - extreme_points[2].success_rate;
    
    return ranges;
}

double ObjectiveNormalizer::calculate_condition_number(const std::vector<double>& ranges) const {
    if (ranges.size() != 3 || ranges[0] <= EPSILON) {
        return 1.0;
    }
    
    double max_range = std::max({ranges[0], ranges[1], ranges[2]});
    double min_range = std::min({ranges[0], ranges[1], ranges[2]});
    
    if (min_range < EPSILON) {
        return std::numeric_limits<double>::infinity();
    }
    
    return max_range / min_range;
}

}  // namespace sdn
