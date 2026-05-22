#pragma once

#include "types.hpp"
#include "chromosome.hpp"
#include "population.hpp"
#include <vector>
#include <set>

namespace sdn {

// ============================================================================
// NONDOMINATED SORTING ENGINE CLASS
// ============================================================================

/**
 * Implements non-dominated sorting for NSGA-III.
 * 
 * Fast non-dominated sorting (O(MN²) where M=objectives, N=population_size):
 * 1. Classifies population into fronts based on dominance
 * 2. Assigns rank to each individual
 * 3. Front 0 contains non-dominated solutions (Pareto front)
 */
class NonDominatedSorting {
public:
    /**
     * Construct non-dominated sorting engine.
     */
    NonDominatedSorting() = default;
    
    ~NonDominatedSorting() = default;
    
    // Delete copy/move (stateless utility)
    NonDominatedSorting(const NonDominatedSorting&) = delete;
    NonDominatedSorting& operator=(const NonDominatedSorting&) = delete;
    NonDominatedSorting(NonDominatedSorting&&) = delete;
    NonDominatedSorting& operator=(NonDominatedSorting&&) = delete;
    
    // ========================================================================
    // DOMINANCE CHECKING
    // ========================================================================
    
    /**
     * Check if chromosome a dominates chromosome b.
     * 
     * Chromosome a dominates b if:
     * - a is at least as good as b on all objectives, AND
     * - a is strictly better on at least one objective
     * 
     * Minimization assumed for completion time and energy.
     * Maximization assumed for success rate.
     * 
     * @param a First chromosome
     * @param b Second chromosome
     * @return true if a strictly dominates b
     * @throws CalculationException if fitness not available
     */
    bool dominates(const Chromosome& a, const Chromosome& b) const;
    
    /**
     * Check if chromosomes a and b are non-dominated (incomparable).
     * 
     * @param a First chromosome
     * @param b Second chromosome
     * @return true if neither dominates the other
     */
    bool are_non_dominated(const Chromosome& a, const Chromosome& b) const {
        return !dominates(a, b) && !dominates(b, a);
    }
    
    // ========================================================================
    // SORTING OPERATIONS
    // ========================================================================
    
    /**
     * Perform fast non-dominated sorting on population.
     * 
     * Classifies population into fronts and assigns ranks.
     * Front 0 = Pareto optimal solutions
     * Front 1 = dominated only by front 0
     * etc.
     * 
     * @param population Population to sort (modified in-place with ranks)
     * @return Vector of fronts (each front is vector of individual indices)
     * @throws CalculationException if fitness not available
     */
    std::vector<std::vector<std::size_t>> sort_population(Population& population) const;
    
    /**
     * Get all individuals in rank 0 (Pareto front).
     * 
     * @param population The population (must be sorted first)
     * @return Vector of non-dominated solutions
     */
    std::vector<const Chromosome*> get_pareto_front(const Population& population) const;
    
    /**
     * Get individuals by rank.
     * 
     * @param population The population (must be sorted first)
     * @param rank The rank to retrieve (0 = Pareto front)
     * @return Vector of chromosomes with given rank
     */
    std::vector<const Chromosome*> get_front_by_rank(const Population& population, std::uint32_t rank) const;
    
    // ========================================================================
    // STATISTICS
    // ========================================================================
    
    /**
     * Calculate percentage of population on Pareto front.
     * 
     * @param population The population
     * @return Percentage [0, 100] on rank 0
     */
    double calculate_front_percentage(const Population& population) const;

private:
    /**
     * Compare two objective vectors considering minimization/maximization.
     * 
     * Returns:
     * -1 if a < b (a is better)
     *  0 if a == b (equal)
     *  1 if a > b (b is better)
     */
    int compare_objectives(const Chromosome& a, const Chromosome& b) const;
};

}  // namespace sdn
