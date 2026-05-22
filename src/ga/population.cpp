#include "sdn/population.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>

namespace sdn {

// ============================================================================
// CONSTRUCTORS
// ============================================================================

Population::Population(std::size_t size, std::size_t num_tasks, std::uint32_t num_edge_nodes) {
    individuals_.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        individuals_.emplace_back(num_tasks, num_edge_nodes);
    }
}

Population::Population(std::vector<Chromosome> chromosomes)
    : individuals_(std::move(chromosomes)) {}

// ============================================================================
// STATISTICS
// ============================================================================

const Chromosome& Population::best_by_completion_time() const {
    if (individuals_.empty()) {
        throw CalculationException("Cannot find best in empty population");
    }
    
    return *std::min_element(individuals_.begin(), individuals_.end(),
        [](const Chromosome& a, const Chromosome& b) {
            auto a_ct = a.completion_time();
            auto b_ct = b.completion_time();
            if (!a_ct || !b_ct) {
                throw CalculationException("Fitness not calculated for chromosome");
            }
            return a_ct.value() < b_ct.value();
        }
    );
}

const Chromosome& Population::best_by_energy() const {
    if (individuals_.empty()) {
        throw CalculationException("Cannot find best in empty population");
    }
    
    return *std::min_element(individuals_.begin(), individuals_.end(),
        [](const Chromosome& a, const Chromosome& b) {
            auto a_energy = a.energy_consumption();
            auto b_energy = b.energy_consumption();
            if (!a_energy || !b_energy) {
                throw CalculationException("Fitness not calculated for chromosome");
            }
            return a_energy.value() < b_energy.value();
        }
    );
}

const Chromosome& Population::best_by_success_rate() const {
    if (individuals_.empty()) {
        throw CalculationException("Cannot find best in empty population");
    }
    
    return *std::max_element(individuals_.begin(), individuals_.end(),
        [](const Chromosome& a, const Chromosome& b) {
            auto a_sr = a.success_rate();
            auto b_sr = b.success_rate();
            if (!a_sr || !b_sr) {
                throw CalculationException("Fitness not calculated for chromosome");
            }
            return a_sr.value() < b_sr.value();
        }
    );
}

double Population::average_completion_time() const {
    if (individuals_.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& indiv : individuals_) {
        auto ct = indiv.completion_time();
        if (!ct) throw CalculationException("Fitness not calculated");
        sum += ct.value();
    }
    return sum / individuals_.size();
}

double Population::average_energy_consumption() const {
    if (individuals_.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& indiv : individuals_) {
        auto energy = indiv.energy_consumption();
        if (!energy) throw CalculationException("Fitness not calculated");
        sum += energy.value();
    }
    return sum / individuals_.size();
}

double Population::average_success_rate() const {
    if (individuals_.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& indiv : individuals_) {
        auto sr = indiv.success_rate();
        if (!sr) throw CalculationException("Fitness not calculated");
        sum += sr.value();
    }
    return sum / individuals_.size();
}

std::size_t Population::count_rank_zero() const {
    return std::count_if(individuals_.begin(), individuals_.end(),
        [](const Chromosome& chr) { return chr.rank() == 0; }
    );
}

// ============================================================================
// SORTING & RANKING
// ============================================================================

void Population::sort_by_rank_and_distance() {
    std::sort(individuals_.begin(), individuals_.end(),
        [](const Chromosome& a, const Chromosome& b) {
            if (a.rank() != b.rank()) {
                return a.rank() < b.rank();
            }
            return a.crowding_distance() > b.crowding_distance();
        }
    );
}

void Population::sort_by_completion_time() {
    std::sort(individuals_.begin(), individuals_.end(),
        [](const Chromosome& a, const Chromosome& b) {
            auto a_ct = a.completion_time();
            auto b_ct = b.completion_time();
            if (!a_ct || !b_ct) return false;
            return a_ct.value() < b_ct.value();
        }
    );
}

void Population::sort_by_energy_consumption() {
    std::sort(individuals_.begin(), individuals_.end(),
        [](const Chromosome& a, const Chromosome& b) {
            auto a_e = a.energy_consumption();
            auto b_e = b.energy_consumption();
            if (!a_e || !b_e) return false;
            return a_e.value() < b_e.value();
        }
    );
}

void Population::sort_by_success_rate() {
    std::sort(individuals_.begin(), individuals_.end(),
        [](const Chromosome& a, const Chromosome& b) {
            auto a_sr = a.success_rate();
            auto b_sr = b.success_rate();
            if (!a_sr || !b_sr) return false;
            return a_sr.value() > b_sr.value();
        }
    );
}

// ========================================================================
// SELECTION
// ========================================================================

Population Population::select_best(std::size_t count) const {
    if (count > individuals_.size()) {
        throw CalculationException("Cannot select more than population size");
    }
    
    // Create copy and sort
    std::vector<Chromosome> sorted_copy = individuals_;
    std::sort(sorted_copy.begin(), sorted_copy.end(),
        [](const Chromosome& a, const Chromosome& b) {
            if (a.rank() != b.rank()) {
                return a.rank() < b.rank();
            }
            return a.crowding_distance() > b.crowding_distance();
        }
    );
    
    // Trim to size using erase
    if (sorted_copy.size() > count) {
        sorted_copy.erase(sorted_copy.begin() + count, sorted_copy.end());
    }
    return Population(std::move(sorted_copy));
}

const Chromosome& Population::tournament_select(std::size_t k) const {
    if (individuals_.empty()) {
        throw CalculationException("Cannot select from empty population");
    }
    
    if (k > individuals_.size()) {
        k = individuals_.size();
    }
    
    // Randomly select k individuals and return best by rank
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<std::size_t> dist(0, individuals_.size() - 1);
    
    const Chromosome* best = &individuals_[dist(rng)];
    for (std::size_t i = 1; i < k; ++i) {
        const Chromosome& candidate = individuals_[dist(rng)];
        if (candidate.rank() < best->rank() ||
            (candidate.rank() == best->rank() && 
             candidate.crowding_distance() > best->crowding_distance())) {
            best = &candidate;
        }
    }
    
    return *best;
}

// ========================================================================
// METRICS
// ========================================================================

double Population::calculate_hypervolume(
    TimeMs ideal_completion,
    EnergyJ ideal_energy) const
{
    if (individuals_.empty()) {
        return 0.0;
    }
    
    double hypervolume = 0.0;
    
    for (const auto& indiv : individuals_) {
        auto ct = indiv.completion_time();
        auto energy = indiv.energy_consumption();
        if (!ct || !energy) continue;
        
        // Normalized contribution
        double normalized_ct = 1.0 - (ct.value() > 0 ? static_cast<double>(ct.value()) / ideal_completion : 0);
        double normalized_energy = 1.0 - (energy.value() > 0 ? energy.value() / ideal_energy : 0);
        
        normalized_ct = std::max(0.0, normalized_ct);
        normalized_energy = std::max(0.0, normalized_energy);
        
        hypervolume += normalized_ct * normalized_energy;
    }
    
    return hypervolume / individuals_.size();
}

}  // namespace sdn
