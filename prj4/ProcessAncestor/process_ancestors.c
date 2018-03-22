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
	struct task_struct task_struct;
	struct task_struct *task;

	struct list_head *list;
	
	long count=0;
	int countChildren=0;
	int count_sibling=0;
	int i=0;
	int j=0;	


   

	for (task = current; task != &init_task; task = task->parent) {
		
		process.pid = (long)task->pid; 
	
		process.state = task->state;
		
		memset(process.name, '\0', sizeof(process.name));
		strcpy(process.name, task->comm);
		

		list_for_each(list, &current->children) {
			countChildren++;
		}
		list_for_each(list, &current->sibling) {
			count_sibling++;
		}
		process.num_children = countChildren;
		process.num_siblings = count_sibling;		
		
		info_array[i] = process;
		i++;
	}

	return 0;
}