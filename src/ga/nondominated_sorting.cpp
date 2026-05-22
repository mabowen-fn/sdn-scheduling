#include "sdn/nondominated_sorting.hpp"

namespace sdn {

// ============================================================================
// DOMINANCE CHECKING
// ============================================================================

bool NonDominatedSorting::dominates(const Chromosome& a, const Chromosome& b) const {
    auto a_ct = a.completion_time();
    auto a_energy = a.energy_consumption();
    auto a_sr = a.success_rate();
    
    auto b_ct = b.completion_time();
    auto b_energy = b.energy_consumption();
    auto b_sr = b.success_rate();
    
    if (!a_ct || !a_energy || !a_sr || !b_ct || !b_energy || !b_sr) {
        throw CalculationException("Fitness not calculated for chromosome");
    }
    
    // a dominates b if:
    // - a.completion_time <= b.completion_time
    // - a.energy <= b.energy
    // - a.success_rate >= b.success_rate
    // AND at least one is strictly better
    
    bool ct_better_or_equal = a_ct.value() <= b_ct.value();
    bool energy_better_or_equal = a_energy.value() <= b_energy.value();
    bool sr_better_or_equal = a_sr.value() >= b_sr.value();
    
    bool ct_strictly_better = a_ct.value() < b_ct.value();
    bool energy_strictly_better = a_energy.value() < b_energy.value();
    bool sr_strictly_better = a_sr.value() > b_sr.value();
    
    bool all_better_or_equal = ct_better_or_equal && energy_better_or_equal && sr_better_or_equal;
    bool at_least_one_strictly_better = ct_strictly_better || energy_strictly_better || sr_strictly_better;
    
    return all_better_or_equal && at_least_one_strictly_better;
}

// ========================================================================
// SORTING OPERATIONS
// ========================================================================

std::vector<std::vector<std::size_t>> NonDominatedSorting::sort_population(Population& population) const {
    std::vector<std::vector<std::size_t>> fronts;
    std::vector<std::uint32_t> ranks(population.size(), 0);
    std::vector<bool> ranked(population.size(), false);
    
    std::uint32_t current_rank = 0;
    
    while (true) {
        std::vector<std::size_t> current_front;
        
        // Find all unranked individuals that are not dominated by ranked individuals
        for (std::size_t i = 0; i < population.size(); ++i) {
            if (ranked[i]) continue;
            
            bool is_dominated = false;
            for (std::size_t j = 0; j < population.size(); ++j) {
                if (i != j && !ranked[j]) {
                    if (dominates(population[j], population[i])) {
                        is_dominated = true;
                        break;
                    }
                }
            }
            
            if (!is_dominated) {
                current_front.push_back(i);
            }
        }
        
        if (current_front.empty()) {
            break;
        }
        
        // Mark this front
        for (std::size_t idx : current_front) {
            ranked[idx] = true;
            ranks[idx] = current_rank;
            population[idx].set_rank(current_rank);
        }
        
        fronts.push_back(std::move(current_front));
        current_rank++;
    }
    
    return fronts;
}

std::vector<const Chromosome*> NonDominatedSorting::get_pareto_front(const Population& population) const {
    std::vector<const Chromosome*> front;
    for (const auto& individual : population.individuals()) {
        if (individual.rank() == 0) {
            front.push_back(&individual);
        }
    }
    return front;
}

std::vector<const Chromosome*> NonDominatedSorting::get_front_by_rank(
    const Population& population,
    std::uint32_t rank) const {
    std::vector<const Chromosome*> front;
    for (const auto& individual : population.individuals()) {
        if (individual.rank() == rank) {
            front.push_back(&individual);
        }
    }
    return front;
}

// ========================================================================
// STATISTICS
// ========================================================================

double NonDominatedSorting::calculate_front_percentage(const Population& population) const {
    if (population.empty()) {
        return 0.0;
    }
    
    std::size_t rank_zero_count = population.count_rank_zero();
    return (static_cast<double>(rank_zero_count) / population.size()) * 100.0;
}

}  // namespace sdn
