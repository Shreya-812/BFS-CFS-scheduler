/*
 * sched_core_mod.c
 *
 * Modified Linux kernel core scheduler logic to support hybrid BFS-CFS scheduling.
 * 
 * Key changes:
 * - Added support for manual switching of tasks to BFS scheduling policy.
 * - Restricted BFS execution to CPU cores 0 and 1.
 * - Implemented a fallback timer to revert BFS tasks to CFS after 15 minutes.
 *
 * Note: This is a simplified reference patch demonstrating core changes.
 */

#include <linux/sched.h>
#include <linux/ktime.h>
#include <linux/timer.h>
#include <linux/cpumask.h>

#define BFS_ALLOWED_CORES_MASK 0x3  // binary 11 => CPU0 and CPU1 allowed
#define BFS_FALLBACK_TIME (15 * 60 * HZ)  // 15 minutes fallback in jiffies

// Extend task_struct to hold BFS scheduling info (example)
struct task_struct {
    // existing fields ...
    bool bfs_enabled;             // Flag indicating if task is scheduled on BFS
    unsigned long bfs_start_time; // Timestamp when task switched to BFS
    // existing fields ...
};

// Check if the current CPU is allowed for BFS execution
static inline bool is_bfs_cpu_allowed(int cpu)
{
    return (BFS_ALLOWED_CORES_MASK & (1 << cpu)) != 0;
}

// Hook point before scheduling decision
void scheduler_tick(struct rq *rq)
{
    struct task_struct *p = rq->curr;
    unsigned long now = jiffies;

    // Check if task is BFS-enabled and running on allowed cores
    if (p->bfs_enabled) {
        if (!is_bfs_cpu_allowed(cpu_of(rq))) {
            // Migrate task to allowed CPU or fallback to CFS
            // (Simplified example)
            p->bfs_enabled = false;
            // Log fallback event
            printk(KERN_INFO "BFS fallback: task %d reverted to CFS\n", p->pid);
        }
        else {
            // Check if fallback time exceeded
            if (time_after(now, p->bfs_start_time + BFS_FALLBACK_TIME)) {
                p->bfs_enabled = false;
                printk(KERN_INFO "BFS timeout: task %d reverted to CFS after 15 minutes\n", p->pid);
            }
        }
    }

    // Continue with normal scheduling tick logic...
}

// Function to manually enable BFS for a task (could be called from userspace via syscall)
int enable_bfs_for_task(struct task_struct *task)
{
    if (!task)
        return -EINVAL;

    task->bfs_enabled = true;
    task->bfs_start_time = jiffies;
    printk(KERN_INFO "BFS enabled for task %d\n", task->pid);

    // Assign task affinity to allowed BFS CPUs only
    cpumask_t mask = CPU_MASK_NONE;
    cpumask_set_cpu(0, &mask);
    cpumask_set_cpu(1, &mask);
    set_cpus_allowed_ptr(task, &mask);

    return 0;
}

// Additional scheduler modifications to integrate BFS logic as needed...

