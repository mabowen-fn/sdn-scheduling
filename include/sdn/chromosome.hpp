#pragma once

#include "types.hpp"
#include "scheduling_strategy.hpp"
#include <vector>
#include <map>
#include <optional>

namespace sdn {

// ============================================================================
// CHROMOSOME CLASS
// ============================================================================

/**
 * Represents a candidate solution as a chromosome in NSGA-III.
 * 
 * A chromosome encodes a scheduling strategy as an integer array where:
 * - Each element represents a task ID (0 to num_tasks-1)
 * - The value at that position is the assigned edge node ID (0 to num_nodes-1)
 * - Special value: num_nodes indicates assignment to data center
 * 
 * This compact representation enables efficient genetic operations.
 */
class Chromosome {
public:
    // ========================================================================
    // CONSTRUCTORS & LIFECYCLE
    // ========================================================================
    
    /**
     * Construct a chromosome with given genome size.
     * 
     * @param num_tasks Number of tasks (genome length)
     * @param num_edge_nodes Number of available edge nodes
     */
    Chromosome(std::size_t num_tasks, std::uint32_t num_edge_nodes);
    
    /**
     * Construct chromosome from explicit genome.
     * 
     * @param genome Integer vector where genome[task_id] = edge_node_id
     * @param num_edge_nodes Number of available edge nodes
     */
    Chromosome(const std::vector<std::uint32_t>& genome, std::uint32_t num_edge_nodes);
    
    /**
     * Construct chromosome from scheduling strategy.
     * 
     * Encodes the strategy into compact genome representation.
     * 
     * @param strategy The scheduling strategy to encode
     * @param num_edge_nodes Number of available edge nodes
     */
    Chromosome(const SchedulingStrategy& strategy, std::uint32_t num_edge_nodes);
    
    // Delete default constructor
    Chromosome() = delete;
    
    // Allow copy and move
    Chromosome(const Chromosome&) = default;
    Chromosome& operator=(const Chromosome&) = default;
    Chromosome(Chromosome&&) = default;
    Chromosome& operator=(Chromosome&&) = default;
    
    ~Chromosome() = default;
    
    // ========================================================================
    // ACCESSORS
    // ========================================================================
    
    /// Get the complete genome
    const std::vector<std::uint32_t>& genome() const { return genome_; }
    
    /// Get genome as mutable (for mutation operations)
    std::vector<std::uint32_t>& genome_mut() { return genome_; }
    
    /// Get genome size (number of tasks)
    std::size_t size() const { return genome_.size(); }
    
    /// Get number of available edge nodes
    std::uint32_t num_edge_nodes() const { return num_edge_nodes_; }
    
    /// Get assignment for a specific task
    std::uint32_t get_assignment(std::size_t task_idx) const;
    
    /// Check if task is assigned to edge node (vs data center)
    bool is_assigned_to_edge(std::size_t task_idx) const;
    
    /// Check if task is assigned to data center
    bool is_assigned_to_datacenter(std::size_t task_idx) const;
    
    // ========================================================================
    // MODIFICATIONS
    // ========================================================================
    
    /// Set assignment for a task
    void set_assignment(std::size_t task_idx, std::uint32_t edge_node_id);
    
    /// Assign task to specific edge node
    void assign_to_edge_node(std::size_t task_idx, std::uint32_t edge_node_id);
    
    /// Assign task to data center
    void assign_to_datacenter(std::size_t task_idx);
    
    // ========================================================================
    // FITNESS & OBJECTIVES
    // ========================================================================
    
    /// Get completion time (ms), if calculated
    std::optional<TimeMs> completion_time() const { return completion_time_; }
    
    /// Set completion time
    void set_completion_time(TimeMs time) { completion_time_ = time; }
    
    /// Get energy consumption (Joules), if calculated
    std::optional<EnergyJ> energy_consumption() const { return energy_consumption_; }
    
    /// Set energy consumption
    void set_energy_consumption(EnergyJ energy) { energy_consumption_ = energy; }
    
    /// Get success rate [0,1], if calculated
    std::optional<double> success_rate() const { return success_rate_; }
    
    /// Set success rate
    void set_success_rate(double rate) {
        if (rate < 0.0 || rate > 1.0) {
            throw CalculationException("Success rate must be in [0, 1]");
        }
        success_rate_ = rate;
    }
    
    /// Clear all fitness values
    void clear_fitness() {
        completion_time_ = std::nullopt;
        energy_consumption_ = std::nullopt;
        success_rate_ = std::nullopt;
    }
    
    // ========================================================================
    // RANK & CROWDING DISTANCE (NSGA-III METRICS)
    // ========================================================================
    
    /// Get NSGA-III rank (0 = best)
    std::uint32_t rank() const { return rank_; }
    
    /// Set NSGA-III rank
    void set_rank(std::uint32_t r) { rank_ = r; }
    
    /// Get crowding distance
    double crowding_distance() const { return crowding_distance_; }
    
    /// Set crowding distance
    void set_crowding_distance(double dist) { crowding_distance_ = dist; }
    
    /// Reset NSGA-III metrics
    void reset_nsga_metrics() {
        rank_ = 0;
        crowding_distance_ = 0.0;
    }
    
    // ========================================================================
    // CONVERSION
    // ========================================================================
    
    /**
     * Convert chromosome to scheduling strategy.
     * 
     * @param workflow_id The workflow this strategy is for
     * @return SchedulingStrategy decoded from this chromosome
     */
    SchedulingStrategy to_strategy(WorkflowId workflow_id) const;
    
    // ========================================================================
    // COMPARISON
    // ========================================================================
    
    /// Check equality
    bool operator==(const Chromosome& other) const {
        return genome_ == other.genome_;
    }
    
    /// Check inequality
    bool operator!=(const Chromosome& other) const {
        return !(*this == other);
    }
    
    // ========================================================================
    // VALIDATION
    // ========================================================================
    
    /**
     * Validate chromosome integrity.
     * 
     * Checks that all assignments are valid.
     * 
     * @throws InvalidScheduleException if validation fails
     */
    void validate() const;

private:
    std::vector<std::uint32_t> genome_;     // genome[task_id] = edge_node_id
    std::uint32_t num_edge_nodes_;          // num_nodes = data center marker
    
    // Fitness metrics
    mutable std::optional<TimeMs> completion_time_;
    mutable std::optional<EnergyJ> energy_consumption_;
    mutable std::optional<double> success_rate_;
    
    // NSGA-III metrics
    std::uint32_t rank_ = 0;
    double crowding_distance_ = 0.0;
};

}  // namespace sdn
