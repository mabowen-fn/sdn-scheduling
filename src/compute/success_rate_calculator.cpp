#include "sdn/success_rate_calculator.hpp"
#include "sdn/workflow.hpp"
#include "sdn/task.hpp"
#include "sdn/critical_path_analyzer.hpp"
#include <algorithm>
#include <cmath>

namespace sdn {

// ============================================================================
// SUCCESS RATE CALCULATION
// ============================================================================

double SuccessRateCalculator::calculate_success_rate(
    const Workflow& workflow,
    const SchedulingStrategy& /* strategy */,
    const std::map<TaskId, TimeMs>& task_times,
    const std::map<TaskId, TimeMs>& task_deadlines) const
{
    std::size_t total_tasks = workflow.task_count();
    if (total_tasks == 0) {
        return 0.0;
    }
    
    std::size_t successful_tasks = count_tasks_meeting_deadline(task_times, task_deadlines);
    
    return static_cast<double>(successful_tasks) / static_cast<double>(total_tasks);
}

std::map<TaskId, double> SuccessRateCalculator::calculate_per_task_success(
    const Workflow& /* workflow */,
    const std::map<TaskId, TimeMs>& task_times,
    const std::map<TaskId, TimeMs>& task_deadlines) const
{
    std::map<TaskId, double> success_map;
    
    for (const auto& [task_id, completion_time] : task_times) {
        auto deadline_it = task_deadlines.find(task_id);
        TimeMs deadline = (deadline_it != task_deadlines.end()) ? deadline_it->second : completion_time;
        
        success_map[task_id] = meets_deadline(completion_time, deadline) ? 1.0 : 0.0;
    }
    
    return success_map;
}

// ============================================================================
// DEADLINE CALCULATION AND ASSIGNMENT
// ============================================================================

std::map<TaskId, TimeMs> SuccessRateCalculator::calculate_qos_deadlines(
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times,
    double deadline_factor) const
{
    validate_deadline_factor(deadline_factor);
    
    CriticalPathAnalyzer analyzer;
    return analyzer.calculate_all_deadlines(workflow, task_times, deadline_factor);
}

void SuccessRateCalculator::validate_deadline_factor(double deadline_factor) const
{
    if (deadline_factor < MINIMUM_DEADLINE_FACTOR) {
        throw CalculationException(
            "Deadline factor must be >= " + std::to_string(MINIMUM_DEADLINE_FACTOR)
        );
    }
}

// ============================================================================
// UNCERTAINTY HANDLING
// ============================================================================

double SuccessRateCalculator::calculate_success_rate_with_uncertainty(
    const Workflow& workflow,
    const std::map<TaskId, TimeMs>& task_times,
    const std::map<TaskId, TimeMs>& task_deadlines,
    double uncertainty_factor) const
{
    if (uncertainty_factor < 0.0 || uncertainty_factor > 1.0) {
        throw CalculationException("Uncertainty factor must be in [0, 1]");
    }
    
    std::size_t total_tasks = workflow.task_count();
    if (total_tasks == 0) {
        return 0.0;
    }
    
    std::size_t successful_tasks = 0;
    
    for (const auto& [task_id, completion_time] : task_times) {
        auto deadline_it = task_deadlines.find(task_id);
        if (deadline_it == task_deadlines.end()) {
            continue;
        }
        
        TimeMs deadline = deadline_it->second;
        
        // Worst case: actual time could be higher by uncertainty_factor
        TimeMs worst_case_time = static_cast<TimeMs>(
            completion_time * (1.0 + uncertainty_factor)
        );
        
        // Task succeeds if even worst case meets deadline
        if (worst_case_time <= deadline) {
            successful_tasks++;
        }
    }
    
    return static_cast<double>(successful_tasks) / static_cast<double>(total_tasks);
}

double SuccessRateCalculator::evaluate_robustness(
    const Workflow& workflow,
    const SchedulingStrategy& strategy,
    const std::map<TaskId, TimeMs>& task_times,
    const std::map<TaskId, TimeMs>& task_deadlines,
    const std::vector<double>& uncertainty_levels) const
{
    if (uncertainty_levels.empty()) {
        return calculate_success_rate(workflow, strategy, task_times, task_deadlines);
    }
    
    double total_success = 0.0;
    
    for (double uncertainty : uncertainty_levels) {
        total_success += calculate_success_rate_with_uncertainty(
            workflow, task_times, task_deadlines, uncertainty
        );
    }
    
    return total_success / static_cast<double>(uncertainty_levels.size());
}

// ============================================================================
// STATISTICS
// ============================================================================

std::size_t SuccessRateCalculator::count_tasks_meeting_deadline(
    const std::map<TaskId, TimeMs>& task_times,
    const std::map<TaskId, TimeMs>& task_deadlines) const
{
    std::size_t count = 0;
    
    for (const auto& [task_id, completion_time] : task_times) {
        auto deadline_it = task_deadlines.find(task_id);
        if (deadline_it != task_deadlines.end()) {
            if (meets_deadline(completion_time, deadline_it->second)) {
                count++;
            }
        }
    }
    
    return count;
}

std::size_t SuccessRateCalculator::count_tasks_missing_deadline(
    const std::map<TaskId, TimeMs>& task_times,
    const std::map<TaskId, TimeMs>& task_deadlines) const
{
    return task_times.size() - count_tasks_meeting_deadline(task_times, task_deadlines);
}

SuccessRateCalculator::DeadlineStatistics 
SuccessRateCalculator::calculate_deadline_statistics(
    const std::map<TaskId, TimeMs>& task_times,
    const std::map<TaskId, TimeMs>& task_deadlines) const
{
    DeadlineStatistics stats;
    stats.num_violations = 0;
    stats.max_violation_ms = 0;
    double total_violation = 0.0;
    
    for (const auto& [task_id, completion_time] : task_times) {
        auto deadline_it = task_deadlines.find(task_id);
        if (deadline_it == task_deadlines.end()) {
            continue;
        }
        
        TimeMs deadline = deadline_it->second;
        if (completion_time > deadline) {
            TimeMs violation = completion_time - deadline;
            stats.num_violations++;
            stats.max_violation_ms = std::max(stats.max_violation_ms, violation);
            total_violation += static_cast<double>(violation);
        }
    }
    
    if (stats.num_violations > 0) {
        stats.avg_violation_ms = total_violation / static_cast<double>(stats.num_violations);
    }
    
    return stats;
}

}  // namespace sdn
