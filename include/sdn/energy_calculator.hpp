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
// ENERGY CALCULATOR CLASS
// ============================================================================

/**
 * Calculates energy consumption for a scheduling strategy.
 * 
 * According to the paper, total energy consumption consists of three components:
 * 1. CMT - Transmission energy (network communication)
 * 2. CME - Execution energy (computation on edge nodes)
 * 3. CMS - Controller synchronization energy (control plane overhead)
 * 
 * Total: CM(Vm) = CMT + CME + CMS
 */
class EnergyCalculator {
public:
    /**
     * Construct an energy calculator.
     * 
     * Requires a configuration for network bandwidth and power rates.
     */
    EnergyCalculator() = default;
    
    ~EnergyCalculator() = default;
    
    // Delete copy/move (stateless utility class)
    EnergyCalculator(const EnergyCalculator&) = delete;
    EnergyCalculator& operator=(const EnergyCalculator&) = delete;
    EnergyCalculator(EnergyCalculator&&) = delete;
    EnergyCalculator& operator=(EnergyCalculator&&) = delete;
    
    // ========================================================================
    // ENERGY CALCULATION METHODS
    // ========================================================================
    
    /**
     * Calculate total energy consumption for a scheduling strategy.
     * 
     * @param workflow The workflow being scheduled
     * @param strategy The scheduling strategy (task to edge node assignments)
     * @param edge_nodes Map of edge node ID to EdgeNode (for energy rates)
     * @param bandwidth Network bandwidth in Mbps for transmission
     * @param task_distance Map of task ID to distance in km (for transmission)
     * @return Total energy consumption in Joules
     * @throws CalculationException if calculation fails
     */
    EnergyJ calculate_total_energy(
        const Workflow& workflow,
        const SchedulingStrategy& strategy,
        const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes,
        BandwidthMbps bandwidth,
        const std::map<TaskId, DistanceKm>& task_distance) const;
    
    /**
     * Calculate transmission energy only.
     * 
     * Transmission energy models network communication costs.
     * For each task assigned to edge, compute:
     *   ct(task, edge) = task_size / bandwidth * distance_km * power_factor
     * 
     * @param workflow The workflow being scheduled
     * @param strategy The scheduling strategy
     * @param bandwidth Network bandwidth in Mbps
     * @param task_distance Map of task ID to distance in km
     * @return Transmission energy in Joules
     * @throws CalculationException if calculation fails
     */
    EnergyJ calculate_transmission_energy(
        const Workflow& workflow,
        const SchedulingStrategy& strategy,
        BandwidthMbps bandwidth,
        const std::map<TaskId, DistanceKm>& task_distance) const;
    
    /**
     * Calculate execution energy only.
     * 
     * Execution energy models computation on edge nodes.
     * For each task assigned to edge node zk:
     *   ce(task, zk) = task_size * power_rate_operating
     * 
     * @param workflow The workflow being scheduled
     * @param strategy The scheduling strategy
     * @param edge_nodes Map of edge node ID to EdgeNode (for power rates)
     * @return Execution energy in Joules
     * @throws CalculationException if calculation fails
     */
    EnergyJ calculate_execution_energy(
        const Workflow& workflow,
        const SchedulingStrategy& strategy,
        const std::map<EdgeNodeId, const EdgeNode*>& edge_nodes) const;
    
    /**
     * Calculate controller synchronization energy.
     * 
     * Models overhead of SDN control plane communications.
     * Typically: CMS = sum of controller_sync_power * num_controller_pairs
     * 
     * For now, returns a simplified model based on number of edge assignments.
     * 
     * @param workflow The workflow being scheduled
     * @param strategy The scheduling strategy
     * @param num_controllers Number of SDN controllers in system (default: 1)
     * @return Controller synchronization energy in Joules
     * @throws CalculationException if calculation fails
     */
    EnergyJ calculate_synchronization_energy(
        const Workflow& workflow,
        const SchedulingStrategy& strategy,
        std::uint32_t num_controllers = 1) const;
    
    /**
     * Calculate energy for a single task on an edge node.
     * 
     * @param task The task
     * @param edge_node The edge node
     * @return Energy in Joules for executing this task on this node
     * @throws CalculationException if calculation fails
     */
    EnergyJ calculate_task_edge_energy(const Task& task, const EdgeNode& edge_node) const;
    
    /**
     * Calculate energy for a single task on data center.
     * 
     * Data center execution typically has different power characteristics.
     * Simplified model: energy = task_size * data_center_power_rate
     * 
     * @param task The task
     * @param datacenter_power_rate Power rate for data center in KW/byte
     * @return Energy in Joules for executing this task on data center
     */
    EnergyJ calculate_task_datacenter_energy(
        const Task& task,
        double datacenter_power_rate = 0.001) const;
    
    // ========================================================================
    // HELPER METHODS
    // ========================================================================
    
    /**
     * Calculate transmission cost factor for network communication.
     * 
     * Models the energy cost of transmitting data over network.
     * Cost = (task_size_bytes / bandwidth_mbps) * distance_km * transmission_power_factor
     * 
     * @param task_size Task size in bytes
     * @param bandwidth Bandwidth in Mbps
     * @param distance Distance in kilometers
     * @return Energy cost in Joules
     */
    EnergyJ calculate_transmission_cost(
        TaskSize task_size,
        BandwidthMbps bandwidth,
        DistanceKm distance) const;
    
private:
    // Constants for energy calculations
    static constexpr double TRANSMISSION_POWER_FACTOR = 0.001;  // KW per Mbps
    static constexpr double BYTES_TO_MB_FACTOR = 1.0 / (1024.0 * 1024.0);
    static constexpr double MS_TO_HOURS = 1.0 / (1000.0 * 60.0 * 60.0);
};

}  // namespace sdn
