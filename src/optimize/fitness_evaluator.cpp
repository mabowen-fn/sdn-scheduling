#include "sdn/fitness_evaluator.hpp"
#include "sdn/workflow.hpp"
#include "sdn/task.hpp"
#include "sdn/edge_node.hpp"

namespace sdn {

// ============================================================================
// CONSTRUCTOR
// ============================================================================

FitnessEvaluator::FitnessEvaluator(
    const Workflow& workflow,
    const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes,
    BandwidthMbps bandwidth,
    double deadline_factor)
    : workflow_(workflow),
      edge_nodes_(edge_nodes),
      bandwidth_(bandwidth),
      deadline_factor_(deadline_factor),
      energy_calc_(),
      time_calc_(),
      success_calc_() {
    
    if (deadline_factor < 1.0) {
        throw CalculationException("Deadline factor must be >= 1.0");
    }
    if (bandwidth == 0) {
        throw CalculationException("Bandwidth cannot be zero");
    }
}

// ============================================================================
// SINGLE CHROMOSOME EVALUATION
// ============================================================================

void FitnessEvaluator::evaluate(Chromosome& chromosome) const {
    if (is_fitness_valid(chromosome)) {
        return;  // Already evaluated
    }
    
    try {
        auto completion_time = evaluate_completion_time(chromosome);
        auto energy_consumption = evaluate_energy_consumption(chromosome);
        auto success_rate = evaluate_success_rate(chromosome);
        
        chromosome.set_completion_time(completion_time);
        chromosome.set_energy_consumption(energy_consumption);
        chromosome.set_success_rate(success_rate);
    } catch (const std::exception& e) {
        throw CalculationException(std::string("Fitness evaluation failed: ") + e.what());
    }
}

TimeMs FitnessEvaluator::evaluate_completion_time(const Chromosome& chromosome) const {
    auto strategy = chromosome_to_strategy(chromosome);
    auto task_distances = get_default_task_distances();
    
    return time_calc_.calculate_workflow_completion_time(
        workflow_, strategy, edge_nodes_, bandwidth_, task_distances
    );
}

EnergyJ FitnessEvaluator::evaluate_energy_consumption(const Chromosome& chromosome) const {
    auto strategy = chromosome_to_strategy(chromosome);
    auto task_distances = get_default_task_distances();
    
    return energy_calc_.calculate_total_energy(
        workflow_, strategy, edge_nodes_, bandwidth_, task_distances
    );
}

double FitnessEvaluator::evaluate_success_rate(const Chromosome& chromosome) const {
    auto strategy = chromosome_to_strategy(chromosome);
    auto task_distances = get_default_task_distances();
    
    // Calculate timing
    auto task_times = time_calc_.calculate_all_task_times(
        workflow_, strategy, edge_nodes_, bandwidth_, task_distances
    );
    
    // Calculate deadlines
    auto task_deadlines = success_calc_.calculate_qos_deadlines(
        workflow_, task_times, deadline_factor_
    );
    
    // Calculate success rate
    return success_calc_.calculate_success_rate(
        workflow_, strategy, task_times, task_deadlines
    );
}

// ========================================================================
// BATCH EVALUATION
// ========================================================================

void FitnessEvaluator::evaluate_population(std::vector<Chromosome>& chromosomes) const {
    for (auto& chromosome : chromosomes) {
        evaluate(chromosome);
    }
}

bool FitnessEvaluator::is_fitness_valid(const Chromosome& chromosome) const {
    return chromosome.completion_time().has_value() &&
           chromosome.energy_consumption().has_value() &&
           chromosome.success_rate().has_value();
}

// ========================================================================
// HELPER METHODS
// ========================================================================

SchedulingStrategy FitnessEvaluator::chromosome_to_strategy(const Chromosome& chromosome) const {
    // Create strategy with assignments based on chromosome
    std::map<TaskId, std::optional<EdgeNodeId>> assignments;
    
    for (std::size_t task_idx = 0; task_idx < chromosome.size(); ++task_idx) {
        TaskId task_id = static_cast<TaskId>(task_idx);
        std::uint32_t assignment = chromosome.get_assignment(task_idx);
        
        if (chromosome.is_assigned_to_datacenter(task_idx)) {
            assignments[task_id] = std::nullopt;
        } else {
            assignments[task_id] = static_cast<EdgeNodeId>(assignment);
        }
    }
    
    return SchedulingStrategy(
        workflow_.id(),
        assignments
    );
}

std::map<TaskId, DistanceKm> FitnessEvaluator::get_default_task_distances() const {
    // Default: all tasks 1 km away (simplified network model)
    std::map<TaskId, DistanceKm> distances;
    for (TaskId task_id : workflow_.get_all_task_ids()) {
        distances[task_id] = 1.0;
    }
    return distances;
}

}  // namespace sdn
