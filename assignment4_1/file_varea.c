#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/types.h>
#include <asm/syscall_wrapper.h>
#include <asm/uaccess.h>
#include <linux/init_task.h>
#include <linux/fs.h>
#include <linux/sched/mm.h>

#define __NR_ftrace 336

asmlinkage int (*real_ftrace)(const struct pt_regs *regs); 

void **syscall_table;       //System Call Table

static asmlinkage int file_varea(const struct pt_regs *regs){

    pid_t pid = (pid_t)(regs->di);      //Get pid from real_ftrace
    struct mm_struct *mm;
    struct vm_area_struct *vm;
    struct file* file;
    char* file_path;
    char buf[100];


    //find task struct using pid
    struct task_struct *findtask = pid_task(find_vpid(pid), PIDTYPE_PID);

    //can't find pid's task struct case
    if(findtask == NULL)    return -1;

   //get mm_struct and it's vm_area_struct
    mm = get_task_mm(findtask);
    vm = mm->mmap;

    //print task struct's info
    printk("###### Loaded files of a process '%d(%s)'' in VM ######\n",findtask->pid, findtask->comm);
    //print all virtual memory
    while(vm != NULL){
        file = vm->vm_file;
	//if file name is NULL, continue next
        if(file == NULL){
            vm = vm->vm_next;
            continue;
        }
	//setting path
        file_path = d_path(&file->f_path, buf, 100);
        printk("mem[%lx ~ %lx] code[%lx ~ %lx] data[%lx ~ %lx] heap[%lx ~ %lx] %s\n", vm->vm_start, vm->vm_end
                , mm->start_code, mm->end_code, mm->start_data, mm->end_data, mm->start_brk, mm->brk, file_path);
        vm = vm->vm_next;
    }
    printk("############################################################\n");

    mmput(mm);

    return pid;
}

//systemcall table get permission(Read Write)
void make_rw(void *addr){
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);
    if(pte->pte &~ _PAGE_RW)
        pte->pte |= _PAGE_RW;
}

//systemcall table get permission(Read Only)
void make_ro(void *addr){
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);
    
    pte->pte = pte->pte &~ _PAGE_RW;
}

//Module initialize Func.
static int __init file_varea_init(void){
    syscall_table = (void**) kallsyms_lookup_name("sys_call_table");
    make_rw(syscall_table);
    real_ftrace = syscall_table[__NR_ftrace];
    syscall_table[__NR_ftrace] = file_varea;
    make_ro(syscall_table);
    return 0;
}

//Module exit Func.
static void __exit file_varea_exit(void){
    make_rw(syscall_table);
    syscall_table[__NR_ftrace] = real_ftrace;
    make_ro(syscall_table);
}

module_init(file_varea_init);
module_exit(file_varea_exit);
MODULE_LICENSE("GPL");
