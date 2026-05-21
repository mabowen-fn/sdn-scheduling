cmake_minimum_required(VERSION 3.15)
project(sdn-scheduler
    VERSION 1.0.0
    DESCRIPTION "SDN-based Edge Computing Workflow Scheduler"
    LANGUAGES CXX
)

# Set C++ standard and flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for all builds
add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wno-sign-conversion
    -fno-omit-frame-pointer
)

# Build type specific flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Project directories
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

# Include directories for all targets
include_directories(${INCLUDE_DIR})

# ============================================================================
# CORE LIBRARY
# ============================================================================

add_library(sdn-scheduler STATIC
    # Core data structures
    ${SOURCE_DIR}/core/workflow.cpp
    ${SOURCE_DIR}/core/task.cpp
    ${SOURCE_DIR}/core/edge_node.cpp
    ${SOURCE_DIR}/core/scheduling_strategy.cpp
    
    # Computation engines
    ${SOURCE_DIR}/compute/energy_calculator.cpp
    ${SOURCE_DIR}/compute/time_calculator.cpp
    ${SOURCE_DIR}/compute/success_rate_calculator.cpp
    ${SOURCE_DIR}/compute/critical_path_analyzer.cpp
    
    # GA/NSGA-III
    ${SOURCE_DIR}/ga/chromosome.cpp
    ${SOURCE_DIR}/ga/population.cpp
    ${SOURCE_DIR}/ga/genetic_operators.cpp
    ${SOURCE_DIR}/ga/nsga3_engine.cpp
    
    # Optimization
    ${SOURCE_DIR}/optimize/fitness_evaluator.cpp
    ${SOURCE_DIR}/optimize/objective_normalizer.cpp
    ${SOURCE_DIR}/optimize/strategy_selector.cpp
    
    # Control framework
    ${SOURCE_DIR}/control/sdn_controller.cpp
    ${SOURCE_DIR}/control/uncertainty_detector.cpp
    ${SOURCE_DIR}/control/dynamic_reoptimizer.cpp
    
    # Configuration
    ${SOURCE_DIR}/config/config_manager.cpp
)

target_include_directories(sdn-scheduler
    PUBLIC
        $<BUILD_INTERFACE:${INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${SOURCE_DIR}
)

set_target_properties(sdn-scheduler PROPERTIES
    PUBLIC_HEADER "${INCLUDE_DIR}/sdn/*.hpp"
)

# ============================================================================
# EXECUTABLE
# ============================================================================

add_executable(sdn-scheduler-cli
    tools/cli/main.cpp
)

target_link_libraries(sdn-scheduler-cli PRIVATE sdn-scheduler)

# ============================================================================
# TESTS
# ============================================================================

enable_testing()

# Catch2 header-only
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/catch2)

add_executable(sdn-scheduler-tests
    # Unit tests
    ${TEST_DIR}/unit/test_workflow.cpp
    ${TEST_DIR}/unit/test_task.cpp
    ${TEST_DIR}/unit/test_edge_node.cpp
    ${TEST_DIR}/unit/test_scheduling_strategy.cpp
    ${TEST_DIR}/unit/test_energy_calculator.cpp
    ${TEST_DIR}/unit/test_time_calculator.cpp
    ${TEST_DIR}/unit/test_success_rate_calculator.cpp
    ${TEST_DIR}/unit/test_critical_path.cpp
    ${TEST_DIR}/unit/test_chromosome.cpp
    ${TEST_DIR}/unit/test_genetic_operators.cpp
    ${TEST_DIR}/unit/test_nondominated_sorting.cpp
    ${TEST_DIR}/unit/test_reference_points.cpp
    ${TEST_DIR}/unit/test_normalization.cpp
    ${TEST_DIR}/unit/test_strategy_selector.cpp
    ${TEST_DIR}/unit/test_main.cpp
)

target_link_libraries(sdn-scheduler-tests PRIVATE sdn-scheduler)

add_test(NAME sdn-scheduler-unit-tests COMMAND sdn-scheduler-tests)

# Integration tests
add_executable(sdn-scheduler-integration-tests
    ${TEST_DIR}/integration/test_integration.cpp
    ${TEST_DIR}/integration/test_main.cpp
)

target_link_libraries(sdn-scheduler-integration-tests PRIVATE sdn-scheduler)

add_test(NAME sdn-scheduler-integration-tests COMMAND sdn-scheduler-integration-tests)

# Benchmark tests
add_executable(sdn-scheduler-benchmarks
    ${TEST_DIR}/bench/test_benchmarks.cpp
    ${TEST_DIR}/bench/test_main.cpp
)

target_link_libraries(sdn-scheduler-benchmarks PRIVATE sdn-scheduler)

add_test(NAME sdn-scheduler-benchmarks COMMAND sdn-scheduler-benchmarks)

# ============================================================================
# INSTALLATION
# ============================================================================

install(TARGETS sdn-scheduler sdn-scheduler-cli
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/sdn
)

install(DIRECTORY ${INCLUDE_DIR}/ DESTINATION include)

# ============================================================================
# SUMMARY
# ============================================================================

message(STATUS "========================================")
message(STATUS "SDN Scheduler Configuration")
message(STATUS "========================================")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Source Directory: ${SOURCE_DIR}")
message(STATUS "Include Directory: ${INCLUDE_DIR}")
message(STATUS "Library: sdn-scheduler")
message(STATUS "Executable: sdn-scheduler-cli")
message(STATUS "Tests: Enabled")
message(STATUS "========================================")
