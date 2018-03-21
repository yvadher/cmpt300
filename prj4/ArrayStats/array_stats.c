
#include <stdio.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

#include "array_stats.h"

asmlinkage long sys_array_stats(struct array_stats *stats, long data[], long size){

	struct array_stats temp = {0, 0, 0};
	long data = 0;
	int i = 0;
	
	if (size <= 0){
		return -EINVAL;	
	}	

	while (i<size)
	{

		if (copy_from_user(&data, &data[i], sizeof(data[i])))
		{
			return -EFAULT;
		} 		

		//Set the min max values to the first element
		if (i == 0){
			temp.min = data;
			temp.max = data;
		}

		if (temp.min > data)
		{
			temp.min = data;
		}
		if (temp.max < data)
		{
			temp.max = data;
		}

		temp.sum = (temp.sum + data);
		
		i++;
	}

	if (copy_to_user(stats, &temp, sizeof(temp)))
	{
		return -EFAULT;
	}
	return 0;



}
