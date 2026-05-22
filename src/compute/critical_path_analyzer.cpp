#include "sdn/critical_path_analyzer.hpp"
#include "sdn/workflow.hpp"
#include "sdn/task.hpp"
#include <algorithm>
#include <queue>
#include <limits>

namespace sdn {

// ============================================================================
// CRITICAL PATH ANALYSIS
// ============================================================================

void CriticalPathAnalyzer::analyze_workflow(
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times) const
{
    // Reset cache
    earliest_start_times_.clear();
    earliest_finish_times_.clear();
    latest_start_times_.clear();
    latest_finish_times_.clear();
    
    if (workflow.task_count() == 0) {
        cache_valid_ = true;
        return;
    }
    
    // Forward pass: calculate earliest times
    for (TaskId task_id : workflow.get_all_task_ids()) {
        const Task* task = workflow.get_task(task_id);
        if (!task) continue;
        
        auto time_it = task_times.find(task_id);
        TimeMs exec_time = (time_it != task_times.end()) ? time_it->second : 0;
        
        // Earliest start = max(earliest finish of predecessors)
        TimeMs est = 0;
        for (TaskId pred : task->predecessors()) {
            auto pred_eft_it = earliest_finish_times_.find(pred);
            if (pred_eft_it != earliest_finish_times_.end()) {
                est = std::max(est, pred_eft_it->second);
            }
        }
        
        earliest_start_times_[task_id] = est;
        earliest_finish_times_[task_id] = est + exec_time;
    }
    
    // Backward pass: calculate latest times
    TimeMs workflow_deadline = 0;
    for (const auto& [task_id, eft] : earliest_finish_times_) {
        workflow_deadline = std::max(workflow_deadline, eft);
    }
    
    for (TaskId task_id : workflow.get_all_task_ids()) {
        const Task* task = workflow.get_task(task_id);
        if (!task) continue;
        
        auto time_it = task_times.find(task_id);
        TimeMs exec_time = (time_it != task_times.end()) ? time_it->second : 0;
        
        // Latest finish = min(latest start of successors)
        TimeMs lft = workflow_deadline;
        if (!task->successors().empty()) {
            lft = std::numeric_limits<TimeMs>::max();
            for (TaskId succ : task->successors()) {
                auto succ_lst_it = latest_start_times_.find(succ);
                if (succ_lst_it != latest_start_times_.end()) {
                    lft = std::min(lft, succ_lst_it->second);
                }
            }
        }
        
        latest_finish_times_[task_id] = lft;
        latest_start_times_[task_id] = (lft > exec_time) ? (lft - exec_time) : 0;
    }
    
    cache_valid_ = true;
}

TimeMs CriticalPathAnalyzer::get_critical_path_length(
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times) const
{
    TimeMs cp_length = 0;
    
    for (const auto& [task_id, exec_time] : task_times) {
        cp_length = std::max(cp_length, exec_time);
    }
    
    // Actually, we need to compute critical path by considering dependencies
    // Simple approach: use workflow's critical path calculation
    return workflow.get_critical_path_length();
}

std::vector<TaskId> CriticalPathAnalyzer::get_critical_path_tasks(
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times) const
{
    std::vector<TaskId> critical_tasks;
    
    for (TaskId task_id : workflow.get_all_task_ids()) {
        if (is_critical_task(*workflow.get_task(task_id), workflow, task_times)) {
            critical_tasks.push_back(task_id);
        }
    }
    
    return critical_tasks;
}

// ============================================================================
// DEADLINE CALCULATION
// ============================================================================

TimeMs CriticalPathAnalyzer::calculate_task_deadline(
    const Task& task,
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times,
    double deadline_factor) const
{
    if (deadline_factor < 1.0) {
        throw CalculationException("Deadline factor must be >= 1.0");
    }
    
    // Get critical path length
    TimeMs cp_length = workflow.get_critical_path_length();
    
    // Workflow deadline
    TimeMs workflow_deadline = static_cast<TimeMs>(cp_length * deadline_factor);
    
    // Task deadline proportional to its position
    auto task_time_it = task_times.find(task.id());
    TimeMs task_time = (task_time_it != task_times.end()) ? task_time_it->second : 0;
    
    // Simple proportional assignment
    TimeMs task_deadline = static_cast<TimeMs>(
        (static_cast<double>(task_time) / cp_length) * workflow_deadline
    );
    
    return std::max(task_deadline, task_time);  // Deadline >= execution time
}

std::map<TaskId, TimeMs> CriticalPathAnalyzer::calculate_all_deadlines(
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times,
    double deadline_factor) const
{
    std::map<TaskId, TimeMs> deadlines;
    
    for (TaskId task_id : workflow.get_all_task_ids()) {
        const Task* task = workflow.get_task(task_id);
        if (task) {
            deadlines[task_id] = calculate_task_deadline(*task, workflow, task_times, deadline_factor);
        }
    }
    
    return deadlines;
}

// ============================================================================
// SLACK AND FLEXIBILITY
// ============================================================================

TimeMs CriticalPathAnalyzer::calculate_task_slack(
    const Task& task,
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times) const
{
    if (!cache_valid_) {
        analyze_workflow(workflow, task_times);
    }
    
    auto est_it = earliest_start_times_.find(task.id());
    auto lst_it = latest_start_times_.find(task.id());
    
    if (est_it == earliest_start_times_.end() || lst_it == latest_start_times_.end()) {
        return 0;
    }
    
    TimeMs slack = lst_it->second - est_it->second;
    return (slack > 0) ? slack : 0;
}

// ============================================================================
// EARLIEST/LATEST TIME CALCULATIONS
// ============================================================================

TimeMs CriticalPathAnalyzer::calculate_earliest_start_time(
    const Task& task,
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times) const
{
    if (!cache_valid_) {
        analyze_workflow(workflow, task_times);
    }
    
    auto it = earliest_start_times_.find(task.id());
    return (it != earliest_start_times_.end()) ? it->second : 0;
}

TimeMs CriticalPathAnalyzer::calculate_earliest_finish_time(
    const Task& task,
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times) const
{
    if (!cache_valid_) {
        analyze_workflow(workflow, task_times);
    }
    
    auto it = earliest_finish_times_.find(task.id());
    return (it != earliest_finish_times_.end()) ? it->second : 0;
}

TimeMs CriticalPathAnalyzer::calculate_latest_finish_time(
    const Task& task,
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times,
    TimeMs /* workflow_deadline */) const
{
    if (!cache_valid_) {
        analyze_workflow(workflow, task_times);
    }
    
    auto it = latest_finish_times_.find(task.id());
    if (it != latest_finish_times_.end()) {
        return it->second;
    }
    return 0;
}

TimeMs CriticalPathAnalyzer::calculate_latest_start_time(
    const Task& task,
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times,
    TimeMs /* workflow_deadline */) const
{
    if (!cache_valid_) {
        analyze_workflow(workflow, task_times);
    }
    
    auto it = latest_start_times_.find(task.id());
    return (it != latest_start_times_.end()) ? it->second : 0;
}

}  // namespace sdn
