
# Kexec Boot Time Optimization for Kernel Live Updates

## Introduction

In this guide, we will explore how kernel boot optimization is essential in modern cloud environments, with a specific focus on improving the boot time during live updates of the host kernel. This discussion covers live updates, live migration, kernel patching, and the various optimization techniques for reducing the downtime during kernel updates.

We will also specialize this optimization process for the specific ECI device and provide solutions to preserve the operational states of virtual machines (VMs) and PCI devices during kernel updates.

## Motivation: Why Kernel Boot Time Matters

In both public and private cloud environments, once a workload is started and virtual machines (VMs) are running, users generally prefer to keep the host environment stable as long as possible. Kernel updates bring security, functionality, and performance benefits, but they can also cause significant downtime, especially for large-scale systems.

Updating the host kernel often requires a reboot, which disrupts ongoing operations. However, methods such as kernel patching and live migration address these challenges by enabling updates without requiring the system to be fully rebooted.

### What is Kernel Patching?

Kernel patching refers to the process of applying patches to the kernel to address bugs, security vulnerabilities, or to add new features. Kernel patching can be performed in two main ways:

1. **Full Kernel Update**: Involves rebooting the system to apply updates.
2. **Live Patching**: Allows updates to be applied without rebooting the system.

## Methods to Update the Host Kernel

There are two main methods for updating the host kernel without causing significant disruption:

1. **Live Migration**  
   Live migration involves transferring a running virtual machine (VM) from one host to another without major disruption. It is a great way to address physical hardware issues but is not directly related to kernel updates. This method requires additional resources and bandwidth.

2. **Live Updates**  
   Live updates involve updating the host kernel by pausing and snapshotting the VM running on the host, followed by a `kexec` boot into the new kernel. Once the kernel is loaded, the VM is resumed, minimizing downtime without requiring additional machines or resources.

### Advantages of Live Updates
- No additional resources needed (unlike live migration).
- Lower bandwidth usage compared to full migrations.
- Can be applied to solve critical security fixes and performance updates.

### PCI Device Pass-Through Issue

When virtual machines use PCI devices (via VFIO pass-through), it’s necessary to preserve the I/O Memory (IOMMU) state of these devices during live updates. The solution to this involves leveraging kernel persistent memory in KVM environments.

#### What is Kernel Persistent Memory?
Persistent memory retains data even when power is off, combining the speed of DRAM with the durability of SSDs and hard drives. This non-volatile memory can act as both fast memory and storage, improving performance and reliability.

### Other Challenges: Host User-Space Applications

Another issue during live updates is managing host user-space applications like DPDK and SPDK.

- **DPDK (Data Plane Development Kit)**: Used for networking tasks.
- **SPDK (Storage Performance Development Kit)**: Used for storage-related operations.

We need to ensure that these applications continue functioning correctly during and after the kernel update.

## Downtime During Live Updates

During live updates, several steps introduce downtime:

1. **VM Pause**: The VM is paused to take a snapshot.
2. **VM Snapshot**: Captures the state of the VM.
3. **Kexec Boot**: The system boots into the new kernel using `kexec`.
4. **VM Restore**: The VM state is restored.
5. **VM Resume**: The VM resumes operations.

### Measuring Kernel Boot Time

The kernel boot time can be measured by using the kernel timestamp logs. These logs mark the first entry, which includes the kernel version, and the final log, which indicates the start of the init process.

- **Star Pages**: The initialization of kernel pages often takes significant time. To optimize this, enabling deferred star pages can help by delaying their initialization, allowing it to run in parallel after the kernel swap daemon starts.

## SMP Boot Time Optimization

### What is SMP Boot Time?

SMP (Symmetric Multiprocessing) boot time refers to the time taken to initialize multiple processor cores and load the operating system. This process involves sequentially booting the CPU cores, which can be time-consuming.

### What is BP Kick?

"BP kick" refers to using breakpoints in the kernel to trigger debugging or runtime analysis. It allows developers to stop the execution at specific locations for inspection.

### Parallel SMP Boot Time

By enabling parallel CPU bring-up, multiple cores are initialized simultaneously, which speeds up the boot process. This parallel boot approach is especially beneficial for systems with many CPU cores, reducing boot times significantly.

#### Implementation

The `cpuhp.parallel=1` kernel parameter can enable or disable parallel CPU bring-up, reducing overall boot times.

### Benefits of Parallel Boot
- **Faster boot times** for systems with many CPU cores.
- **Minimized VM downtime** during reboot.

### Performance Gains
By using SMP parallel booting, we can reduce kernel time from 2.7 seconds to 1 second, and SMP boot time from 1.7 seconds to 60 milliseconds.

## Kernel Time vs. SMP Boot Time

The key difference between kernel boot time and SMP boot time is that kernel time measures the time taken by the kernel to load and initialize, while SMP boot time measures the initialization of multiple processor cores. Optimizing SMP boot time involves parallelizing the process of booting multiple CPUs.

## Kernel Update Steps Using Kexec

Below are the steps for updating the kernel using `kexec` without rebooting the system:

### 1. Check Current Kernel Version

```bash
uname -r
```

### 2. Install a Second Kernel (e.g., LTS)

```bash
sudo pacman -Syu linux-lts linux-lts-headers
```

### 3. Verify Installed Kernels

```bash
ls /boot
```

### 4. Install Kexec Tools

```bash
sudo pacman -S kexec-tools
```

### 5. Load the New Kernel into Memory (But Don’t Boot Yet)

```bash
sudo kexec -l /boot/vmlinuz-linux-lts --initrd=/boot/initramfs-linux-lts.img --command-line="$(cat /proc/cmdline)"
```

### 6. Sync and Prepare the File System

```bash
sudo sync
```

### 7. Isolate Services and Enter Rescue Mode

```bash
sudo systemctl isolate rescue.target
```

### 8. Save the Running Processes

Create a script `save-running.sh`:

```bash
#!/bin/bash
mkdir -p /var/tmp/kexec-session
ps -eo comm | sort | uniq > /var/tmp/kexec-session/running_apps.txt
```

Make it executable:

```bash
chmod +x save-running.sh
```

Run the script:

```bash
./save-running.sh
```

### 9. Boot into the New Kernel

```bash
sudo kexec -e
```

### 10. Restore Running Processes

Create the `restore-apps.sh` script to restore applications:

```bash
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
```

Make it executable:

```bash
chmod +x restore-apps.sh
```

Run the script:

```bash
./restore-apps.sh
```

### 11. Check System Boot Time

```bash
who -b
```
# 💻 Kernel Live Patching Demo on Arch Linux

This project demonstrates how to create and load a basic kernel module in Arch Linux that prints a message to the kernel log. It’s a great first step into understanding how kernel modules work — a foundation for live patching.

---

## 📦 Step 1: Install Required Packages

Install kernel headers and development tools needed for compiling kernel modules:

```bash
sudo pacman -S linux-headers git base-devel
sudo pacman -S gcc make elfutils
```

---

## 🌐 Step 2: (Optional) Clone kpatch Repository

You can clone the `kpatch` repo for exploring advanced live patching tools:

```bash
git clone https://github.com/dynup/kpatch.git
```

> **Note:** This isn't required for the demo below, but useful for deeper exploration.

---

## 🗂️ Step 3: Create Project Directory

```bash
mkdir ~/kpatch-demo
cd ~/kpatch-demo
```

---

## ✍️ Step 4: Write the Kernel Module (`hello.c`)

Create a file called `hello.c`:

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A Simple Hello World Kernel Module");

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello, BCS_4A! Project Present to Sir Amin\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Goodbye, BCS_4A!\n");
}

module_init(hello_init);
module_exit(hello_exit);
```

### 🔍 What does this file do?

This C file defines the kernel module:
- `hello_init()` is called when the module is loaded.
- `hello_exit()` is called when the module is removed.
- `printk()` sends a message to the kernel log (viewable using `dmesg`).

---

## ⚙️ Step 5: Write the `Makefile`

Create a file named `Makefile`:

```makefile
obj-m += hello.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

### 🔍 What does the Makefile do?

It tells the build system:
- How to compile your module using your kernel’s build system.
- Where the kernel headers are located.

---

## 🛠️ Step 6: Compile the Module

```bash
make
```

This will compile your kernel module and produce a file called `hello.ko`.

---

## 📥 Step 7: Insert the Kernel Module

```bash
sudo insmod hello.ko
```

> This inserts the kernel module into the running kernel.

---

## 🔎 Step 8: Verify Module is Loaded

```bash
lsmod | grep hello
```

You should see the module listed.

---

## 🧾 Step 9: Check Kernel Messages

```bash
dmesg | tail
```

This will show the most recent kernel log entries. You should see:

```
Hello, BCS_4A! Project Present to Sir Amin
```

---

## 🧹 Step 10: Remove the Kernel Module

```bash
sudo rmmod hello
dmesg | tail
```

You should now see:

```
Goodbye, BCS_4A!
```

---

## 🧠 Additional Tip (Optional but Helpful)

If you get this error while running `dmesg`:

```bash
dmesg: read kernel buffer failed: Operation not permitted
```

Run this command to add your user to the `adm` group (which has access to kernel logs):

```bash
sudo usermod -aG adm $USER
```

Then **log out and log back in** to apply the change.
---

## ✅ Summary

This demo helped you:
- Write and compile a simple kernel module.
- Insert and remove the module.
- View kernel log messages using `dmesg`.

### Resources

- Official Arch Wiki on Kexec: [Kexec - ArchWiki](https://wiki.archlinux.org/title/Kexec)
- Official Arch Wiki on Live Patching:[Arch Wiki – Kernel Live Patching](https://wiki.archlinux.org/title/Kernel_live_patching)
- My Personal Blog: [Preserving Services with Faster Kernel Reboots Using Kexec](https://systemadmin-insights.hashnode.dev/preserving-services-with-faster-kernel-reboots-using-kexec)



## Conclusion

In this guide, we’ve discussed the importance of reducing kernel boot time, especially during live updates. We explored methods like `kexec` and parallel SMP booting to reduce downtime and optimize the process. The use of persistent memory and handling user-space applications also plays a vital role in the efficiency of live updates.

### Key Takeaways:
- **Kernel live updates** are crucial for minimizing downtime in cloud and virtualized environments.
- **Parallel booting** of multiple cores using SMP significantly reduces boot time.
- **Persistent memory** ensures that devices and applications remain functional during updates.
- **Kexec** enables switching kernels without rebooting, allowing for faster updates.

---


