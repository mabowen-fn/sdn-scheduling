#pragma once

#include "types.hpp"
#include "chromosome.hpp"
#include "population.hpp"
#include <vector>
#include <random>

namespace sdn {

// ============================================================================
// GENETIC OPERATORS CLASS
// ============================================================================

/**
 * Implements genetic operators for NSGA-III.
 * 
 * Operators include:
 * - Crossover: Two-point crossover and uniform crossover
 * - Mutation: Random assignment mutation
 * - Selection: Roulette wheel and tournament selection
 */
class GeneticOperators {
public:
    /**
     * Construct genetic operators with configuration.
     * 
     * @param num_tasks Number of tasks (genome length)
     * @param num_edge_nodes Number of available edge nodes
     * @param crossover_prob Probability of crossover (0.0 to 1.0)
     * @param mutation_prob Probability of mutation per gene (0.0 to 1.0)
     * @param seed Random seed (0 = use random device)
     */
    GeneticOperators(
        std::size_t num_tasks,
        std::uint32_t num_edge_nodes,
        double crossover_prob = 0.8,
        double mutation_prob = 0.1,
        std::uint32_t seed = 0);
    
    ~GeneticOperators() = default;
    
    // Delete copy (mutable RNG state)
    GeneticOperators(const GeneticOperators&) = delete;
    GeneticOperators& operator=(const GeneticOperators&) = delete;
    
    // Allow move
    GeneticOperators(GeneticOperators&&) = default;
    GeneticOperators& operator=(GeneticOperators&&) = default;
    
    // ========================================================================
    // CROSSOVER OPERATIONS
    // ========================================================================
    
    /**
     * Perform two-point crossover.
     * 
     * Selects two random crossover points and swaps genome segments.
     * 
     * @param parent1 First parent chromosome
     * @param parent2 Second parent chromosome
     * @return Child chromosome (copy of parent1 with crossed-over genes)
     */
    Chromosome crossover_two_point(
        const Chromosome& parent1,
        const Chromosome& parent2) const;
    
    /**
     * Perform uniform crossover.
     * 
     * For each gene, randomly inherits from either parent.
     * 
     * @param parent1 First parent chromosome
     * @param parent2 Second parent chromosome
     * @param crossover_rate Probability of taking from parent2 (default 0.5)
     * @return Child chromosome
     */
    Chromosome crossover_uniform(
        const Chromosome& parent1,
        const Chromosome& parent2,
        double crossover_rate = 0.5) const;
    
    /**
     * Perform crossover with optional gene inheritance pattern.
     * 
     * Applies crossover with configured probability.
     * If crossover doesn't occur, returns copy of parent1.
     * 
     * @param parent1 First parent
     * @param parent2 Second parent
     * @param use_uniform Use uniform crossover (true) or two-point (false)
     * @return Child chromosome
     */
    Chromosome crossover(
        const Chromosome& parent1,
        const Chromosome& parent2,
        bool use_uniform = false) const;
    
    // ========================================================================
    // MUTATION OPERATIONS
    // ========================================================================
    
    /**
     * Perform random assignment mutation.
     * 
     * For each gene with probability mutation_prob:
     * - Randomly reassign task to different edge node or data center
     * 
     * @param chromosome Chromosome to mutate (modified in-place)
     */
    void mutate_random_assignment(Chromosome& chromosome) const;
    
    /**
     * Perform swap mutation.
     * 
     * Randomly selects two genes and swaps their assignments.
     * 
     * @param chromosome Chromosome to mutate (modified in-place)
     */
    void mutate_swap(Chromosome& chromosome) const;
    
    /**
     * Perform mutation with configurable type.
     * 
     * Applies mutation with configured probability.
     * 
     * @param chromosome Chromosome to mutate (modified in-place)
     * @param use_swap Use swap mutation (true) or random assignment (false)
     */
    void mutate(Chromosome& chromosome, bool use_swap = false) const;
    
    /**
     * Apply multiple mutations.
     * 
     * Repeatedly applies mutation with random count.
     * 
     * @param chromosome Chromosome to mutate (modified in-place)
     * @param max_mutations Maximum number of mutations to apply (default 3)
     */
    void mutate_multiple(Chromosome& chromosome, std::size_t max_mutations = 3) const;
    
    // ========================================================================
    // SELECTION OPERATIONS
    // ========================================================================
    
    /**
     * Roulette wheel selection based on fitness.
     * 
     * Selects individuals with probability proportional to fitness.
     * Requires all individuals have fitness calculated.
     * 
     * @param population Population to select from
     * @return Selected chromosome
     * @throws CalculationException if fitness not available
     */
    const Chromosome& roulette_wheel_select(const Population& population) const;
    
    /**
     * Tournament selection.
     * 
     * Randomly selects K individuals and returns best by rank.
     * 
     * @param population Population to select from
     * @param tournament_size Number of participants (default 2)
     * @return Selected chromosome
     */
    const Chromosome& tournament_select(
        const Population& population,
        std::size_t tournament_size = 2) const;
    
    /**
     * Random selection.
     * 
     * @param population Population to select from
     * @return Randomly selected chromosome
     */
    const Chromosome& random_select(const Population& population) const;
    
    // ========================================================================
    // CONFIGURATION
    // ========================================================================
    
    /// Get crossover probability
    double crossover_probability() const { return crossover_prob_; }
    
    /// Set crossover probability
    void set_crossover_probability(double prob) {
        if (prob < 0.0 || prob > 1.0) {
            throw CalculationException("Crossover probability must be in [0, 1]");
        }
        crossover_prob_ = prob;
    }
    
    /// Get mutation probability per gene
    double mutation_probability() const { return mutation_prob_; }
    
    /// Set mutation probability per gene
    void set_mutation_probability(double prob) {
        if (prob < 0.0 || prob > 1.0) {
            throw CalculationException("Mutation probability must be in [0, 1]");
        }
        mutation_prob_ = prob;
    }
    
    /// Get number of tasks (genome length)
    std::size_t num_tasks() const { return num_tasks_; }
    
    /// Get number of edge nodes
    std::uint32_t num_edge_nodes() const { return num_edge_nodes_; }

private:
    std::size_t num_tasks_;
    std::uint32_t num_edge_nodes_;
    double crossover_prob_;
    double mutation_prob_;
    
    // Random number generation
    mutable std::mt19937 rng_;
    
    // Helper methods
    std::uint32_t random_edge_node() const;
    std::size_t random_index(std::size_t max) const;
    bool random_bool(double probability) const;
};

}  // namespace sdn
