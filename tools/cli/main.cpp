#include "sdn/sdn.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "SDN-Based Edge Computing Workflow Scheduler v" << sdn::VERSION << std::endl;
    std::cout << "===========================================" << std::endl;
    
    if (argc < 2) {
        std::cout << "\nUsage: sdn-scheduler-cli <command> [options]" << std::endl;
        std::cout << "\nCommands:" << std::endl;
        std::cout << "  schedule <workflow> <config>  - Schedule a workflow" << std::endl;
        std::cout << "  test                          - Run basic tests" << std::endl;
        std::cout << "  version                       - Show version information" << std::endl;
        return 1;
    }
    
    std::string command = argv[1];
    
    if (command == "version") {
        std::cout << "Version: " << sdn::VERSION << std::endl;
        return 0;
    } else if (command == "test") {
        std::cout << "Running basic tests..." << std::endl;
        
        try {
            // Test 1: Create a task
            sdn::Task task1(0, 1000, 100);
            std::cout << "✓ Created task" << std::endl;
            
            // Test 2: Create an edge node
            sdn::EdgeNode edge_node(0, 4, 2000, 0.2, 0.03, 0.05);
            std::cout << "✓ Created edge node" << std::endl;
            
            // Test 3: Create a workflow
            sdn::Workflow workflow(0);
            workflow.add_task(sdn::Task(0, 1000, 100));
            workflow.add_task(sdn::Task(1, 2000, 150));
            workflow.add_dependency(0, 1);
            workflow.validate();
            std::cout << "✓ Created workflow with dependencies" << std::endl;
            
            // Test 4: Create a scheduling strategy
            sdn::SchedulingStrategy strategy(0, 2);
            strategy.set_assignment(0, 0);
            strategy.set_assignment(1, std::nullopt);
            std::cout << "✓ Created scheduling strategy" << std::endl;
            
            std::cout << "\n✓ All basic tests passed!" << std::endl;
            return 0;
        } catch (const sdn::SDNException& e) {
            std::cerr << "✗ Test failed: " << e.what() << std::endl;
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "✗ Unexpected error: " << e.what() << std::endl;
            return 1;
        }
    } else if (command == "schedule") {
        std::cout << "Scheduling workflow..." << std::endl;
        std::cout << "Note: Full scheduling implementation in Phase 3-5" << std::endl;
        return 0;
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }
}
