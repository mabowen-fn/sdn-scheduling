#include "sdn/chromosome.hpp"
#include "sdn/scheduling_strategy.hpp"
#include "sdn/task.hpp"
#include <algorithm>

namespace sdn {

// ============================================================================
// CONSTRUCTORS
// ============================================================================

Chromosome::Chromosome(std::size_t num_tasks, std::uint32_t num_edge_nodes)
    : genome_(num_tasks, 0), num_edge_nodes_(num_edge_nodes) {
    if (num_tasks == 0) {
        throw InvalidScheduleException("Chromosome must have at least one task");
    }
}

Chromosome::Chromosome(const std::vector<std::uint32_t>& genome, std::uint32_t num_edge_nodes)
    : genome_(genome), num_edge_nodes_(num_edge_nodes) {
    if (genome.empty()) {
        throw InvalidScheduleException("Chromosome genome cannot be empty");
    }
    validate();
}

Chromosome::Chromosome(const SchedulingStrategy& strategy, std::uint32_t num_edge_nodes)
    : genome_(strategy.task_count()), num_edge_nodes_(num_edge_nodes) {
    
    // Encode strategy into genome
    std::size_t task_idx = 0;
    for (const auto& [task_id, edge_node_id] : strategy.get_assignments()) {
        if (edge_node_id.has_value()) {
            genome_[task_idx] = edge_node_id.value();
        } else {
            genome_[task_idx] = num_edge_nodes_;  // Data center marker
        }
        task_idx++;
    }
}

// ============================================================================
// ACCESSORS
// ============================================================================

std::uint32_t Chromosome::get_assignment(std::size_t task_idx) const {
    if (task_idx >= genome_.size()) {
        throw InvalidScheduleException("Task index out of bounds");
    }
    return genome_[task_idx];
}

bool Chromosome::is_assigned_to_edge(std::size_t task_idx) const {
    if (task_idx >= genome_.size()) {
        throw InvalidScheduleException("Task index out of bounds");
    }
    return genome_[task_idx] < num_edge_nodes_;
}

bool Chromosome::is_assigned_to_datacenter(std::size_t task_idx) const {
    if (task_idx >= genome_.size()) {
        throw InvalidScheduleException("Task index out of bounds");
    }
    return genome_[task_idx] == num_edge_nodes_;
}

// ============================================================================
// MODIFICATIONS
// ============================================================================

void Chromosome::set_assignment(std::size_t task_idx, std::uint32_t edge_node_id) {
    if (task_idx >= genome_.size()) {
        throw InvalidScheduleException("Task index out of bounds");
    }
    if (edge_node_id > num_edge_nodes_) {
        throw InvalidScheduleException("Edge node ID out of bounds");
    }
    genome_[task_idx] = edge_node_id;
}

void Chromosome::assign_to_edge_node(std::size_t task_idx, std::uint32_t edge_node_id) {
    if (edge_node_id >= num_edge_nodes_) {
        throw InvalidScheduleException("Edge node ID out of bounds");
    }
    set_assignment(task_idx, edge_node_id);
}

void Chromosome::assign_to_datacenter(std::size_t task_idx) {
    set_assignment(task_idx, num_edge_nodes_);
}

// ============================================================================
// CONVERSION
// ============================================================================

SchedulingStrategy Chromosome::to_strategy(WorkflowId workflow_id) const {
    std::map<TaskId, std::optional<EdgeNodeId>> assignments;
    
    for (std::size_t task_idx = 0; task_idx < genome_.size(); ++task_idx) {
        TaskId task_id = static_cast<TaskId>(task_idx);
        std::uint32_t assignment = genome_[task_idx];
        
        if (assignment == num_edge_nodes_) {
            assignments[task_id] = std::nullopt;  // Data center
        } else {
            assignments[task_id] = static_cast<EdgeNodeId>(assignment);
        }
    }
    
    return SchedulingStrategy(workflow_id, assignments);
}

// ============================================================================
// VALIDATION
// ============================================================================

void Chromosome::validate() const {
    if (genome_.empty()) {
        throw InvalidScheduleException("Chromosome genome cannot be empty");
    }
    
    for (std::size_t i = 0; i < genome_.size(); ++i) {
        if (genome_[i] > num_edge_nodes_) {
            throw InvalidScheduleException(
                "Chromosome assignment at position " + std::to_string(i) + " is out of bounds"
            );
        }
    }
}

}  // namespace sdn
