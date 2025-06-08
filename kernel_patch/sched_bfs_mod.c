/*
 * sched_bfs_mod.c
 *
 * BFS-like custom scheduling logic for selected tasks.
 * Integrated as part of a hybrid BFS-CFS scheduler in Linux.
 *
 * Key Features:
 * - Assigns BFS-flagged tasks to specific cores (e.g., CPU 0 and 1)
 * - Simplified round-robin or FIFO logic for BFS runqueue
 * - Auto-revert support after timeout via external trigger
 */

#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/timer.h>

#define BFS_CPU_MASK 0x3  // CPU 0 and 1
#define BFS_TIMEOUT_MIN 15

struct bfs_task_info {
    struct task_struct *task;
    struct list_head list;
    ktime_t bfs_start_time;
};

static LIST_HEAD(bfs_runqueue);
static DEFINE_SPINLOCK(bfs_lock);

// Add task to BFS queue
void enqueue_task_bfs(struct task_struct *task)
{
    struct bfs_task_info *info;

    if (!task || task->bfs_enabled)
        return;

    task->bfs_enabled = true;

    info = kmalloc(sizeof(*info), GFP_KERNEL);
    if (!info)
        return;

    INIT_LIST_HEAD(&info->list);
    info->task = task;
    info->bfs_start_time = ktime_get();

    spin_lock(&bfs_lock);
    list_add_tail(&info->list, &bfs_runqueue);
    spin_unlock(&bfs_lock);

    cpumask_clear(task->cpus_ptr);
    cpumask_set_cpu(0, task->cpus_ptr);
    cpumask_set_cpu(1, task->cpus_ptr);

    printk(KERN_INFO "[BFS] Task %d enqueued on BFS scheduler\n", task->pid);
}

// Simple round-robin BFS scheduler
struct task_struct *pick_next_task_bfs(void)
{
    struct bfs_task_info *info, *temp;
    struct task_struct *next_task = NULL;

    spin_lock(&bfs_lock);
    list_for_each_entry_safe(info, temp, &bfs_runqueue, list) {
        if (info->task->state == TASK_RUNNING) {
            next_task = info->task;
            list_move_tail(&info->list, &bfs_runqueue);  // rotate
            break;
        }
    }
    spin_unlock(&bfs_lock);

    return next_task;
}

// Revert expired BFS tasks to CFS
void check_and_revert_expired_bfs_tasks(void)
{
    struct bfs_task_info *info, *tmp;
    ktime_t now = ktime_get();
    s64 diff;

    spin_lock(&bfs_lock);
    list_for_each_entry_safe(info, tmp, &bfs_runqueue, list) {
        diff = ktime_to_minutes(ktime_sub(now, info->bfs_start_time));

        if (diff >= BFS_TIMEOUT_MIN) {
            struct task_struct *task = info->task;

            task->bfs_enabled = false;
            cpumask_copy(task->cpus_ptr, cpu_all_mask);
            list_del(&info->list);
            kfree(info);

            printk(KERN_INFO "[BFS] Task %d reverted to CFS after timeout\n", task->pid);
            // Note: enqueue in CFS done separately
        }
    }
    spin_unlock(&bfs_lock);
}
