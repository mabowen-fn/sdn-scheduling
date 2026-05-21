#include "sdn/scheduling_strategy.hpp"
#include <random>
#include <sstream>
#include <algorithm>

namespace sdn {

// ============================================================================
// CONSTRUCTORS
// ============================================================================

SchedulingStrategy::SchedulingStrategy(WorkflowId workflow_id, std::size_t num_tasks)
    : workflow_id_(workflow_id) {
    
    if (num_tasks == 0) {
        throw InvalidWorkflowException("SchedulingStrategy must have at least 1 task");
    }
    
    // Initialize all tasks as unassigned (will be set later)
    for (std::size_t i = 0; i < num_tasks; ++i) {
        assignments_[static_cast<TaskId>(i)] = std::nullopt;
    }
}

SchedulingStrategy::SchedulingStrategy(
    WorkflowId workflow_id,
    const std::map<TaskId, std::optional<EdgeNodeId>>& assignments)
    : workflow_id_(workflow_id), assignments_(assignments) {
    
    if (assignments.empty()) {
        throw InvalidWorkflowException("SchedulingStrategy must have at least 1 assignment");
    }
}

// ============================================================================
// ACCESSORS
// ============================================================================

std::optional<EdgeNodeId> SchedulingStrategy::get_assignment(TaskId task_id) const {
    auto it = assignments_.find(task_id);
    if (it == assignments_.end()) {
        std::ostringstream oss;
        oss << "Task " << task_id << " not in scheduling strategy";
        throw InvalidScheduleException(oss.str());
    }
    
    return it->second;
}

// ============================================================================
// MODIFICATIONS
// ============================================================================

void SchedulingStrategy::set_assignment(TaskId task_id,
                                       std::optional<EdgeNodeId> edge_node_id) {
    auto it = assignments_.find(task_id);
    if (it == assignments_.end()) {
        std::ostringstream oss;
        oss << "Task " << task_id << " not in scheduling strategy";
        throw InvalidScheduleException(oss.str());
    }
    
    it->second = edge_node_id;
    
    // Clear cached metrics since assignment changed
    clear_metrics();
}

void SchedulingStrategy::randomize_assignments(std::uint32_t num_edge_nodes,
                                              std::uint32_t seed) {
    std::mt19937 rng(seed == 0 ? std::random_device{}() : seed);
    
    // Distribution: 70% edge nodes, 30% data center
    std::uniform_real_distribution<double> location_dist(0.0, 1.0);
    std::uniform_int_distribution<std::uint32_t> edge_dist(0, num_edge_nodes - 1);
    
    for (auto& [task_id, assignment] : assignments_) {
        double location_rand = location_dist(rng);
        
        if (location_rand < 0.7 && num_edge_nodes > 0) {
            // Assign to edge node
            EdgeNodeId edge_id = edge_dist(rng);
            assignment = edge_id;
        } else {
            // Assign to data center
            assignment = std::nullopt;
        }
    }
    
    // Clear cached metrics
    clear_metrics();
}

// ============================================================================
// SUCCESS RATE
// ============================================================================

void SchedulingStrategy::set_success_rate(double rate) {
    if (rate < 0.0 || rate > 1.0) {
        throw InvalidScheduleException("Success rate must be in [0, 1]");
    }
    
    success_rate_ = rate;
}

// ============================================================================
// COMPARISON
// ============================================================================

bool SchedulingStrategy::operator==(const SchedulingStrategy& other) const {
    if (workflow_id_ != other.workflow_id_) {
        return false;
    }
    
    if (assignments_.size() != other.assignments_.size()) {
        return false;
    }
    
    for (const auto& [task_id, assignment] : assignments_) {
        auto other_it = other.assignments_.find(task_id);
        if (other_it == other.assignments_.end()) {
            return false;
        }
        
        if (assignment != other_it->second) {
            return false;
        }
    }
    
    return true;
}

bool SchedulingStrategy::operator!=(const SchedulingStrategy& other) const {
    return !(*this == other);
}

// ============================================================================
// VALIDATION
// ============================================================================

void SchedulingStrategy::validate() const {
    // All tasks must exist in assignments_ and can have any value (nullopt is valid)
    if (assignments_.empty()) {
        throw InvalidScheduleException("SchedulingStrategy must have at least one assignment");
    }
}

}  // namespace sdn
