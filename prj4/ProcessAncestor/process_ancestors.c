#include "process_ancestors.h"
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/uaccess.h>


asmlinkage long sys_process_ancestors(struct process_info info_array[],long size,long *num_filled) ;

//Helper functions
struct process_info get_process_info(struct task_struct* this) {
    struct process_info result;
    struct list_head* ptr;
    int count_children = 0;
    int count_sibalings = 0;

    /* Assign trivial info. */
    result.pid = this->pid;
    memcpy(result.name, this->comm, ANCESTOR_NAME_LEN);  
    result.state = this->state;
    result.uid = this->cred->uid.val;
    result.nvcsw = this->nvcsw;
    result.nivcsw = this->nivcsw;

	
    list_for_each(ptr, &(this->children)) {
    	++count_children;
    }
	
    list_for_each(ptr, &(this->sibling)) {
    	++count_sibalings;
    }

    //Assign childrens and sibalings
    result.num_children = count_children;
    result.num_siblings = count_sibalings;

    return result;
}

asmlinkage long sys_process_ancestors(struct process_info info_array[],long size,long *num_filled) {
	int i;
	long filled_processes= 0;
	struct task_struct *task = current; 
	struct task_struct *prev;
	struct process_info *info_array_temp;

    	//Error checks
    	if (size<=0) return -EINVAL;
	if (!info_array || !num_filled) return -EFAULT;
	 
	
	// Allocate memory
    	info_array_temp = kmalloc(sizeof (struct process_info) * size, GFP_KERNEL);
	
	// Copy info_array from user to kernal space
	
	for (i = 0; i < size; i++) {
		if (copy_from_user(&info_array_temp[i], &info_array[i], sizeof (struct process_info))) {
		    kfree(info_array_temp);
				printk("Safe memory violation in porcess ancestors. Cant copy the memory from user to kernal.\n");
		    return -EFAULT;
		}
	}

	// fill info_array_temp with irrerating to processes
	i = 0;
	do {
		info_array_temp[i] = get_process_info(task);
		filled_processes++;
		prev = task;
		task= task->parent;
		i++;
	} while (i < size && task != prev);


	//Copy info_array_temp to user space
	for (i = 0; i < filled_processes; i++) {
		if (copy_to_user(&info_array[i], &info_array_temp[i], sizeof (struct process_info))) {
		    kfree(info_array_temp);
				printk("Safe memory violation! Cant copy data to user space!");
		    return -EFAULT;
		}
	}

	// Copy  the num_filed number
	if (copy_to_user(num_filled, &filled_processes, sizeof (long))) {
		kfree(info_array_temp);
		printk("Safe memory violation! Cant copy data to user space!");
		return -EFAULT;
	}
	
	kfree(info_array_temp);
	return 0;
}
