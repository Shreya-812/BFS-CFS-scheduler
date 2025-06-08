/*
 * sched_fair_mod.c
 *
 * Modifications to the Completely Fair Scheduler (CFS) for BFS-CFS hybrid scheduling.
 * 
 * Key updates:
 * - Respect task BFS state to avoid scheduling BFS-enabled tasks on CFS cores.
 * - Coordinate with BFS scheduler to ensure tasks revert properly.
 *
 * This file shows example hooks and changes; adjust with actual kernel APIs.
 */

#include <linux/sched.h>
#include <linux/cpumask.h>

extern bool is_bfs_task(struct task_struct *task);  // Helper from BFS code
extern void revert_task_to_cfs(struct task_struct *task);  // Revert logic

// Modified pick_next_task_fair to skip BFS tasks and avoid scheduling them on CFS cores
static struct task_struct *pick_next_task_fair(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
    struct task_struct *p;

    // Original CFS pick_next_task logic (simplified)
    p = pick_best_cfs_task(rq);

    // If the task is BFS-enabled, skip and pick another
    while (p && is_bfs_task(p)) {
        // Remove BFS task temporarily from CFS runqueue and pick next
        dequeue_task(rq, p, 0);
        p = pick_best_cfs_task(rq);
    }

    return p;
}

// Hook or wrapper for enqueue_task_fair to prevent BFS tasks from entering CFS runqueue
static void enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    if (is_bfs_task(p)) {
        // BFS tasks should not be enqueued in CFS runqueue
        return;
    }

    // Otherwise, enqueue normally
    enqueue_task_cfs(rq, p, flags);
}

// Hook for dequeue_task_fair if needed
static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    if (is_bfs_task(p)) {
        // BFS tasks are not in CFS queue
        return;
    }

    dequeue_task_cfs(rq, p, flags);
}

// Function to revert BFS tasks back to CFS - called on timeout or manual fallback
void revert_task_to_cfs(struct task_struct *task)
{
    if (!task)
        return;

    task->bfs_enabled = false;

    // Reset CPU affinity to all cores (simplified)
    cpumask_t full_mask = cpu_all_mask;
    set_cpus_allowed_ptr(task, &full_mask);

    // Enqueue the task in CFS runqueue
    // Note: locking and proper rq handling needed in real code
    enqueue_task_cfs(task_rq(task), task, 0);

    printk(KERN_INFO "Task %d reverted to CFS scheduler\n", task->pid);
}

// Helper to check if a task is BFS-enabled (could be inline or macro)
bool is_bfs_task(struct task_struct *task)
{
    return task->bfs_enabled;
}

