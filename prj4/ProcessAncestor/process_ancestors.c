#include "process_ancestors.h"
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/compiler.h>
#include <asm/percpu.h>
#include <asm-generic/errno-base.h>
#include <linux/errno.h>
#include <linux/string.h>


asmlinkage long sys_process_ancestors(struct process_info info_array[],long size,long *num_filled) ;


asmlinkage long sys_process_ancestors(struct process_info info_array[],long size,long *num_filled) {
	//Error checks
    	if (size<=0) return -EINVAL;
	if (!info_array || !num_filled) return -EFAULT;

	struct process_info process;
	struct task_struct *task = current; // getting global current pointer
	struct task_struct *prev;

	struct list_head *list;
	
	long count=0;
	int countChildren=0;
	int countSibling=0;	

	do {
	    	// do the process
		process.pid = (long)task->pid;
		printk("Process ID: %ld\n", process.pid);	

		process.state = task->state;
		printk("Process State: %ld\n", process.state);		

		memset(process.name, '\0', sizeof(process.name));
		strcpy(process.name, task->comm);

		printk("Process Name: %s\n", process.name);
	

		list_for_each(list, &task->children) {
			countChildren++;
		}
		list_for_each(list, &task->sibling) {
			countSibling++;
		}
		process.num_children = countChildren;
		process.num_siblings = countSibling;	

		printk("Process numChildren: %ld\n", process.num_children);			
	
		info_array[count] = process;
		
		count++;		

		if(count == size){
			*num_filled = count;
			printk("Process numFilled: %ld\n", *num_filled);	
			return 0;		
		}

		
		
	    	prev = task;
	    	task = task->parent;

	} while (prev->pid != 0);


	*num_filled = count;
	printk("Process numFilled: %ld\n", *num_filled);	

	return 0;
}
