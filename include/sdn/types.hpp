#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <stdexcept>

namespace sdn {

// ============================================================================
// FUNDAMENTAL TYPES
// ============================================================================

/// Unique identifier for tasks
using TaskId = std::uint32_t;

/// Unique identifier for edge nodes
using EdgeNodeId = std::uint32_t;

/// Unique identifier for workflows
using WorkflowId = std::uint32_t;

/// Unique identifier for controllers
using ControllerId = std::uint32_t;

/// Task size in bytes
using TaskSize = std::uint64_t;

/// Time value in milliseconds
using TimeMs = std::uint64_t;

/// Energy value in Joules
using EnergyJ = double;

/// Distance value in kilometers
using DistanceKm = double;

/// Processing power in MHz
using ProcessingPowerMhz = std::uint32_t;

/// Bandwidth in Mbps
using BandwidthMbps = std::uint32_t;

// ============================================================================
// EXCEPTION TYPES
// ============================================================================

class SDNException : public std::runtime_error {
public:
    explicit SDNException(const std::string& message)
        : std::runtime_error(message) {}
};

class InvalidWorkflowException : public SDNException {
public:
    explicit InvalidWorkflowException(const std::string& msg)
        : SDNException("Invalid workflow: " + msg) {}
};

class InvalidScheduleException : public SDNException {
public:
    explicit InvalidScheduleException(const std::string& msg)
        : SDNException("Invalid schedule: " + msg) {}
};

class CalculationException : public SDNException {
public:
    explicit CalculationException(const std::string& msg)
        : SDNException("Calculation error: " + msg) {}
};

// ============================================================================
// CONFIGURATION TYPES
// ============================================================================

struct SimulationConfig {
    // Bandwidth between mobile devices and edge nodes (Mbps)
    BandwidthMbps bandwidth_between_mobile_and_edge = 1000;
    
    // Processing power per VM (MHz)
    ProcessingPowerMhz processing_power_per_vm = 2000;
    
    // Number of edge nodes
    std::uint32_t num_edge_nodes = 20;
    
    // Power consumption values
    double power_rate_startup_kw = 0.2;      // KW
    double power_rate_idle_kw = 0.03;         // KW
    double power_rate_operating_kw = 0.05;    // KW
    
    // QoS deadline factor (α > 1)
    double deadline_factor = 1.5;
    
    // GA parameters
    std::uint32_t population_size = 100;
    std::uint32_t num_generations = 50;
    double crossover_probability = 0.8;
    double mutation_probability = 0.1;
    
    // Weights for multi-objective optimization
    double weight_completion_time = 0.5;
    double weight_energy = 0.5;
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/// Check if value is approximately equal to another (for floating-point)
inline bool approximately_equal(double a, double b, double epsilon = 1e-9) {
    return std::abs(a - b) < epsilon;
}

/// Clamp a value to [min, max]
template <typename T>
T clamp(T value, T min_val, T max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

}  // namespace sdn
