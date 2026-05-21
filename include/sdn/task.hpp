#pragma once

#include "types.hpp"
#include <vector>
#include <set>

namespace sdn {

// ============================================================================
// TASK CLASS
// ============================================================================

/**
 * Represents a computing task in a workflow.
 * 
 * A task has:
 * - Unique ID within its workflow
 * - Size (data to process)
 * - Deadline for completion
 * - Dependencies on other tasks (predecessors)
 * - Optional distance to edge node (set during scheduling)
 */
class Task {
public:
    // ========================================================================
    // CONSTRUCTORS & LIFECYCLE
    // ========================================================================
    
    /**
     * Construct a task with required parameters.
     * 
     * @param id Unique task ID within workflow
     * @param size Size of task in bytes
     * @param deadline Deadline for completion in ms
     * @throws InvalidWorkflowException if parameters are invalid
     */
    Task(TaskId id, TaskSize size, TimeMs deadline);
    
    // Delete default constructor
    Task() = delete;
    
    // Allow move semantics
    Task(Task&&) = default;
    Task& operator=(Task&&) = default;
    
    // Copy constructor/assignment (explicit for clarity)
    Task(const Task&) = default;
    Task& operator=(const Task&) = default;
    
    ~Task() = default;
    
    // ========================================================================
    // ACCESSORS
    // ========================================================================
    
    /// Get task ID
    TaskId id() const { return id_; }
    
    /// Get task size in bytes
    TaskSize size() const { return size_; }
    
    /// Get task deadline in ms
    TimeMs deadline() const { return deadline_; }
    
    /// Get predecessor task IDs
    const std::set<TaskId>& predecessors() const { return predecessors_; }
    
    /// Get successor task IDs
    const std::set<TaskId>& successors() const { return successors_; }
    
    /// Check if task has predecessors
    bool has_predecessors() const { return !predecessors_.empty(); }
    
    /// Check if task has successors
    bool has_successors() const { return !successors_.empty(); }
    
    // ========================================================================
    // DEPENDENCY MANAGEMENT
    // ========================================================================
    
    /**
     * Add a predecessor task.
     * 
     * @param predecessor_id ID of the predecessor task
     * @throws InvalidWorkflowException if task already has this predecessor
     */
    void add_predecessor(TaskId predecessor_id);
    
    /**
     * Add a successor task.
     * 
     * @param successor_id ID of the successor task
     * @throws InvalidWorkflowException if task already has this successor
     */
    void add_successor(TaskId successor_id);
    
    /**
     * Remove a predecessor task.
     * 
     * @param predecessor_id ID of the predecessor task
     */
    void remove_predecessor(TaskId predecessor_id);
    
    /**
     * Remove a successor task.
     * 
     * @param successor_id ID of the successor task
     */
    void remove_successor(TaskId successor_id);
    
    /**
     * Clear all predecessors.
     */
    void clear_predecessors();
    
    /**
     * Clear all successors.
     */
    void clear_successors();
    
    // ========================================================================
    // RUNTIME STATE
    // ========================================================================
    
    /**
     * Set the actual completion time (recorded after execution).
     * 
     * @param completion_time Actual completion time in ms
     */
    void set_completion_time(TimeMs completion_time) {
        completion_time_ = completion_time;
    }
    
    /// Get actual completion time (if available)
    std::optional<TimeMs> completion_time() const { return completion_time_; }
    
    /**
     * Set the assigned edge node (after scheduling decision).
     * 
     * @param edge_node_id ID of the assigned edge node, or nullopt for data center
     */
    void set_assigned_edge_node(std::optional<EdgeNodeId> edge_node_id) {
        assigned_edge_node_id_ = edge_node_id;
    }
    
    /// Get assigned edge node (if scheduled)
    std::optional<EdgeNodeId> assigned_edge_node() const {
        return assigned_edge_node_id_;
    }
    
    // ========================================================================
    // COMPARISON & HASHING
    // ========================================================================
    
    /// Compare tasks by ID
    bool operator==(const Task& other) const { return id_ == other.id_; }
    bool operator<(const Task& other) const { return id_ < other.id_; }
    
private:
    TaskId id_;
    TaskSize size_;
    TimeMs deadline_;
    std::set<TaskId> predecessors_;
    std::set<TaskId> successors_;
    
    // Runtime state
    std::optional<TimeMs> completion_time_;
    std::optional<EdgeNodeId> assigned_edge_node_id_;
};

}  // namespace sdn
