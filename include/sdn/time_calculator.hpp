#pragma once

#include "types.hpp"
#include "workflow.hpp"
#include "edge_node.hpp"
#include "scheduling_strategy.hpp"
#include <map>
#include <optional>
#include <vector>

namespace sdn {

// ============================================================================
// TIME CALCULATOR CLASS
// ============================================================================

/**
 * Calculates completion time for a scheduling strategy.
 * 
 * Completion time consists of:
 * 1. OT - Transmission time (network communication delay)
 * 2. ET - Execution time (computation on edge nodes)
 * 3. ST - Start time (considering task dependencies)
 * 4. CT - Completion time (start + execution + transmission)
 * 
 * Total workflow completion time = max(CT) of all exit tasks
 */
class TimeCalculator {
public:
    /**
     * Construct a time calculator.
     */
    TimeCalculator() = default;
    
    ~TimeCalculator() = default;
    
    // Delete copy/move (stateless utility class)
    TimeCalculator(const TimeCalculator&) = delete;
    TimeCalculator& operator=(const TimeCalculator&) = delete;
    TimeCalculator(TimeCalculator&&) = delete;
    TimeCalculator& operator=(TimeCalculator&&) = delete;
    
    // ========================================================================
    // COMPLETION TIME CALCULATION
    // ========================================================================
    
    /**
     * Calculate total workflow completion time for a scheduling strategy.
     * 
     * Considers task dependencies, transmission delays, and execution times.
     * 
     * @param workflow The workflow being scheduled
     * @param strategy The scheduling strategy (task to edge node assignments)
     * @param edge_nodes Map of edge node ID to EdgeNode (for processing power)
     * @param bandwidth Network bandwidth in Mbps
     * @param task_distance Map of task ID to distance in km
     * @return Total completion time in milliseconds
     * @throws CalculationException if calculation fails
     */
    TimeMs calculate_workflow_completion_time(
        const Workflow& workflow,
        const SchedulingStrategy& strategy,
        const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes,
        BandwidthMbps bandwidth,
        const std::map<TaskId, DistanceKm>& task_distance) const;
    
    /**
     * Calculate completion time for a single task.
     * 
     * Completion time = start_time + execution_time + transmission_time
     * where start_time = max(predecessor completion times)
     * 
     * @param task The task
     * @param edge_node The assigned edge node (nullopt for data center)
     * @param task_completion_times Map of completed task times (for dependencies)
     * @param workflow The workflow (to access dependencies)
     * @param bandwidth Network bandwidth in Mbps
     * @param distance Distance to edge node in km
     * @return Completion time in milliseconds
     * @throws CalculationException if calculation fails
     */
    TimeMs calculate_task_completion_time(
        const Task& task,
        const std::optional<const EdgeNode*>& edge_node,
        const std::map<TaskId, TimeMs>& task_completion_times,
        const Workflow& workflow,
        BandwidthMbps bandwidth,
        DistanceKm distance) const;
    
    // ========================================================================
    // TIME COMPONENT CALCULATIONS
    // ========================================================================
    
    /**
     * Calculate transmission time for a task.
     * 
     * Transmission time = task_size / bandwidth * distance_km
     * Simplified: assumes transmission proportional to data size and distance
     * 
     * @param task_size Task size in bytes
     * @param bandwidth Bandwidth in Mbps
     * @param distance Distance in kilometers
     * @return Transmission time in milliseconds
     */
    TimeMs calculate_transmission_time(
        TaskSize task_size,
        BandwidthMbps bandwidth,
        DistanceKm distance) const;
    
    /**
     * Calculate execution time for a task on an edge node.
     * 
     * @param task_size Task size in bytes
     * @param edge_node The edge node (has processing power)
     * @return Execution time in milliseconds
     * @throws CalculationException if edge_node is nullptr
     */
    TimeMs calculate_execution_time(
        TaskSize task_size,
        const EdgeNode& edge_node) const;
    
    /**
     * Calculate start time for a task considering dependencies.
     * 
     * Start time = max(completion time of all predecessors)
     * For tasks with no predecessors, start time = 0
     * 
     * @param task The task
     * @param task_completion_times Map of already-completed task times
     * @param workflow The workflow (to access dependencies)
     * @return Start time in milliseconds
     */
    TimeMs calculate_start_time(
        const Task& task,
        const std::map<TaskId, TimeMs>& task_completion_times,
        const Workflow& workflow) const;
    
    /**
     * Calculate data center execution time.
     * 
     * Data center typically has much higher processing power.
     * Simplified model: execution_time = task_size / datacenter_processing_power
     * 
     * @param task_size Task size in bytes
     * @param datacenter_processing_power Processing power in MHz (typically very high)
     * @return Execution time in milliseconds
     */
    TimeMs calculate_datacenter_execution_time(
        TaskSize task_size,
        ProcessingPowerMhz datacenter_processing_power = 100000) const;
    
    // ========================================================================
    // TOPOLOGICAL PROCESSING
    // ========================================================================
    
    /**
     * Get tasks in topological order (process dependencies first).
     * 
     * Uses breadth-first search from entry tasks.
     * 
     * @param workflow The workflow
     * @return Vector of task IDs in valid topological order
     */
    std::vector<TaskId> get_topological_order(const Workflow& workflow) const;
    
    /**
     * Build complete task timing information.
     * 
     * Calculates start time, execution time, and completion time for each task.
     * 
     * @param workflow The workflow
     * @param strategy The scheduling strategy
     * @param edge_nodes Map of edge node ID to EdgeNode
     * @param bandwidth Network bandwidth
     * @param task_distance Map of task distances
     * @return Map of task ID to completion time
     * @throws CalculationException if calculation fails
     */
    std::map<TaskId, TimeMs> calculate_all_task_times(
        const Workflow& workflow,
        const SchedulingStrategy& strategy,
        const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes,
        BandwidthMbps bandwidth,
        const std::map<TaskId, DistanceKm>& task_distance) const;

private:
    // Conversion constants
    static constexpr double BYTES_TO_MB_FACTOR = 1.0 / (1024.0 * 1024.0);
    static constexpr double BYTES_TO_MBPS_MS_FACTOR = 8.0 / 1024.0;  // 8 bits per byte, convert to ms
};

}  // namespace sdn
