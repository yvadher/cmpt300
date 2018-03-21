#include <linux/kernel.h>
#include <linux/uaccess.h>

#include "array_stats.h"

asmlinkage long sys_array_stats(struct array_stats *stats, long data[], long size){

	struct array_stats temp = {0, 0, 0};
	long dataIn = 0;
	int i = 0;
	
	if (size <= 0){
		return -EINVAL;	
	}	

	while (i<size)
	{

		if (copy_from_user(&dataIn, &data[i], sizeof(data[i])))
		{
			return -EFAULT;
		} 		

		//Set the min max values to the first element
		if (i == 0){
			temp.min = dataIn;
			temp.max = dataIn;
		}

		if (temp.min > dataIn)
		{
			temp.min = dataIn;
		}
		if (temp.max < dataIn)
		{
			temp.max = dataIn;
		}

		temp.sum = (temp.sum + dataIn);
		
		i++;
	}

	if (copy_to_user(stats, &temp, sizeof(temp)))
	{
		return -EFAULT;
	}
	return 0;

}
