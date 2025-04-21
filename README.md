# Operating-system-project
This repository contains an operating systems project focused on boot time optimization and saving the state of running processes.

Improving Kexec Boot Time
Introduction

In this project, we explore kernel boot optimization, with a focus on live updates for the host kernel. The aim is to enhance the boot time for systems running in cloud environments, particularly when updating host kernels, while maintaining minimal downtime. We delve into live kernel updates, their benefits, challenges, and the specific optimizations tailored for the ECI (Industrial Containerized Execution) device.
What is ECI?

ECI stands for Execution Containerized Interface, a software platform that leverages virtualization and containerization to manage control execution as containerized microservices in industrial environments. In this context, optimizing the boot process is critical to maintaining uptime and improving system efficiency.
Motivation for Kernel Boot Optimization

The motivation behind optimizing kernel boot time is especially important in public and private cloud environments. Once a workload is initiated, virtual machines (VMs) run with the expectation that the host environment remains stable. However, updating the host kernel brings essential security, functionality, and performance benefits, which may not be achievable with other methods like kernel patching.
Kernel Patching

Kernel patching is a process to update the kernel to fix bugs, address security vulnerabilities, or add new features. It can be done either by updating the entire kernel, which requires a system reboot, or through live patching, which allows updates without interrupting the system.
Kernel Update Methods

There are two main ways to update the host kernel:
1. Live Migration

    Live migration involves transferring a running virtual machine (VM) from one host to another without causing significant disruption. It is primarily used to deal with hardware issues and is not the focus here.

2. Live Update

    Live updates allow updating the host kernel without the need for additional resources, unlike live migration. The process involves pausing the VM, taking a snapshot, booting into the new kernel using kexec, and resuming the VM without requiring extra bandwidth or machines.

Key Challenge: PCI Device Pass-through

    When PCI devices are passed through a virtual machine (e.g., via VFIO pass-through), it's essential to preserve their IOMMU (Input/Output Memory Management Unit) state during live updates. This can be handled using Kernel Persistent Memory.

Kernel Persistent Memory

    Persistent memory retains its data even when the power is turned off. It combines the speed of DRAM with the durability of SSDs, offering both high-speed memory and non-volatile storage.

User Space Applications: DPDK and SPDK

    DPDK (Data Plane Development Kit) is used for high-performance networking, and SPDK (Storage Performance Development Kit) handles storage-related operations. Both are crucial in environments where kernel updates are applied with minimal downtime.

Downtime During Live Updates

The downtime during live updates can be broken down as follows:

    VM Pause

    VM Snapshot

    Kexec Boot

    VM Restore

    VM Resume

Measuring Kernel Boot Time

The kernel boot time can be measured using kernel time-stamp logs. This log shows the kernel version and marks the point when the init process begins.
Optimization Solution: Deferred Startup Pages

One solution to optimize kernel boot time is to enable Deferred Startup Pages. This delays the initialization of certain memory pages by using a parallel thread, which reduces boot time significantly.
Optimizing SMP Boot Time

SMP Boot Time refers to the time taken to initialize multiple processor cores in a system. By booting the system sequentially on one processor, it leads to delays. However, Parallel SMP Boot allows for simultaneous initialization of multiple cores, reducing boot time.
What is BP Kick?

    BP Kick refers to using breakpoints (BP) within the kernel for debugging or runtime analysis. This technique allows developers to inspect the kernel state before continuing its execution.

Enabling Parallel Boot in Linux

To improve SMP boot times, we can enable parallel CPU bring-up by using the cpuhp.parallel=1 kernel parameter. This allows for faster boot times by initializing multiple CPU cores simultaneously.
Achieving Optimized Boot Times

By using parallel boot techniques, kernel boot time can be reduced from 2.7 seconds to 1 second, and SMP boot time can drop from 1.7 seconds to 60 milliseconds.
Steps for Kernel Update Using Kexec

    Check Current Kernel Version

uname -r

Install a Second Kernel (e.g., LTS)

sudo pacman -Syu linux-lts linux-lts-headers

Verify Installed Kernels

ls /boot

Install kexec-tools

sudo pacman -S kexec-tools

Load the New Kernel into Memory (But Don’t Boot Yet)

sudo kexec -l /boot/vmlinuz-linux-lts --initrd=/boot/initramfs-linux-lts.img --command-line="$(cat /proc/cmdline)"

Sync and Prepare the Filesystem

sudo sync

Enter Rescue Mode

sudo systemctl isolate rescue.target

Save Running Processes
Create a script (save-running.sh) to save currently running processes:

#!/bin/bash
mkdir -p /var/tmp/kexec-session
ps -eo comm | sort | uniq > /var/tmp/kexec-session/running_apps.txt

Make it executable:

chmod +x save-running.sh

Run the script:

./save-running.sh

Boot into the New Kernel Without Rebooting

sudo kexec -e

This command loads the new kernel from memory without performing a full reboot.

Restore Running Applications
Create a script (restore-apps.sh) to restore the saved processes:

#!/bin/bash
FILE="/var/tmp/kexec-session/running_apps.txt"

if [[ -f $FILE ]]; then
    while read -r app; do
        if command -v "$app" &>/dev/null; then
            nohup "$app" &>/dev/null &
            echo "Started $app"
        fi
    done < "$FILE"
else
    echo "No app list found!"
fi

Make it executable:

chmod +x restore-apps.sh

Run the script:

    ./restore-apps.sh

    Check System Boot Time
    Use the who -b command to check the last system boot time.

Conclusion

By leveraging Kexec for kernel updates and employing various optimizations like Parallel SMP Boot, Deferred Startup Pages, and Kernel Persistent Memory, we can significantly improve the kernel boot time and reduce downtime during live updates. Additionally, restoring user sessions ensures that the transition to the new kernel is smooth, minimizing disruption.
