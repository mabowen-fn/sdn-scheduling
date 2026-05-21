#include "sdn/task.hpp"
#include <sstream>
#include <stdexcept>

namespace sdn {

// ============================================================================
// CONSTRUCTORS
// ============================================================================

Task::Task(TaskId id, TaskSize size, TimeMs deadline)
    : id_(id), size_(size), deadline_(deadline) {
    if (size == 0) {
        throw InvalidWorkflowException("Task size must be greater than 0");
    }
    if (deadline == 0) {
        throw InvalidWorkflowException("Task deadline must be greater than 0");
    }
}

// ============================================================================
// DEPENDENCY MANAGEMENT
// ============================================================================

void Task::add_predecessor(TaskId predecessor_id) {
    if (predecessor_id == id_) {
        throw InvalidWorkflowException("Task cannot be its own predecessor");
    }
    
    auto [it, inserted] = predecessors_.insert(predecessor_id);
    if (!inserted) {
        std::ostringstream oss;
        oss << "Task " << predecessor_id << " is already a predecessor of task " << id_;
        throw InvalidWorkflowException(oss.str());
    }
}

void Task::add_successor(TaskId successor_id) {
    if (successor_id == id_) {
        throw InvalidWorkflowException("Task cannot be its own successor");
    }
    
    auto [it, inserted] = successors_.insert(successor_id);
    if (!inserted) {
        std::ostringstream oss;
        oss << "Task " << successor_id << " is already a successor of task " << id_;
        throw InvalidWorkflowException(oss.str());
    }
}

void Task::remove_predecessor(TaskId predecessor_id) {
    predecessors_.erase(predecessor_id);
}

void Task::remove_successor(TaskId successor_id) {
    successors_.erase(successor_id);
}

void Task::clear_predecessors() {
    predecessors_.clear();
}

void Task::clear_successors() {
    successors_.clear();
}

}  // namespace sdn
