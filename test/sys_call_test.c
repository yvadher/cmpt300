#include "process_ancestors.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

#define PROCESS_ANCESTORS_SYS_CALL 342

const long ARRAY_SIZE = 10L;

int main() {
	struct process_info array[ARRAY_SIZE];
	long num_filled;
	struct process_info *process;
	int i;

	printf("Diving to kernel level\n\n");

	long result = syscall(PROCESS_ANCESTORS_SYS_CALL, arr, ARRAY_SIZE, &num_filled);
	printf("Result : %ld | num_filled: %ld | \n", result, num_filled);

    printf("Process info:\n");
	for (i = 0; i < num_filled; i++) {
		printf("pid: %ld\nname: %s\nstate: %ld\nuid: %ld\nnvcsw: %ld\nnivcsw: %ld\nnum_children: %ld\nnum_siblings: %ld\n\n",
			array[i].pid,
			array[i].name,
			array[i].state,
			array[i].uid,
			array[i].nvcsw,
			array[i].nivcsw,
			array[i].num_children,
			array[i].num_siblings);
	}

	// should atleast have swapper, init, and the process itself (probably has bash as well)
	assert(num_filled >= 3);

	//checking swapper
	process = &array[num_filled - 1];
	assert(process->pid == 0);
	assert(process->num_siblings == 0);
	assert(process->num_children > 0);

	// checking init
	process = &array[num_filled - 2];
	assert(process->pid == 1);
	assert(process->num_children > 0);

	// checking this caller process itself
	process = &array[0];
	assert(process->num_children == 0);

	// checking the number of siblings should be consistent with number of children of its parent
	for (i = num_filled - 1; i > 0; i--) {
		assert(array[i].num_children == array[i-1].num_siblings);
	}
	printf("\n\n\nAll the tests passed\n");
	return 0;
}

