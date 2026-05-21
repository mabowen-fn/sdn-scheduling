#pragma once

/**
 * SDN-Based Edge Computing Workflow Scheduler
 * 
 * A comprehensive C++ implementation of the research paper:
 * "Dynamic Resource Provisioning for Workflow Scheduling Under Uncertainty
 *  in Edge Computing Environment" (Xu et al., 2020)
 */

// Core data structures
#include "types.hpp"
#include "task.hpp"
#include "edge_node.hpp"
#include "workflow.hpp"
#include "scheduling_strategy.hpp"

// Computation engines (forward declarations - implement later)
// #include "energy_calculator.hpp"
// #include "time_calculator.hpp"
// #include "success_rate_calculator.hpp"
// #include "critical_path_analyzer.hpp"

// Genetic algorithm components (forward declarations)
// #include "chromosome.hpp"
// #include "population.hpp"
// #include "nsga3_engine.hpp"

// Optimization components (forward declarations)
// #include "fitness_evaluator.hpp"
// #include "strategy_selector.hpp"

// Control framework (forward declarations)
// #include "sdn_controller.hpp"
// #include "uncertainty_detector.hpp"

// Configuration (forward declarations)
// #include "config_manager.hpp"

namespace sdn {

/// Library version
inline constexpr const char* VERSION = "1.0.0";

/// Get library version string
inline const char* get_version() { return VERSION; }

}  // namespace sdn
