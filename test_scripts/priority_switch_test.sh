#!/bin/bash
# Test Script to Trigger BFS Scheduler via Priority

echo "===== Starting BFS Trigger Test ====="

# Create a CPU-intensive task
stress --cpu 1 --timeout 300 &  # Simulate workload
TASK_PID=$!
echo "Started test task with PID: $TASK_PID"

# Change the nice value to indicate BFS mode (-10 or similar)
renice -n -10 -p $TASK_PID

# Output task info
sleep 2
ps -o pid,psr,ni,pri,comm -p $TASK_PID

echo "Task should now be on BFS-designated core (e.g., CPU 0 or 1)"
echo "Check dmesg logs for BFS mode confirmation"

# Wait for task to complete
wait $TASK_PID

echo "===== BFS Trigger Test Completed ====="
