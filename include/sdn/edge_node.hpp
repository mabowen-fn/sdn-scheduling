#pragma once

#include "types.hpp"
#include <cstdint>

namespace sdn {

// ============================================================================
// EDGE NODE CLASS
// ============================================================================

/**
 * Represents a computing edge node in the network.
 * 
 * An edge node provides:
 * - Computing capacity (number of VMs and their processing power)
 * - Energy metrics (power consumption rates)
 * - Execution capabilities for tasks
 */
class EdgeNode {
public:
    // ========================================================================
    // CONSTRUCTORS & LIFECYCLE
    // ========================================================================
    
    /**
     * Construct an edge node.
     * 
     * @param id Unique edge node ID
     * @param num_vms Number of virtual machines
     * @param processing_power_per_vm Processing power per VM in MHz
     * @param power_rate_startup Startup power rate in KW
     * @param power_rate_idle Idle power rate in KW
     * @param power_rate_operating Operating power rate in KW
     * @throws InvalidWorkflowException if parameters are invalid
     */
    EdgeNode(EdgeNodeId id,
             std::uint32_t num_vms,
             ProcessingPowerMhz processing_power_per_vm,
             double power_rate_startup,
             double power_rate_idle,
             double power_rate_operating);
    
    // Delete default constructor
    EdgeNode() = delete;
    
    // Allow move semantics
    EdgeNode(EdgeNode&&) = default;
    EdgeNode& operator=(EdgeNode&&) = default;
    
    // Copy constructor/assignment
    EdgeNode(const EdgeNode&) = default;
    EdgeNode& operator=(const EdgeNode&) = default;
    
    ~EdgeNode() = default;
    
    // ========================================================================
    // ACCESSORS - IDENTITY
    // ========================================================================
    
    /// Get edge node ID
    EdgeNodeId id() const { return id_; }
    
    // ========================================================================
    // ACCESSORS - CAPACITY
    // ========================================================================
    
    /// Get number of virtual machines
    std::uint32_t num_vms() const { return num_vms_; }
    
    /// Get total processing power in MHz
    ProcessingPowerMhz total_processing_power() const {
        return num_vms_ * processing_power_per_vm_;
    }
    
    /// Get processing power per VM in MHz
    ProcessingPowerMhz processing_power_per_vm() const {
        return processing_power_per_vm_;
    }
    
    // ========================================================================
    // ACCESSORS - ENERGY METRICS
    // ========================================================================
    
    /// Get startup power rate in KW
    double power_rate_startup() const { return power_rate_startup_; }
    
    /// Get idle power rate in KW
    double power_rate_idle() const { return power_rate_idle_; }
    
    /// Get operating power rate in KW
    double power_rate_operating() const { return power_rate_operating_; }
    
    // ========================================================================
    // EXECUTION TIME CALCULATION
    // ========================================================================
    
    /**
     * Calculate execution time for a task.
     * 
     * Execution time = task_size / total_processing_power
     * 
     * @param task_size Size of task in bytes
     * @return Execution time in milliseconds
     */
    TimeMs calculate_execution_time(TaskSize task_size) const;
    
    /**
     * Calculate execution energy for a task.
     * 
     * Based on task size and operating power rate.
     * 
     * @param task_size Size of task in bytes
     * @return Execution energy in Joules
     */
    EnergyJ calculate_execution_energy(TaskSize task_size) const;
    
    // ========================================================================
    // COMPARISON & HASHING
    // ========================================================================
    
    /// Compare edge nodes by ID
    bool operator==(const EdgeNode& other) const { return id_ == other.id_; }
    bool operator<(const EdgeNode& other) const { return id_ < other.id_; }
    
private:
    EdgeNodeId id_;
    std::uint32_t num_vms_;
    ProcessingPowerMhz processing_power_per_vm_;
    double power_rate_startup_;
    double power_rate_idle_;
    double power_rate_operating_;
};

}  // namespace sdn
