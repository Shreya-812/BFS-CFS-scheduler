# BFS-CFS Hybrid Scheduler Simulation

## Project Overview

This repository documents the simulation and testing of a **hybrid BFS-CFS (Brain Fuck Scheduler - Completely Fair Scheduler)** kernel-level modification. The goal is to enable dynamic switching between BFS and CFS scheduling policies in a Linux environment based on task characteristics, core availability, and time-based fallbacks.

The project is executed inside a virtualized Ubuntu setup to ensure safe kernel modification without affecting the host system.

## Environment Setup

* **Host OS:** Windows 11 (HP Victus, 11th Gen Intel Core i5/i7)
* **Virtualization Software:** VirtualBox
* **Guest OS:** Ubuntu 22.04 LTS Desktop
* **Linux Kernel Version:** 5.x (from kernel.org)

## Feature Summary

* Manual switching of tasks to BFS or default CFS scheduler
* BFS tasks restricted to specific CPU cores (e.g., 0 and 1)
* 15-minute timeout mechanism to revert tasks to CFS if they linger
* Logs and monitoring implemented via `dmesg`, `ps`, and custom logs

---

## Virtual Machine Initialization (20 minutes)

* Create a new VM named `Ubuntu_kernel_test`
* Allocate 4 GB RAM and 2 CPU cores
* Mount Ubuntu 22.04 ISO and install OS
* Skip Guest Additions if mounting fails
* Complete installation and update packages:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev
```

---

## Kernel Source Setup (25 minutes)

```bash
wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.15.1.tar.xz
tar -xvf linux-5.15.1.tar.xz
cd linux-5.15.1
make defconfig
```

---

## Kernel Modification: BFS-CFS Logic (30 minutes)

Files modified:

* `kernel/sched/core.c`
* `kernel/sched/fair.c`
* New File: `kernel/sched/bfs.c`

Features added:

* Manual trigger for BFS mode (e.g., via nice value or flag)
* BFS logic on specified cores only (CPU 0 & 1)
* Task fallback to CFS after 15 minutes

Version control:

```bash
git init
git add . && git commit -m "BFS-CFS hybrid scheduling logic implemented"
```

---

## Building and Installing Custom Kernel (40 minutes)

```bash
make -j$(nproc)
sudo make modules_install
sudo make install
```

---

## Booting into Custom Kernel (10 minutes)

```bash
sudo reboot
```

* During reboot, select the custom kernel from GRUB menu.

---

## Testing and Log Analysis (30 minutes)

* Create and execute sample load scripts
* Manually assign tasks to BFS via priority controls
* Confirm behavior through system logs

```bash
dmesg | grep "BFS"
ps -eo pid,psr,comm,ni,pri | grep test_task
```

---

## Approximate Duration

**Total Time:** \~2.5 to 3 Hours

---

## Screenshots and Output Samples

* Custom kernel boot entry in GRUB
* Terminal output for `dmesg` filters
* CPU-core distribution of test tasks
* Transitions logged from BFS back to CFS

(Screenshots will be uploaded soon)

---

## Future Development Scope

* Integrate AI for dynamic task classifier and scheduler switching
* Build GUI to manually manage scheduling and visualize process mapping
* Conduct benchmark testing against default Linux scheduling policies
* Improve BFS affinity and enhance fallback granularity


---

## Repository File Structure

* `sched/core.c` (modified logic)
* `sched/bfs.c` (new logic module)
* `Makefile`, `.config` (build configuration)
* `test_scripts/` (user scripts to trigger BFS mode)
