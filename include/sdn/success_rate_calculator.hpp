#pragma once

#include "types.hpp"
#include "workflow.hpp"
#include "edge_node.hpp"
#include "scheduling_strategy.hpp"
#include <map>
#include <optional>
#include <vector>

namespace sdn {

// Forward declaration
class CriticalPathAnalyzer;
class TimeCalculator;

// ============================================================================
// SUCCESS RATE CALCULATOR CLASS
// ============================================================================

/**
 * Calculates success rate for a scheduling strategy based on QoS requirements.
 * 
 * Success rate evaluates whether tasks meet their deadlines despite uncertainties.
 * 
 * From the paper:
 * - Per-task success: k(task) = 1 if actual_time < deadline, 0 otherwise
 * - Overall success rate: SUC(Vm) = (1/T) * sum of k(task)
 * 
 * Where:
 * - Deadline is calculated from critical path: DT(Vm) = α * WT(optimal)
 * - α is the deadline factor (typically 1.5, must be >= 1.0)
 */
class SuccessRateCalculator {
public:
    /**
     * Construct a success rate calculator.
     */
    SuccessRateCalculator() = default;
    
    ~SuccessRateCalculator() = default;
    
    // Delete copy/move (stateless utility class)
    SuccessRateCalculator(const SuccessRateCalculator&) = delete;
    SuccessRateCalculator& operator=(const SuccessRateCalculator&) = delete;
    SuccessRateCalculator(SuccessRateCalculator&&) = delete;
    SuccessRateCalculator& operator=(SuccessRateCalculator&&) = delete;
    
    // ========================================================================
    // SUCCESS RATE CALCULATION
    // ========================================================================
    
    /**
     * Calculate overall success rate for a scheduling strategy.
     * 
     * Success rate = (number of tasks meeting deadline) / (total tasks)
     * 
     * @param workflow The workflow being scheduled
     * @param strategy The scheduling strategy
     * @param task_times Map of task ID to actual completion time
     * @param task_deadlines Map of task ID to QoS deadline
     * @return Success rate in range [0.0, 1.0]
     * @throws CalculationException if calculation fails
     */
    double calculate_success_rate(
        const Workflow& workflow,
        const SchedulingStrategy& strategy,
        const std::map<TaskId, TimeMs>& task_times,
        const std::map<TaskId, TimeMs>& task_deadlines) const;
    
    /**
     * Calculate per-task success indicators.
     * 
     * @param workflow The workflow
     * @param task_times Map of task ID to completion time
     * @param task_deadlines Map of task ID to deadline
     * @return Map of task ID to success (1.0 = met deadline, 0.0 = missed)
     * @throws CalculationException if calculation fails
     */
    std::map<TaskId, double> calculate_per_task_success(
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times,
        const std::map<TaskId, TimeMs>& task_deadlines) const;
    
    /**
     * Check if a single task meets its deadline.
     * 
     * @param completion_time Actual completion time in ms
     * @param deadline Deadline in ms
     * @return true if completion_time <= deadline
     */
    bool meets_deadline(TimeMs completion_time, TimeMs deadline) const {
        return completion_time <= deadline;
    }
    
    // ========================================================================
    // DEADLINE CALCULATION AND ASSIGNMENT
    // ========================================================================
    
    /**
     * Calculate QoS deadline for all tasks based on critical path.
     * 
     * Algorithm:
     * 1. Calculate critical path length (minimum workflow completion time)
     * 2. Apply deadline factor: adjusted_deadline = critical_path * deadline_factor
     * 3. Assign per-task deadlines proportionally to critical path
     * 
     * @param workflow The workflow
     * @param task_times Map of task ID to execution time
     * @param deadline_factor QoS multiplier (default 1.5, must be >= 1.0)
     * @return Map of task ID to deadline
     * @throws CalculationException if deadline_factor < 1.0
     */
    std::map<TaskId, TimeMs> calculate_qos_deadlines(
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times,
        double deadline_factor = 1.5) const;
    
    /**
     * Validate deadline factor.
     * 
     * @param deadline_factor Value to validate
     * @throws CalculationException if deadline_factor < 1.0
     */
    void validate_deadline_factor(double deadline_factor) const;
    
    // ========================================================================
    // SUCCESS METRICS UNDER UNCERTAINTY
    // ========================================================================
    
    /**
     * Calculate success rate accounting for execution time uncertainty.
     * 
     * Models scenarios where actual execution time may differ from estimates.
     * Computes probability of meeting deadline under uncertainty assumptions.
     * 
     * Simplified model: accounts for max execution time variance of ±10%
     * 
     * @param workflow The workflow
     * @param task_times Map of task ID to estimated completion time
     * @param task_deadlines Map of task ID to deadline
     * @param uncertainty_factor Percentage uncertainty (default 0.1 = 10%)
     * @return Success rate accounting for uncertainty
     */
    double calculate_success_rate_with_uncertainty(
        const Workflow& workflow,
        const std::map<TaskId, TimeMs>& task_times,
        const std::map<TaskId, TimeMs>& task_deadlines,
        double uncertainty_factor = 0.1) const;
    
    /**
     * Evaluate robustness of a scheduling strategy.
     * 
     * Robustness = how well the strategy performs under various uncertainty scenarios.
     * Computed as the average success rate across multiple uncertainty levels.
     * 
     * @param workflow The workflow
     * @param strategy The scheduling strategy
     * @param task_times Map of task ID to completion time
     * @param task_deadlines Map of task ID to deadline
     * @param uncertainty_levels Vector of uncertainty factors to test
     * @return Average success rate across uncertainty levels
     */
    double evaluate_robustness(
        const Workflow& workflow,
        const SchedulingStrategy& strategy,
        const std::map<TaskId, TimeMs>& task_times,
        const std::map<TaskId, TimeMs>& task_deadlines,
        const std::vector<double>& uncertainty_levels) const;
    
    // ========================================================================
    // STATISTICS
    // ========================================================================
    
    /**
     * Get number of tasks meeting deadline.
     * 
     * @param task_times Map of task ID to completion time
     * @param task_deadlines Map of task ID to deadline
     * @return Number of tasks with completion_time <= deadline
     */
    std::size_t count_tasks_meeting_deadline(
        const std::map<TaskId, TimeMs>& task_times,
        const std::map<TaskId, TimeMs>& task_deadlines) const;
    
    /**
     * Get number of tasks missing deadline.
     * 
     * @param task_times Map of task ID to completion time
     * @param task_deadlines Map of task ID to deadline
     * @return Number of tasks with completion_time > deadline
     */
    std::size_t count_tasks_missing_deadline(
        const std::map<TaskId, TimeMs>& task_times,
        const std::map<TaskId, TimeMs>& task_deadlines) const;
    
    /**
     * Calculate statistics about deadline violations.
     * 
     * @param task_times Map of task ID to completion time
     * @param task_deadlines Map of task ID to deadline
     * @return Tuple: (num_missed, max_violation_ms, avg_violation_ms)
     */
    struct DeadlineStatistics {
        std::size_t num_violations = 0;
        TimeMs max_violation_ms = 0;
        double avg_violation_ms = 0.0;
    };
    
    DeadlineStatistics calculate_deadline_statistics(
        const std::map<TaskId, TimeMs>& task_times,
        const std::map<TaskId, TimeMs>& task_deadlines) const;

private:
    static constexpr double MINIMUM_DEADLINE_FACTOR = 1.0;
};

}  // namespace sdn
