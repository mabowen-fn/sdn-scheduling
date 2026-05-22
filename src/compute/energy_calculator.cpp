#include "sdn/energy_calculator.hpp"
#include "sdn/workflow.hpp"
#include "sdn/edge_node.hpp"
#include "sdn/task.hpp"
#include <algorithm>
#include <cmath>

namespace sdn {

// ============================================================================
// TOTAL ENERGY CALCULATION
// ============================================================================

EnergyJ EnergyCalculator::calculate_total_energy(
    const Workflow& workflow,
    const SchedulingStrategy& strategy,
    const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes,
    BandwidthMbps bandwidth,
    const std::map<TaskId, DistanceKm>& task_distance) const
{
    EnergyJ total = 0.0;
    
    try {
        total += calculate_transmission_energy(workflow, strategy, bandwidth, task_distance);
        total += calculate_execution_energy(workflow, strategy, edge_nodes);
        total += calculate_synchronization_energy(workflow, strategy);
        
        return total;
    } catch (const std::exception& e) {
        throw CalculationException(std::string("Total energy calculation failed: ") + e.what());
    }
}

// ============================================================================
// TRANSMISSION ENERGY
// ============================================================================

EnergyJ EnergyCalculator::calculate_transmission_energy(
    const Workflow& workflow,
    const SchedulingStrategy& strategy,
    BandwidthMbps bandwidth,
    const std::map<TaskId, DistanceKm>& task_distance) const
{
    if (bandwidth == 0) {
        throw CalculationException("Bandwidth cannot be zero");
    }
    
    EnergyJ transmission_energy = 0.0;
    
    // For each task in the strategy
    for (const auto& [task_id, edge_node_id] : strategy.get_assignments()) {
        // Only count transmission if assigned to edge node
        if (edge_node_id.has_value()) {
            const Task* task = workflow.get_task(task_id);
            if (!task) {
                continue;  // Skip if task not found
            }
            
            // Get distance (default to 1 km if not specified)
            DistanceKm distance = 1.0;
            auto dist_it = task_distance.find(task_id);
            if (dist_it != task_distance.end()) {
                distance = dist_it->second;
            }
            
            transmission_energy += calculate_transmission_cost(
                task->size(),
                bandwidth,
                distance
            );
        }
    }
    
    return transmission_energy;
}

EnergyJ EnergyCalculator::calculate_transmission_cost(
    TaskSize task_size,
    BandwidthMbps bandwidth,
    DistanceKm distance) const
{
    if (bandwidth == 0) {
        throw CalculationException("Bandwidth cannot be zero");
    }
    
    // Convert task size from bytes to MB
    double task_size_mb = task_size * BYTES_TO_MB_FACTOR;
    
    // Transmission time in milliseconds
    // time = (size_MB) / (bandwidth_Mbps) = seconds, convert to ms
    double transmission_time_ms = (task_size_mb / bandwidth) * 1000.0;
    
    // Energy based on transmission time and distance
    // Simplified model: energy = transmission_time * distance * transmission_power
    double energy_kw_ms = TRANSMISSION_POWER_FACTOR * distance;
    
    // Convert to Joules: KW*ms = J/s * ms = J * (ms/1000)
    EnergyJ energy_joules = energy_kw_ms * transmission_time_ms / 1000.0;
    
    return energy_joules;
}

// ============================================================================
// EXECUTION ENERGY
// ============================================================================

EnergyJ EnergyCalculator::calculate_execution_energy(
    const Workflow& workflow,
    const SchedulingStrategy& strategy,
    const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes) const
{
    EnergyJ execution_energy = 0.0;
    
    for (const auto& [task_id, edge_node_id] : strategy.get_assignments()) {
        const Task* task = workflow.get_task(task_id);
        if (!task) {
            continue;
        }
        
        if (edge_node_id.has_value()) {
            // Task assigned to edge node
            auto node_it = edge_nodes.find(edge_node_id.value());
            if (node_it != edge_nodes.end() && node_it->second) {
                execution_energy += calculate_task_edge_energy(*task, *node_it->second);
            }
        } else {
            // Task assigned to data center
            execution_energy += calculate_task_datacenter_energy(*task);
        }
    }
    
    return execution_energy;
}

EnergyJ EnergyCalculator::calculate_task_edge_energy(
    const Task& task,
    const EdgeNode& edge_node) const
{
    // Use EdgeNode's execution energy calculation
    return edge_node.calculate_execution_energy(task.size());
}

EnergyJ EnergyCalculator::calculate_task_datacenter_energy(
    const Task& task,
    double datacenter_power_rate) const
{
    // Simplified data center energy model
    // Energy = task_size * power_rate
    double task_size_mb = task.size() * BYTES_TO_MB_FACTOR;
    return task_size_mb * datacenter_power_rate;
}

// ============================================================================
// SYNCHRONIZATION ENERGY
// ============================================================================

EnergyJ EnergyCalculator::calculate_synchronization_energy(
    const Workflow& /* workflow */,
    const SchedulingStrategy& strategy,
    std::uint32_t num_controllers) const
{
    // Simplified synchronization energy model
    // Based on number of edge node assignments
    
    std::size_t edge_assignments = 0;
    for (const auto& [task_id, edge_node_id] : strategy.get_assignments()) {
        if (edge_node_id.has_value()) {
            edge_assignments++;
        }
    }
    
    // Energy proportional to number of edge assignments and controllers
    // Simplified: sync_energy = num_controllers * num_edge_assignments * sync_power_per_task
    double sync_power_per_task = 0.01;  // KW per task synchronization
    
    // Convert to Joules (assuming 1ms per synchronization event)
    EnergyJ sync_energy = num_controllers * edge_assignments * sync_power_per_task / 1000.0;
    
    return sync_energy;
}

}  // namespace sdn
