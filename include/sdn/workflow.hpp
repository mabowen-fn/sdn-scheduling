#pragma once

#include "types.hpp"
#include "task.hpp"
#include "edge_node.hpp"
#include <map>
#include <memory>
#include <vector>

namespace sdn {

// ============================================================================
// WORKFLOW CLASS
// ============================================================================

/**
 * Represents a workflow of dependent computing tasks.
 * 
 * A workflow is a DAG (Directed Acyclic Graph) of tasks where:
 * - Tasks have dependencies (predecessors/successors)
 * - All tasks must complete for workflow to complete
 * - Completion time is determined by critical path
 */
class Workflow {
public:
    // ========================================================================
    // CONSTRUCTORS & LIFECYCLE
    // ========================================================================
    
    /**
     * Construct a workflow.
     * 
     * @param id Unique workflow ID
     * @throws InvalidWorkflowException if ID is invalid
     */
    explicit Workflow(WorkflowId id);
    
    // Delete default constructor
    Workflow() = delete;
    
    // Delete copy and move (workflows are usually unique/large)
    Workflow(const Workflow&) = delete;
    Workflow& operator=(const Workflow&) = delete;
    Workflow(Workflow&&) = default;
    Workflow& operator=(Workflow&&) = default;
    
    ~Workflow() = default;
    
    // ========================================================================
    // ACCESSORS - IDENTITY
    // ========================================================================
    
    /// Get workflow ID
    WorkflowId id() const { return id_; }
    
    // ========================================================================
    // TASK MANAGEMENT
    // ========================================================================
    
    /**
     * Add a task to the workflow.
     * 
     * @param task Task to add (by move)
     * @throws InvalidWorkflowException if task ID already exists
     */
    void add_task(Task task);
    
    /**
     * Get a task by ID.
     * 
     * @param task_id ID of the task
     * @return Pointer to task, or nullptr if not found
     */
    Task* get_task(TaskId task_id);
    const Task* get_task(TaskId task_id) const;
    
    /**
     * Get all tasks.
     * 
     * @return Vector of all task IDs in order
     */
    std::vector<TaskId> get_all_task_ids() const;
    
    /**
     * Get number of tasks.
     */
    std::size_t task_count() const { return tasks_.size(); }
    
    /**
     * Check if workflow has a task.
     * 
     * @param task_id ID of the task
     * @return true if task exists in workflow
     */
    bool has_task(TaskId task_id) const;
    
    // ========================================================================
    // DEPENDENCY MANAGEMENT
    // ========================================================================
    
    /**
     * Add a dependency between two tasks.
     * 
     * @param predecessor_id ID of predecessor task
     * @param successor_id ID of successor task
     * @throws InvalidWorkflowException if:
     *   - Either task doesn't exist
     *   - Dependency already exists
     *   - Would create a cycle
     */
    void add_dependency(TaskId predecessor_id, TaskId successor_id);
    
    /**
     * Remove a dependency between two tasks.
     * 
     * @param predecessor_id ID of predecessor task
     * @param successor_id ID of successor task
     * @throws InvalidWorkflowException if dependency doesn't exist
     */
    void remove_dependency(TaskId predecessor_id, TaskId successor_id);
    
    /**
     * Check if workflow is a valid DAG (no cycles).
     * 
     * @return true if workflow is acyclic
     */
    bool is_valid_dag() const;
    
    /**
     * Get entry tasks (tasks with no predecessors).
     * 
     * @return Vector of entry task IDs
     */
    std::vector<TaskId> get_entry_tasks() const;
    
    /**
     * Get exit tasks (tasks with no successors).
     * 
     * @return Vector of exit task IDs
     */
    std::vector<TaskId> get_exit_tasks() const;
    
    // ========================================================================
    // QUERY OPERATIONS
    // ========================================================================
    
    /**
     * Get critical path length (longest dependency chain).
     * 
     * @return Length of critical path in milliseconds
     */
    TimeMs get_critical_path_length() const;
    
    /**
     * Get all tasks on the critical path.
     * 
     * @return Vector of task IDs on critical path
     */
    std::vector<TaskId> get_critical_path() const;
    
    /**
     * Get total workflow size (sum of all task sizes).
     * 
     * @return Total size in bytes
     */
    TaskSize get_total_size() const;
    
    // ========================================================================
    // VALIDATION
    // ========================================================================
    
    /**
     * Validate the workflow.
     * 
     * Checks:
     * - At least one task exists
     * - All dependencies are valid
     * - DAG is acyclic
     * 
     * @throws InvalidWorkflowException if validation fails
     */
    void validate() const;
    
private:
    WorkflowId id_;
    std::map<TaskId, Task> tasks_;  // Keep tasks in insertion order
    
    /**
     * Check if there's a path from source to target (for cycle detection).
     * Used internally by add_dependency.
     */
    bool has_path(TaskId source, TaskId target) const;
};

}  // namespace sdn
