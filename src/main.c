#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/uaccess.h>

// Your Target Configuration
#define TARGET_LIB "libunity.so"
#define TARGET_OFFSET 0x595632C

// Hex: 20 00 80 D2 C0 03 5F D6 (MOV W0, #1; RET)
static unsigned char patch_data[] = {0x20, 0x00, 0x80, 0xD2, 0xC0, 0x03, 0x5F, 0xD6};

// Helper to find the library base address in kernel space
static uintptr_t get_module_base(struct task_struct *task, const char *name) {
    struct mm_struct *mm = task->mm;
    struct vm_area_struct *vma;
    uintptr_t base = 0;

    if (!mm) return 0;

    mmap_read_lock(mm);
    for (vma = mm->mmap; vma; vma = vma->vm_next) {
        if (vma->vm_file) {
            char buf[256];
            char *path = d_path(&vma->vm_file->f_path, buf, sizeof(buf));
            if (!IS_ERR(path) && strstr(path, name)) {
                base = vma->vm_start;
                break;
            }
        }
    }
    mmap_read_unlock(mm);
    return base;
}

// Main patching logic
void run_wallhack_patch(void) {
    struct task_struct *task;
    for_each_process(task) {
        // You can filter by process name if needed: if (strstr(task->comm, "game_name"))
        uintptr_t base = get_module_base(task, TARGET_LIB);
        if (base) {
            // access_process_vm bypasses all user-mode write protections
            access_process_vm(task, base + TARGET_OFFSET, patch_data, sizeof(patch_data), 1);
            pr_info("KPM: Wallhack applied to %s at 0x%lx\n", task->comm, base + TARGET_OFFSET);
        }
    }
}

static int __init kpm_wallhack_init(void) {
    pr_info("KPM: Red Wallhack Module Loaded\n");
    run_wallhack_patch();
    return 0;
}

static void __exit kpm_wallhack_exit(void) {
    pr_info("KPM: Red Wallhack Module Unloaded\n");
}

module_init(kpm_wallhack_init);
module_exit(kpm_wallhack_exit);
MODULE_LICENSE("GPL");
