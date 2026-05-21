# SDN-Based Edge Workflow Scheduler

A compact C++ implementation of dynamic resource provisioning and workflow scheduling for edge computing. Uses SDN-aware control to adapt task placement under runtime uncertainty (performance degradation, failures, new arrivals).

Problems it solves
- Reduces end-to-end latency by placing tasks on suitable edge nodes.
- Minimizes energy and completion time while meeting QoS deadlines.
- Adapts to runtime uncertainty with fast, dynamic re-scheduling.

Benefits
- Improved QoS and robustness under real-world variability.
- Better resource and energy efficiency compared to static schedules.
- Small, portable C++ codebase suitable for simulation and experimentation.

Quick start — build & run
```bash
# Configure and build (macOS/Linux)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.ncpu)

# Run lightweight smoke tests
./build/sdn-scheduler-cli test

# Run the scheduler (example)
./build/sdn-scheduler-cli schedule <workflow-file> <config-file>
```

Reference
- Xu, X., Cao, H., Geng, Q., et al. (2020). "Dynamic Resource Provisioning for Workflow Scheduling Under Uncertainty in Edge Computing Environment." Concurrency and Computation: Practice and Experience, 32(14), e5674.

