#pragma once

#include "types.hpp"
#include "chromosome.hpp"
#include "workflow.hpp"
#include "edge_node.hpp"
#include "energy_calculator.hpp"
#include "time_calculator.hpp"
#include "success_rate_calculator.hpp"
#include <map>
#include <vector>

namespace sdn {

// ============================================================================
// FITNESS EVALUATOR CLASS
// ============================================================================

/**
 * Evaluates fitness of chromosomes/strategies for multi-objective optimization.
 * 
 * Fitness consists of three objectives:
 * 1. Completion Time (to be minimized)
 * 2. Energy Consumption (to be minimized)
 * 3. Success Rate (to be maximized)
 * 
 * The evaluator computes these objectives for each chromosome and stores results.
 */
class FitnessEvaluator {
public:
    /**
     * Construct a fitness evaluator.
     * 
     * @param workflow The workflow to evaluate strategies for
     * @param edge_nodes Map of edge node ID to EdgeNode pointer
     * @param bandwidth Network bandwidth in Mbps
     * @param deadline_factor QoS deadline multiplier (default 1.5)
     */
    FitnessEvaluator(
        const Workflow& workflow,
        const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes,
        BandwidthMbps bandwidth,
        double deadline_factor = 1.5);
    
    ~FitnessEvaluator() = default;
    
    // Delete copy/move (holds references)
    FitnessEvaluator(const FitnessEvaluator&) = delete;
    FitnessEvaluator& operator=(const FitnessEvaluator&) = delete;
    FitnessEvaluator(FitnessEvaluator&&) = delete;
    FitnessEvaluator& operator=(FitnessEvaluator&&) = delete;
    
    // ========================================================================
    // SINGLE CHROMOSOME EVALUATION
    // ========================================================================
    
    /**
     * Evaluate all objectives for a single chromosome.
     * 
     * Computes:
     * - Completion time
     * - Energy consumption
     * - Success rate
     * 
     * Results are stored in the chromosome's fitness fields.
     * 
     * @param chromosome The chromosome to evaluate
     * @throws CalculationException if evaluation fails
     */
    void evaluate(Chromosome& chromosome) const;
    
    /**
     * Evaluate completion time objective only.
     * 
     * @param chromosome The chromosome to evaluate
     * @return Completion time in milliseconds
     * @throws CalculationException if calculation fails
     */
    TimeMs evaluate_completion_time(const Chromosome& chromosome) const;
    
    /**
     * Evaluate energy consumption objective only.
     * 
     * @param chromosome The chromosome to evaluate
     * @return Energy consumption in Joules
     * @throws CalculationException if calculation fails
     */
    EnergyJ evaluate_energy_consumption(const Chromosome& chromosome) const;
    
    /**
     * Evaluate success rate objective only.
     * 
     * @param chromosome The chromosome to evaluate
     * @return Success rate in [0, 1]
     * @throws CalculationException if calculation fails
     */
    double evaluate_success_rate(const Chromosome& chromosome) const;
    
    // ========================================================================
    // BATCH EVALUATION
    // ========================================================================
    
    /**
     * Evaluate all chromosomes in a population.
     * 
     * @param chromosomes Vector of chromosomes to evaluate (modified in-place)
     * @throws CalculationException if evaluation fails
     */
    void evaluate_population(std::vector<Chromosome>& chromosomes) const;
    
    /**
     * Check if a chromosome's fitness is already cached/valid.
     * 
     * @param chromosome The chromosome to check
     * @return true if all objectives are already calculated
     */
    bool is_fitness_valid(const Chromosome& chromosome) const;
    
    // ========================================================================
    // CONFIGURATION
    // ========================================================================
    
    /// Get deadline factor
    double deadline_factor() const { return deadline_factor_; }
    
    /// Set deadline factor (for QoS tuning)
    void set_deadline_factor(double factor) {
        if (factor < 1.0) {
            throw CalculationException("Deadline factor must be >= 1.0");
        }
        deadline_factor_ = factor;
    }
    
    /// Get network bandwidth
    BandwidthMbps bandwidth() const { return bandwidth_; }
    
private:
    const Workflow& workflow_;
    const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes_;
    BandwidthMbps bandwidth_;
    double deadline_factor_;
    
    // Helper calculation engines
    mutable EnergyCalculator energy_calc_;
    mutable TimeCalculator time_calc_;
    mutable SuccessRateCalculator success_calc_;
    
    // Helper methods
    SchedulingStrategy chromosome_to_strategy(const Chromosome& chromosome) const;
    std::map<TaskId, DistanceKm> get_default_task_distances() const;
};

}  // namespace sdn
