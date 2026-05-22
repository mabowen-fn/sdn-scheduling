#include "sdn/time_calculator.hpp"
#include "sdn/workflow.hpp"
#include "sdn/edge_node.hpp"
#include "sdn/task.hpp"
#include <algorithm>
#include <queue>
#include <set>

namespace sdn {

// ============================================================================
// WORKFLOW COMPLETION TIME
// ============================================================================

TimeMs TimeCalculator::calculate_workflow_completion_time(
    const Workflow& workflow,
    const SchedulingStrategy& strategy,
    const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes,
    BandwidthMbps bandwidth,
    const std::map<TaskId, DistanceKm>& task_distance) const
{
    if (workflow.task_count() == 0) {
        return 0;
    }
    
    // Calculate all task completion times
    auto all_task_times = calculate_all_task_times(
        workflow, strategy, edge_nodes, bandwidth, task_distance
    );
    
    if (all_task_times.empty()) {
        return 0;
    }
    
    // Workflow completion time = max(completion time of all exit tasks)
    TimeMs max_completion = 0;
    for (const auto& exit_task_id : workflow.get_exit_tasks()) {
        auto it = all_task_times.find(exit_task_id);
        if (it != all_task_times.end()) {
            max_completion = std::max(max_completion, it->second);
        }
    }
    
    return max_completion;
}

// ============================================================================
// SINGLE TASK COMPLETION TIME
// ============================================================================

TimeMs TimeCalculator::calculate_task_completion_time(
    const Task& task,
    const std::optional<const EdgeNode*>& edge_node,
    const std::map<TaskId, TimeMs>& task_completion_times,
    const Workflow& workflow,
    BandwidthMbps bandwidth,
    DistanceKm distance) const
{
    // Start time = max(completion times of predecessors)
    TimeMs start_time = calculate_start_time(task, task_completion_times, workflow);
    
    TimeMs execution_time = 0;
    TimeMs transmission_time = 0;
    
    if (edge_node.has_value() && edge_node.value() != nullptr) {
        // Task assigned to edge node
        execution_time = calculate_execution_time(task.size(), *edge_node.value());
        transmission_time = calculate_transmission_time(task.size(), bandwidth, distance);
    } else {
        // Task assigned to data center
        execution_time = calculate_datacenter_execution_time(task.size());
        transmission_time = 0;  // Assume no transmission delay for data center
    }
    
    return start_time + execution_time + transmission_time;
}

// ============================================================================
// TIME COMPONENTS
// ============================================================================

TimeMs TimeCalculator::calculate_transmission_time(
    TaskSize task_size,
    BandwidthMbps bandwidth,
    DistanceKm distance) const
{
    if (bandwidth == 0) {
        throw CalculationException("Bandwidth cannot be zero");
    }
    
    // Simplified transmission time model
    // Assume transmission speed = bandwidth
    // Time = size / bandwidth (in ms, converting bytes and Mbps)
    
    double task_size_mb = task_size * BYTES_TO_MB_FACTOR;
    double transmission_time_seconds = task_size_mb / bandwidth;
    TimeMs transmission_time_ms = static_cast<TimeMs>(transmission_time_seconds * 1000.0);
    
    // Account for distance (propagation delay)
    TimeMs propagation_delay_ms = static_cast<TimeMs>(distance);  // Simplified: 1ms per km
    
    return transmission_time_ms + propagation_delay_ms;
}

TimeMs TimeCalculator::calculate_execution_time(
    TaskSize task_size,
    const EdgeNode& edge_node) const
{
    return edge_node.calculate_execution_time(task_size);
}

TimeMs TimeCalculator::calculate_start_time(
    const Task& task,
    const std::map<TaskId, TimeMs>& task_completion_times,
    const Workflow& /* workflow */) const
{
    if (task.predecessors().empty()) {
        return 0;
    }
    
    // Start time = max(completion times of all predecessors)
    TimeMs max_predecessor_completion = 0;
    for (TaskId pred_id : task.predecessors()) {
        auto it = task_completion_times.find(pred_id);
        if (it != task_completion_times.end()) {
            max_predecessor_completion = std::max(max_predecessor_completion, it->second);
        }
    }
    
    return max_predecessor_completion;
}

TimeMs TimeCalculator::calculate_datacenter_execution_time(
    TaskSize task_size,
    ProcessingPowerMhz datacenter_processing_power) const
{
    if (datacenter_processing_power == 0) {
        throw CalculationException("Data center processing power cannot be zero");
    }
    
    // Execution time = task_size / processing_power
    // task_size is in bytes, processing_power is in MHz
    double task_size_mb = task_size * BYTES_TO_MB_FACTOR;
    double execution_time_seconds = task_size_mb / datacenter_processing_power;
    
    return static_cast<TimeMs>(execution_time_seconds * 1000.0);
}

// ============================================================================
// TOPOLOGICAL PROCESSING
// ============================================================================

std::vector<TaskId> TimeCalculator::get_topological_order(const Workflow& workflow) const
{
    std::vector<TaskId> result;
    
    if (workflow.task_count() == 0) {
        return result;
    }
    
    std::map<TaskId, std::size_t> in_degree;
    std::map<TaskId, std::vector<TaskId>> adjacency;
    
    // Initialize
    for (TaskId task_id : workflow.get_all_task_ids()) {
        const Task* task = workflow.get_task(task_id);
        if (task) {
            in_degree[task_id] = task->predecessors().size();
            for (TaskId succ : task->successors()) {
                adjacency[task_id].push_back(succ);
            }
        }
    }
    
    // Find all tasks with no predecessors
    std::queue<TaskId> queue;
    for (TaskId task_id : workflow.get_all_task_ids()) {
        if (in_degree[task_id] == 0) {
            queue.push(task_id);
        }
    }
    
    // Process tasks in topological order
    while (!queue.empty()) {
        TaskId current = queue.front();
        queue.pop();
        result.push_back(current);
        
        // Process successors
        for (TaskId successor : adjacency[current]) {
            in_degree[successor]--;
            if (in_degree[successor] == 0) {
                queue.push(successor);
            }
        }
    }
    
    return result;
}

// ============================================================================
// ALL TASK TIMES
// ============================================================================

std::map<TaskId, TimeMs> TimeCalculator::calculate_all_task_times(
    const Workflow& workflow,
    const SchedulingStrategy& strategy,
    const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes,
    BandwidthMbps bandwidth,
    const std::map<TaskId, DistanceKm>& task_distance) const
{
    std::map<TaskId, TimeMs> task_times;
    
    // Get topological order
    auto topo_order = get_topological_order(workflow);
    
    // Process tasks in topological order
    for (TaskId task_id : topo_order) {
        const Task* task = workflow.get_task(task_id);
        if (!task) {
            continue;
        }
        
        // Get assignment for this task
        auto assignment = strategy.get_assignment(task_id);
        std::optional<const EdgeNode*> edge_node = std::nullopt;
        
        if (assignment.has_value()) {
            auto node_it = edge_nodes.find(assignment.value());
            if (node_it != edge_nodes.end()) {
                edge_node = node_it->second;
            }
        }
        
        // Get distance
        DistanceKm distance = 1.0;
        auto dist_it = task_distance.find(task_id);
        if (dist_it != task_distance.end()) {
            distance = dist_it->second;
        }
        
        // Calculate completion time
        task_times[task_id] = calculate_task_completion_time(
            *task,
            edge_node,
            task_times,
            workflow,
            bandwidth,
            distance
        );
    }
    
    return task_times;
}

}  // namespace sdn
