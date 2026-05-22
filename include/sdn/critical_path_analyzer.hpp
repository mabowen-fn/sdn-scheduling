#pragma once

#include "types.hpp"
#include "workflow.hpp"
#include "edge_node.hpp"
#include "scheduling_strategy.hpp"
#include <map>
#include <vector>
#include <optional>

namespace sdn {

// ============================================================================
// CRITICAL PATH ANALYZER CLASS
// ============================================================================

/**
 * Analyzes the critical path in a workflow DAG.
 * 
 * The critical path is the longest dependency chain from entry to exit tasks.
 * It determines the minimum possible completion time for the entire workflow.
 * 
 * Used to:
 * 1. Calculate task-level deadlines based on QoS requirements
 * 2. Identify scheduling bottlenecks
 * 3. Determine the workflow's theoretical minimum completion time
 */
class CriticalPathAnalyzer {
public:
    /**
     * Construct a critical path analyzer.
     */
    CriticalPathAnalyzer() = default;
    
    ~CriticalPathAnalyzer() = default;
    
    // Delete copy/move (stateless utility class)
    CriticalPathAnalyzer(const CriticalPathAnalyzer&) = delete;
    CriticalPathAnalyzer& operator=(const CriticalPathAnalyzer&) = delete;
    CriticalPathAnalyzer(CriticalPathAnalyzer&&) = delete;
    CriticalPathAnalyzer& operator=(CriticalPathAnalyzer&&) = delete;
    
    // ========================================================================
    // CRITICAL PATH ANALYSIS
    // ========================================================================
    
    /**
     * Analyze the workflow to determine critical path metrics.
     * 
     * Computes for each task:
     * - Earliest start time
     * - Earliest finish time
     * - Latest start time
     * - Latest finish time
     * - Slack (flexibility in scheduling)
     * 
     * @param workflow The workflow to analyze
     * @param task_times Map of task ID to estimated execution time (in ms)
     * @throws CalculationException if workflow is invalid
     */
    void analyze_workflow(
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times) const;
    
    /**
     * Get the longest path length in the workflow.
     * 
     * This is the theoretical minimum completion time.
     * 
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @return Critical path length in milliseconds
     * @throws CalculationException if workflow is invalid
     */
    TimeMs get_critical_path_length(
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times) const;
    
    /**
     * Get all tasks on the critical path.
     * 
     * These are the tasks that directly impact overall completion time.
     * Any delay in a critical path task delays the entire workflow.
     * 
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @return Vector of task IDs on critical path
     * @throws CalculationException if workflow is invalid
     */
    std::vector<TaskId> get_critical_path_tasks(
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times) const;
    
    // ========================================================================
    // DEADLINE CALCULATION (QoS)
    // ========================================================================
    
    /**
     * Calculate QoS deadline for a task based on critical path.
     * 
     * From the paper:
     * DT(Vm) = α * WT(optimal)  where α >= 1 is the deadline factor
     * 
     * Per-task deadline is proportional to its position on the critical path.
     * Tasks on the critical path get stricter deadlines.
     * 
     * @param task The task
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @param deadline_factor Multiplier for QoS (default 1.5, must be >= 1.0)
     * @return QoS deadline in milliseconds
     * @throws CalculationException if parameters invalid
     */
    TimeMs calculate_task_deadline(
        const Task& task,
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times,
        double deadline_factor = 1.5) const;
    
    /**
     * Calculate deadlines for all tasks.
     * 
     * Propagates deadlines from workflow completion time down to individual tasks,
     * with stricter deadlines for critical path tasks.
     * 
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @param deadline_factor QoS deadline multiplier
     * @return Map of task ID to calculated deadline
     * @throws CalculationException if calculation fails
     */
    std::map<TaskId, TimeMs> calculate_all_deadlines(
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times,
        double deadline_factor = 1.5) const;
    
    // ========================================================================
    // SLACK AND FLEXIBILITY
    // ========================================================================
    
    /**
     * Calculate slack for a task.
     * 
     * Slack = latest_start_time - earliest_start_time
     * 
     * Tasks with zero slack are on the critical path (non-negotiable schedule).
     * Tasks with positive slack have flexibility in scheduling.
     * 
     * @param task The task
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @return Slack in milliseconds
     */
    TimeMs calculate_task_slack(
        const Task& task,
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times) const;
    
    /**
     * Check if a task is on the critical path.
     * 
     * Critical path tasks have zero slack.
     * 
     * @param task The task
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @return true if task has zero slack
     */
    bool is_critical_task(
        const Task& task,
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times) const {
        return calculate_task_slack(task, workflow, task_times) == 0;
    }
    
    // ========================================================================
    // EARLIEST/LATEST TIME CALCULATIONS
    // ========================================================================
    
    /**
     * Calculate earliest start time for a task.
     * 
     * Earliest start = max(earliest finish of all predecessors)
     * 
     * @param task The task
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @return Earliest start time in milliseconds
     */
    TimeMs calculate_earliest_start_time(
        const Task& task,
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times) const;
    
    /**
     * Calculate earliest finish time for a task.
     * 
     * Earliest finish = earliest start + execution time
     * 
     * @param task The task
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @return Earliest finish time in milliseconds
     */
    TimeMs calculate_earliest_finish_time(
        const Task& task,
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times) const;
    
    /**
     * Calculate latest finish time for a task.
     * 
     * Latest finish = min(latest start of all successors)
     * 
     * @param task The task
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @param workflow_deadline Total workflow deadline (from critical path)
     * @return Latest finish time in milliseconds
     */
    TimeMs calculate_latest_finish_time(
        const Task& task,
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times,
        TimeMs /* workflow_deadline */) const;
    
    /**
     * Calculate latest start time for a task.
     * 
     * Latest start = latest finish - execution time
     * 
     * @param task The task
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @param workflow_deadline Total workflow deadline
     * @return Latest start time in milliseconds
     */
    TimeMs calculate_latest_start_time(
        const Task& task,
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times,
        TimeMs workflow_deadline) const;

private:
    // Cache for computed values (mutable to allow computation in const methods)
    mutable std::map<TaskId, TimeMs> earliest_start_times_;
    mutable std::map<TaskId, TimeMs> earliest_finish_times_;
    mutable std::map<TaskId, TimeMs> latest_start_times_;
    mutable std::map<TaskId, TimeMs> latest_finish_times_;
    mutable bool cache_valid_ = false;
};

}  // namespace sdn
