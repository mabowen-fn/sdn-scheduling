#pragma once

#include "types.hpp"
#include "chromosome.hpp"
#include <vector>
#include <algorithm>

namespace sdn {

// ============================================================================
// POPULATION CLASS
// ============================================================================

/**
 * Represents a population of chromosomes in NSGA-III.
 * 
 * A population is a collection of candidate solutions being evolved.
 * It manages:
 * - Storage of chromosomes
 * - Sorting and ranking
 * - Selection operations
 * - Statistics and metrics
 */
class Population {
public:
    // ========================================================================
    // CONSTRUCTORS & LIFECYCLE
    // ========================================================================
    
    /**
     * Construct an empty population.
     */
    Population() = default;
    
    /**
     * Construct a population with initial size.
     * 
     * @param size Initial population size
     * @param num_tasks Number of tasks per chromosome
     * @param num_edge_nodes Number of available edge nodes
     */
    Population(std::size_t size, std::size_t num_tasks, std::uint32_t num_edge_nodes);
    
    /**
     * Construct population from existing chromosomes.
     * 
     * @param chromosomes Vector of chromosomes to initialize with
     */
    explicit Population(std::vector<Chromosome> chromosomes);
    
    // Allow copy and move
    Population(const Population&) = default;
    Population& operator=(const Population&) = default;
    Population(Population&&) = default;
    Population& operator=(Population&&) = default;
    
    ~Population() = default;
    
    // ========================================================================
    // ACCESS & QUERY
    // ========================================================================
    
    /// Get population size
    std::size_t size() const { return individuals_.size(); }
    
    /// Check if population is empty
    bool empty() const { return individuals_.empty(); }
    
    /// Get all chromosomes
    const std::vector<Chromosome>& individuals() const { return individuals_; }
    
    /// Get mutable access to chromosomes
    std::vector<Chromosome>& individuals_mut() { return individuals_; }
    
    /// Get reference to individual at index
    const Chromosome& operator[](std::size_t idx) const { return individuals_[idx]; }
    
    /// Get mutable reference to individual at index
    Chromosome& operator[](std::size_t idx) { return individuals_[idx]; }
    
    /// Get individual at index (with bounds checking)
    const Chromosome& at(std::size_t idx) const { return individuals_.at(idx); }
    Chromosome& at(std::size_t idx) { return individuals_.at(idx); }
    
    // ========================================================================
    // MODIFICATION
    // ========================================================================
    
    /// Add chromosome to population
    void add(const Chromosome& chromosome) {
        individuals_.push_back(chromosome);
    }
    
    /// Add chromosome to population (move)
    void add(Chromosome&& chromosome) {
        individuals_.push_back(std::move(chromosome));
    }
    
    /// Add multiple chromosomes
    void add_all(const std::vector<Chromosome>& chromosomes) {
        individuals_.insert(individuals_.end(), chromosomes.begin(), chromosomes.end());
    }
    
    /// Clear population
    void clear() {
        individuals_.clear();
    }
    
    /// Replace entire population
    void set_individuals(std::vector<Chromosome> chromosomes) {
        individuals_ = std::move(chromosomes);
    }
    
    /// Resize population to specific size (removes excess individuals)
    void resize(std::size_t new_size) {
        if (new_size < individuals_.size()) {
            individuals_.erase(individuals_.begin() + new_size, individuals_.end());
        }
    }
    
    // ========================================================================
    // STATISTICS
    // ========================================================================
    
    /**
     * Get best individual by completion time.
     * 
     * @return Reference to chromosome with minimum completion time
     * @throws CalculationException if population empty or fitness not set
     */
    const Chromosome& best_by_completion_time() const;
    
    /**
     * Get best individual by energy consumption.
     * 
     * @return Reference to chromosome with minimum energy
     * @throws CalculationException if population empty or fitness not set
     */
    const Chromosome& best_by_energy() const;
    
    /**
     * Get best individual by success rate.
     * 
     * @return Reference to chromosome with maximum success rate
     * @throws CalculationException if population empty or fitness not set
     */
    const Chromosome& best_by_success_rate() const;
    
    /**
     * Calculate average completion time in population.
     * 
     * @return Average completion time (ms)
     * @throws CalculationException if fitness not calculated
     */
    double average_completion_time() const;
    
    /**
     * Calculate average energy consumption in population.
     * 
     * @return Average energy consumption (J)
     * @throws CalculationException if fitness not calculated
     */
    double average_energy_consumption() const;
    
    /**
     * Calculate average success rate in population.
     * 
     * @return Average success rate [0, 1]
     * @throws CalculationException if fitness not calculated
     */
    double average_success_rate() const;
    
    /**
     * Count individuals on Pareto front (rank 0).
     * 
     * @return Number of non-dominated individuals
     */
    std::size_t count_rank_zero() const;
    
    // ========================================================================
    // SORTING & RANKING
    // ========================================================================
    
    /**
     * Sort population by rank (ascending) then crowding distance (descending).
     * 
     * This is the standard NSGA-III selection order.
     */
    void sort_by_rank_and_distance();
    
    /**
     * Sort population by completion time (ascending).
     */
    void sort_by_completion_time();
    
    /**
     * Sort population by energy consumption (ascending).
     */
    void sort_by_energy_consumption();
    
    /**
     * Sort population by success rate (descending).
     */
    void sort_by_success_rate();
    
    // ========================================================================
    // SELECTION
    // ========================================================================
    
    /**
     * Select best N individuals by rank and distance.
     * 
     * Used in NSGA-III environmental selection.
     * 
     * @param count Number of individuals to select
     * @return New population with selected individuals
     */
    Population select_best(std::size_t count) const;
    
    /**
     * Select tournament of K random individuals.
     * 
     * @param k Tournament size
     * @return Winning chromosome
     */
    const Chromosome& tournament_select(std::size_t k = 2) const;
    
    // ========================================================================
    // METRICS
    // ========================================================================
    
    /**
     * Calculate hypervolume of population (approximation).
     * 
     * Uses weighted sum approach for 3D objective space.
     * 
     * @param ideal_completion Ideal (minimum) completion time
     * @param ideal_energy Ideal (minimum) energy consumption
     * @return Hypervolume estimate
     */
    double calculate_hypervolume(
        TimeMs ideal_completion,
        EnergyJ ideal_energy) const;

private:
    std::vector<Chromosome> individuals_;
};

}  // namespace sdn
