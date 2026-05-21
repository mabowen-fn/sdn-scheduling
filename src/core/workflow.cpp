#include "sdn/workflow.hpp"
#include <algorithm>
#include <queue>
#include <set>
#include <sstream>

namespace sdn {

// ============================================================================
// CONSTRUCTORS
// ============================================================================

Workflow::Workflow(WorkflowId id)
    : id_(id) {
}

// ============================================================================
// TASK MANAGEMENT
// ============================================================================

void Workflow::add_task(Task task) {
    TaskId task_id = task.id();
    
    if (tasks_.count(task_id) > 0) {
        std::ostringstream oss;
        oss << "Task " << task_id << " already exists in workflow";
        throw InvalidWorkflowException(oss.str());
    }
    
    tasks_.emplace(task_id, std::move(task));
}

Task* Workflow::get_task(TaskId task_id) {
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return nullptr;
    }
    return &it->second;
}

const Task* Workflow::get_task(TaskId task_id) const {
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<TaskId> Workflow::get_all_task_ids() const {
    std::vector<TaskId> ids;
    ids.reserve(tasks_.size());
    for (const auto& [id, _] : tasks_) {
        ids.push_back(id);
    }
    return ids;
}

bool Workflow::has_task(TaskId task_id) const {
    return tasks_.count(task_id) > 0;
}

// ============================================================================
// DEPENDENCY MANAGEMENT
// ============================================================================

void Workflow::add_dependency(TaskId predecessor_id, TaskId successor_id) {
    auto* pred_task = get_task(predecessor_id);
    auto* succ_task = get_task(successor_id);
    
    if (!pred_task) {
        std::ostringstream oss;
        oss << "Predecessor task " << predecessor_id << " not found";
        throw InvalidWorkflowException(oss.str());
    }
    
    if (!succ_task) {
        std::ostringstream oss;
        oss << "Successor task " << successor_id << " not found";
        throw InvalidWorkflowException(oss.str());
    }
    
    // Check for cycles: would adding this edge create a cycle?
    if (has_path(successor_id, predecessor_id)) {
        throw InvalidWorkflowException(
            "Adding dependency would create a cycle in the workflow");
    }
    
    // Add the dependency
    pred_task->add_successor(successor_id);
    succ_task->add_predecessor(predecessor_id);
}

void Workflow::remove_dependency(TaskId predecessor_id, TaskId successor_id) {
    auto* pred_task = get_task(predecessor_id);
    auto* succ_task = get_task(successor_id);
    
    if (!pred_task || !succ_task) {
        throw InvalidWorkflowException("Task not found in workflow");
    }
    
    pred_task->remove_successor(successor_id);
    succ_task->remove_predecessor(predecessor_id);
}

bool Workflow::has_path(TaskId source, TaskId target) const {
    if (source == target) {
        return true;
    }
    
    // BFS to find path from source to target
    std::queue<TaskId> queue;
    std::set<TaskId> visited;
    
    queue.push(source);
    visited.insert(source);
    
    while (!queue.empty()) {
        TaskId current = queue.front();
        queue.pop();
        
        const Task* task = get_task(current);
        if (!task) {
            continue;
        }
        
        for (TaskId successor : task->successors()) {
            if (successor == target) {
                return true;
            }
            
            if (visited.find(successor) == visited.end()) {
                visited.insert(successor);
                queue.push(successor);
            }
        }
    }
    
    return false;
}

bool Workflow::is_valid_dag() const {
    // Check for cycles using DFS
    std::set<TaskId> all_tasks_set;
    for (const auto& [id, _] : tasks_) {
        all_tasks_set.insert(id);
    }
    
    std::set<TaskId> visited;
    std::set<TaskId> rec_stack;
    
    std::function<bool(TaskId)> has_cycle = [&](TaskId task_id) -> bool {
        visited.insert(task_id);
        rec_stack.insert(task_id);
        
        const Task* task = get_task(task_id);
        if (!task) {
            return false;
        }
        
        for (TaskId succ : task->successors()) {
            if (visited.find(succ) == visited.end()) {
                if (has_cycle(succ)) {
                    return true;
                }
            } else if (rec_stack.find(succ) != rec_stack.end()) {
                return true;  // Back edge found - cycle exists
            }
        }
        
        rec_stack.erase(task_id);
        return false;
    };
    
    for (TaskId task_id : all_tasks_set) {
        if (visited.find(task_id) == visited.end()) {
            if (has_cycle(task_id)) {
                return false;
            }
        }
    }
    
    return true;
}

std::vector<TaskId> Workflow::get_entry_tasks() const {
    std::vector<TaskId> entries;
    
    for (const auto& [task_id, task] : tasks_) {
        if (!task.has_predecessors()) {
            entries.push_back(task_id);
        }
    }
    
    return entries;
}

std::vector<TaskId> Workflow::get_exit_tasks() const {
    std::vector<TaskId> exits;
    
    for (const auto& [task_id, task] : tasks_) {
        if (!task.has_successors()) {
            exits.push_back(task_id);
        }
    }
    
    return exits;
}

// ============================================================================
// QUERY OPERATIONS
// ============================================================================

TimeMs Workflow::get_critical_path_length() const {
    if (tasks_.empty()) {
        return 0;
    }
    
    // Compute longest path using dynamic programming
    std::map<TaskId, TimeMs> longest_path;
    
    // Initialize all tasks
    for (const auto& [task_id, task] : tasks_) {
        longest_path[task_id] = task.deadline();
    }
    
    // Topological sort and dynamic programming
    // This is simplified - actual critical path needs task execution times
    TimeMs max_deadline = 0;
    for (const auto& [task_id, deadline] : longest_path) {
        max_deadline = std::max(max_deadline, deadline);
    }
    
    return max_deadline;
}

std::vector<TaskId> Workflow::get_critical_path() const {
    std::vector<TaskId> path;
    
    // For now, return all tasks (actual implementation requires full critical path analysis)
    for (const auto& [task_id, _] : tasks_) {
        path.push_back(task_id);
    }
    
    return path;
}

TaskSize Workflow::get_total_size() const {
    TaskSize total = 0;
    
    for (const auto& [_, task] : tasks_) {
        total += task.size();
    }
    
    return total;
}

// ============================================================================
// VALIDATION
// ============================================================================

void Workflow::validate() const {
    if (tasks_.empty()) {
        throw InvalidWorkflowException("Workflow must contain at least one task");
    }
    
    if (!is_valid_dag()) {
        throw InvalidWorkflowException("Workflow contains cycles");
    }
}

}  // namespace sdn
