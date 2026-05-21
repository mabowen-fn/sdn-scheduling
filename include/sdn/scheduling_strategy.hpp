#pragma once

#include "types.hpp"
#include "workflow.hpp"
#include "edge_node.hpp"
#include <map>
#include <optional>
#include <vector>

namespace sdn {

// ============================================================================
// SCHEDULING STRATEGY CLASS
// ============================================================================

/**
 * Represents a scheduling decision for a workflow.
 * 
 * A scheduling strategy assigns each task to either:
 * - A specific edge node (for execution on edge)
 * - Null/Data Center (for execution on remote data center)
 * 
 * The strategy also caches computed metrics (completion time, energy).
 */
class SchedulingStrategy {
public:
    // ========================================================================
    // CONSTRUCTORS & LIFECYCLE
    // ========================================================================
    
    /**
     * Construct an empty scheduling strategy for a workflow.
     * 
     * @param workflow_id ID of the workflow
     * @param num_tasks Number of tasks in workflow
     * @throws InvalidWorkflowException if num_tasks is zero
     */
    explicit SchedulingStrategy(WorkflowId workflow_id, std::size_t num_tasks);
    
    /**
     * Construct a scheduling strategy with initial assignments.
     * 
     * @param workflow_id ID of the workflow
     * @param assignments Map of task_id -> edge_node_id (nullopt for data center)
     */
    SchedulingStrategy(WorkflowId workflow_id,
                      const std::map<TaskId, std::optional<EdgeNodeId>>& assignments);
    
    // Delete default constructor
    SchedulingStrategy() = delete;
    
    // Allow copy and move
    SchedulingStrategy(const SchedulingStrategy&) = default;
    SchedulingStrategy& operator=(const SchedulingStrategy&) = default;
    SchedulingStrategy(SchedulingStrategy&&) = default;
    SchedulingStrategy& operator=(SchedulingStrategy&&) = default;
    
    ~SchedulingStrategy() = default;
    
    // ========================================================================
    // ACCESSORS
    // ========================================================================
    
    /// Get workflow ID
    WorkflowId workflow_id() const { return workflow_id_; }
    
    /// Get number of tasks
    std::size_t task_count() const { return assignments_.size(); }
    
    /**
     * Get the assigned edge node for a task.
     * 
     * @param task_id ID of the task
     * @return Edge node ID if assigned to edge, nullopt if assigned to data center
     * @throws InvalidScheduleException if task not in strategy
     */
    std::optional<EdgeNodeId> get_assignment(TaskId task_id) const;
    
    /**
     * Get all assignments.
     * 
     * @return Map of task_id -> assigned edge_node_id (nullopt for data center)
     */
    const std::map<TaskId, std::optional<EdgeNodeId>>& get_assignments() const {
        return assignments_;
    }
    
    // ========================================================================
    // MODIFICATIONS
    // ========================================================================
    
    /**
     * Set assignment for a task.
     * 
     * @param task_id ID of the task
     * @param edge_node_id Edge node ID (nullopt for data center)
     * @throws InvalidScheduleException if task not in strategy
     */
    void set_assignment(TaskId task_id, std::optional<EdgeNodeId> edge_node_id);
    
    /**
     * Assign a task to an edge node.
     * 
     * @param task_id ID of the task
     * @param edge_node_id ID of edge node
     * @throws InvalidScheduleException if task not in strategy
     */
    void assign_to_edge_node(TaskId task_id, EdgeNodeId edge_node_id) {
        set_assignment(task_id, edge_node_id);
    }
    
    /**
     * Assign a task to data center.
     * 
     * @param task_id ID of the task
     * @throws InvalidScheduleException if task not in strategy
     */
    void assign_to_data_center(TaskId task_id) {
        set_assignment(task_id, std::nullopt);
    }
    
    /**
     * Randomly assign all tasks to edge nodes or data center.
     * 
     * @param num_edge_nodes Total number of edge nodes available
     * @param seed Random seed (0 = use random device)
     */
    void randomize_assignments(std::uint32_t num_edge_nodes, std::uint32_t seed = 0);
    
    // ========================================================================
    // METRICS (COMPUTED ON DEMAND)
    // ========================================================================
    
    /**
     * Get completion time for this strategy.
     * 
     * Must be set after calculation.
     * 
     * @return Completion time in ms, or nullopt if not yet calculated
     */
    std::optional<TimeMs> completion_time() const { return completion_time_; }
    
    /**
     * Set completion time for this strategy.
     * 
     * @param time Completion time in ms
     */
    void set_completion_time(TimeMs time) { completion_time_ = time; }
    
    /**
     * Get energy consumption for this strategy.
     * 
     * Must be set after calculation.
     * 
     * @return Energy in Joules, or nullopt if not yet calculated
     */
    std::optional<EnergyJ> energy_consumption() const { return energy_consumption_; }
    
    /**
     * Set energy consumption for this strategy.
     * 
     * @param energy Energy in Joules
     */
    void set_energy_consumption(EnergyJ energy) { energy_consumption_ = energy; }
    
    /**
     * Get success rate for this strategy.
     * 
     * Must be set after calculation.
     * 
     * @return Success rate [0, 1], or nullopt if not yet calculated
     */
    std::optional<double> success_rate() const { return success_rate_; }
    
    /**
     * Set success rate for this strategy.
     * 
     * @param rate Success rate [0, 1]
     * @throws InvalidScheduleException if rate not in [0, 1]
     */
    void set_success_rate(double rate);
    
    /**
     * Clear all computed metrics.
     */
    void clear_metrics() {
        completion_time_ = std::nullopt;
        energy_consumption_ = std::nullopt;
        success_rate_ = std::nullopt;
    }
    
    // ========================================================================
    // COMPARISON
    // ========================================================================
    
    /// Check if two strategies are identical
    bool operator==(const SchedulingStrategy& other) const;
    
    /// Check if two strategies differ
    bool operator!=(const SchedulingStrategy& other) const;
    
    // ========================================================================
    // VALIDATION
    // ========================================================================
    
    /**
     * Validate that all tasks have assignments.
     * 
     * @throws InvalidScheduleException if any task is unassigned
     */
    void validate() const;
    
private:
    WorkflowId workflow_id_;
    std::map<TaskId, std::optional<EdgeNodeId>> assignments_;
    
    // Computed metrics (lazy evaluation)
    mutable std::optional<TimeMs> completion_time_;
    mutable std::optional<EnergyJ> energy_consumption_;
    mutable std::optional<double> success_rate_;
};

}  // namespace sdn
