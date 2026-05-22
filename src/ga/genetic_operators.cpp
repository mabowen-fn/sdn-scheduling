#include "sdn/genetic_operators.hpp"
#include <algorithm>
#include <cmath>

namespace sdn {

// ============================================================================
// CONSTRUCTOR
// ============================================================================

GeneticOperators::GeneticOperators(
    std::size_t num_tasks,
    std::uint32_t num_edge_nodes,
    double crossover_prob,
    double mutation_prob,
    std::uint32_t seed)
    : num_tasks_(num_tasks),
      num_edge_nodes_(num_edge_nodes),
      crossover_prob_(crossover_prob),
      mutation_prob_(mutation_prob),
      rng_(seed == 0 ? std::random_device{}() : seed) {
    
    if (num_tasks == 0) {
        throw CalculationException("Number of tasks must be > 0");
    }
    if (crossover_prob < 0.0 || crossover_prob > 1.0) {
        throw CalculationException("Crossover probability must be in [0, 1]");
    }
    if (mutation_prob < 0.0 || mutation_prob > 1.0) {
        throw CalculationException("Mutation probability must be in [0, 1]");
    }
}

// ========================================================================
// CROSSOVER OPERATIONS
// ========================================================================

Chromosome GeneticOperators::crossover_two_point(
    const Chromosome& parent1,
    const Chromosome& parent2) const
{
    Chromosome child = parent1;
    
    // Select two random crossover points
    std::uniform_int_distribution<std::size_t> dist(0, num_tasks_ - 1);
    std::size_t point1 = dist(rng_);
    std::size_t point2 = dist(rng_);
    
    if (point1 > point2) std::swap(point1, point2);
    
    // Swap segment between points
    for (std::size_t i = point1; i <= point2 && i < num_tasks_; ++i) {
        child.set_assignment(i, parent2.get_assignment(i));
    }
    
    return child;
}

Chromosome GeneticOperators::crossover_uniform(
    const Chromosome& parent1,
    const Chromosome& parent2,
    double crossover_rate) const
{
    Chromosome child = parent1;
    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
    
    for (std::size_t i = 0; i < num_tasks_; ++i) {
        if (prob_dist(rng_) < crossover_rate) {
            child.set_assignment(i, parent2.get_assignment(i));
        }
    }
    
    return child;
}

Chromosome GeneticOperators::crossover(
    const Chromosome& parent1,
    const Chromosome& parent2,
    bool use_uniform) const
{
    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
    
    if (prob_dist(rng_) > crossover_prob_) {
        return parent1;  // No crossover, return copy of parent1
    }
    
    if (use_uniform) {
        return crossover_uniform(parent1, parent2);
    } else {
        return crossover_two_point(parent1, parent2);
    }
}

// ========================================================================
// MUTATION OPERATIONS
// ========================================================================

void GeneticOperators::mutate_random_assignment(Chromosome& chromosome) const
{
    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
    
    for (std::size_t i = 0; i < num_tasks_; ++i) {
        if (prob_dist(rng_) < mutation_prob_) {
            // Randomly assign to different edge node
            chromosome.set_assignment(i, random_edge_node());
        }
    }
}

void GeneticOperators::mutate_swap(Chromosome& chromosome) const
{
    std::uniform_int_distribution<std::size_t> index_dist(0, num_tasks_ - 1);
    
    std::size_t idx1 = index_dist(rng_);
    std::size_t idx2 = index_dist(rng_);
    
    if (idx1 != idx2) {
        std::swap(chromosome.genome_mut()[idx1], chromosome.genome_mut()[idx2]);
    }
}

void GeneticOperators::mutate(Chromosome& chromosome, bool use_swap) const
{
    if (use_swap) {
        mutate_swap(chromosome);
    } else {
        mutate_random_assignment(chromosome);
    }
}

void GeneticOperators::mutate_multiple(Chromosome& chromosome, std::size_t max_mutations) const
{
    std::uniform_int_distribution<std::size_t> dist(1, max_mutations);
    std::size_t num_mutations = dist(rng_);
    
    for (std::size_t i = 0; i < num_mutations; ++i) {
        mutate(chromosome, false);
    }
}

// ========================================================================
// SELECTION OPERATIONS
// ========================================================================

const Chromosome& GeneticOperators::roulette_wheel_select(const Population& population) const
{
    if (population.empty()) {
        throw CalculationException("Cannot select from empty population");
    }
    
    // Calculate fitness sum (using inverse for minimization objectives)
    double total_fitness = 0.0;
    for (const auto& individual : population.individuals()) {
        auto ct = individual.completion_time();
        auto energy = individual.energy_consumption();
        if (!ct || !energy) {
            throw CalculationException("Fitness not calculated");
        }
        
        // Inverted fitness: maximize if minimizing objective
        double fitness = 1.0 / (1.0 + ct.value() + energy.value());
        total_fitness += fitness;
    }
    
    // Roulette selection
    std::uniform_real_distribution<double> dist(0.0, total_fitness);
    double selection_point = dist(rng_);
    
    double cumulative = 0.0;
    for (const auto& individual : population.individuals()) {
        auto ct = individual.completion_time();
        auto energy = individual.energy_consumption();
        double fitness = 1.0 / (1.0 + ct.value() + energy.value());
        
        cumulative += fitness;
        if (cumulative >= selection_point) {
            return individual;
        }
    }
    
    // Fallback (shouldn't reach here)
    return population.individuals().back();
}

const Chromosome& GeneticOperators::tournament_select(
    const Population& population,
    std::size_t tournament_size) const
{
    if (population.empty()) {
        throw CalculationException("Cannot select from empty population");
    }
    
    return population.tournament_select(tournament_size);
}

const Chromosome& GeneticOperators::random_select(const Population& population) const
{
    if (population.empty()) {
        throw CalculationException("Cannot select from empty population");
    }
    
    std::uniform_int_distribution<std::size_t> dist(0, population.size() - 1);
    return population[dist(rng_)];
}

// ========================================================================
// HELPER METHODS
// ========================================================================

std::uint32_t GeneticOperators::random_edge_node() const
{
    std::uniform_int_distribution<std::uint32_t> dist(0, num_edge_nodes_);
    return dist(rng_);  // includes num_edge_nodes_ for data center
}

std::size_t GeneticOperators::random_index(std::size_t max) const
{
    std::uniform_int_distribution<std::size_t> dist(0, max - 1);
    return dist(rng_);
}

bool GeneticOperators::random_bool(double probability) const
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng_) < probability;
}

}  // namespace sdn
