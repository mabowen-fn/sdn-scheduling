#include "sdn/edge_node.hpp"
#include <sstream>
#include <stdexcept>

namespace sdn {

// ============================================================================
// CONSTRUCTORS
// ============================================================================

EdgeNode::EdgeNode(EdgeNodeId id,
                   std::uint32_t num_vms,
                   ProcessingPowerMhz processing_power_per_vm,
                   double power_rate_startup,
                   double power_rate_idle,
                   double power_rate_operating)
    : id_(id),
      num_vms_(num_vms),
      processing_power_per_vm_(processing_power_per_vm),
      power_rate_startup_(power_rate_startup),
      power_rate_idle_(power_rate_idle),
      power_rate_operating_(power_rate_operating) {
    
    if (num_vms == 0) {
        throw InvalidWorkflowException("EdgeNode must have at least 1 VM");
    }
    if (processing_power_per_vm == 0) {
        throw InvalidWorkflowException("Processing power per VM must be greater than 0");
    }
    if (power_rate_startup < 0.0 || power_rate_idle < 0.0 || power_rate_operating < 0.0) {
        throw InvalidWorkflowException("Power rates cannot be negative");
    }
}

// ============================================================================
// EXECUTION CALCULATIONS
// ============================================================================

TimeMs EdgeNode::calculate_execution_time(TaskSize task_size) const {
    if (task_size == 0) {
        return 0;
    }
    
    // execution_time = task_size / total_processing_power
    // Convert task_size (bytes) to be compatible with processing power (MHz)
    // Simplified: time (ms) = size (bytes) / processing_power (MHz)
    // We treat bytes and MHz as compatible in the time calculation
    double processing_power_mhz = static_cast<double>(total_processing_power());
    double size_bytes = static_cast<double>(task_size);
    
    double time_ms = size_bytes / processing_power_mhz;
    
    // Ensure minimum 1ms execution time
    if (time_ms < 1.0) {
        return 1;
    }
    
    return static_cast<TimeMs>(time_ms);
}

EnergyJ EdgeNode::calculate_execution_energy(TaskSize task_size) const {
    if (task_size == 0) {
        return 0.0;
    }
    
    // Energy = (task_size * power_rate_operating) / processing_power
    // This gives us energy in Joules for executing this task
    double size_bytes = static_cast<double>(task_size);
    double processing_power_mhz = static_cast<double>(total_processing_power());
    
    // Normalize: energy = size_bytes / processing_power * power_rate
    double energy_j = (size_bytes / processing_power_mhz) * power_rate_operating_;
    
    return energy_j;
}

}  // namespace sdn
