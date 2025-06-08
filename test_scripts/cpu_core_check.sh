#!/bin/bash
# Check on which CPU core the task is running

echo "===== CPU Core Distribution Test ====="

# Run a sample background task
yes > /dev/null &
TASK_PID=$!
sleep 1

# Display CPU core assignment
ps -o pid,psr,ni,pri,comm -p $TASK_PID

# Optional: Pin task to core 0 or 1 (BFS cores)
taskset -cp 0 $TASK_PID

# Recheck CPU core assignment
sleep 1
ps -o pid,psr,ni,pri,comm -p $TASK_PID

kill $TASK_PID
echo "===== Test Completed ====="
