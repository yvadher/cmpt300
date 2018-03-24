#include "process_ancestors.h"
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/string.h>


asmlinkage long sys_process_ancestors(struct process_info info_array[],long size,long *num_filled) ;


asmlinkage long sys_process_ancestors(struct process_info info_array[],long size,long *num_filled) {

	//Error checks
    	if (size<=0) return -EINVAL;
	if (!info_array || !num_filled) return -EFAULT;
	 
	int counter= 0;
	struct task_struct *task = current;	
	
	while (counter < size){
		struct process_info process;
		
		process.pid = (long)task->pid; 
		strncpy(process.name, task->comm, 16);
		
		process.state = (long) task->state;
		process.uid = (long) task->cred->uid.val;
		process.nvcsw = task->nvcsw;
		process.nivcsw = task->nivcsw;
		
		//Finding childrens 
		if (&current->children == NULL){
			return 0;		
		}
		
		long countChildren = 0;
		struct task_struct *child_task;
		struct list_head *children_list;
		list_for_each(children_list, &task->children) {
			child_task = list_entry(children_list, struct task_struct, children);
			countChildren++;
		}
		process.num_children = countChildren;

		//Finding a sibalings 
		long countSibalings = 0;

		if(&task->sibling == NULL) {
			return 0;
		}

		struct task_struct *sibling_task;
		struct list_head *sibling_list;
		list_for_each(sibling_list, &task->sibling) {
			sibling_task = list_entry(sibling_list, struct task_struct, sibling);
			countSibalings++;
		}

		process.num_siblings = countSibalings;

		//Break loop if tasks prent is itself 
		if (task == task->parent){
			info_array[counter] = process;
			counter++;
			break;		
		}

		task = task->parent;
		info_array[counter] = process;
		counter++;
		
	}

	*num_filled = counter;
	return 0;
}
