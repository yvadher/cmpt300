#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>

#include "bbuff.h"
#include "stats.h"

typedef struct  {
    int factory_number;
    double time_stamp_in_ms;
} candy_t;

_Bool stop_thread = false;

//--------------------------- HELPER FUNCTIONS ---------------------------------------------
// Str to integer conversion
int strToInt(const char a[]){
	for (int i=0; a[i] != '\0'; i++){
		if (!isdigit(a[i])) return -1;
	}
	return atoi(a);
}

// Current time in ms
double current_time_in_ms(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000.0 + now.tv_nsec/1000000.0;
}

// createing object
candy_t* makeCandy(int id){
	candy_t* candy = malloc(sizeof(candy_t));
	candy->factory_number = id;
	candy->time_stamp_in_ms = current_time_in_ms();
	return candy;
}

//Functions  
void *func_factories(void * id){

	int idNo = *((int *) id);
	srand(time(NULL));

	while(!stop_thread){
		int rand_sec = rand()%4;
		printf("\tFactory %d ships candy & waits %ds\n", idNo, rand_sec );
		candy_t* candy = makeCandy(idNo);
		bbuff_blocking_insert(candy);
		stats_record_produced(idNo);
		
		sleep(rand_sec);	
	}
	printf("Candy-factory %d done\n", idNo);
	return 0; 
}

void *func_kids(void * id){

	srand(time(NULL));
	//run forever until thread gets cancled 
	while(true){
		candy_t* candy = bbuff_blocking_extract();
		if (candy) {
			double tempTime = (current_time_in_ms() - candy->time_stamp_in_ms);
			stats_record_consumed(candy->factory_number, tempTime);
			free(candy);
		}
		sleep(rand()%2);
	}
	
	return 0;
}


int main(int argc, const char * argv[]){

	//1. Extract arguments-----------------------------------------
	
	if (argc <4) { 
		printf ("Error: Please provide enough arguments.\n");
		exit(0);
	}

	int factories = strToInt(argv[1]);
	int kids = strToInt(argv[2]);
	int seconds = strToInt(argv[3]);

	if (factories <= 0 || kids <= 0 || seconds <= 0){
		printf ("Error: Please provide correct arguments.\n");
		exit(0);
	}

	//2. Initialize stats and bounded buffer modules-------------------------------
	stats_init(factories);
	bbuff_init();
	//Fomating print to match proffesor output 
	printf("Time 0s:\n");

	//3. Launching Factories threads-----------------------------------------------------
	int retFactory = -1;
	pthread_t threadsFactories[factories];
	int t_FactoriesIds[factories]; 

	for (int i = 0; i < factories; i++) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		t_FactoriesIds[i] = i;
	    retFactory = pthread_create(&threadsFactories[i], &attr, &func_factories, (void *)&t_FactoriesIds[i]);
	    if(retFactory != 0) {
	        printf ("Create pthread error!\n");
	        exit(1);
	    }
	}	

	//4 . Launching kid threads--------------------------------------------
	int retKid = -1;
	pthread_t threadsKids[kids];

	for (int i = 0; i < kids; i++) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
	    retKid = pthread_create(&threadsKids[i], &attr, &func_kids, NULL);
	    if(retKid != 0) {
	        printf ("Create pthread error!\n");
	        exit(1);
	    }
	}	
	
	//5. Wait for requested time-------------------------------------------
	for (int i = 1; i < seconds; i++){
		sleep(1);
		printf("Time %ds:\n", i);
	}

	//6. Stop factory threads------------------------------------ 
	stop_thread = true;
	printf ("Stopping candy factories...\n");
	for(int i=0; i < factories; i++){
		pthread_join(threadsFactories[i], NULL);
	}

	//7. Wait until no more candy-------------------------------------------
	while (bbuff_is_empty() == false){
		printf("Waiting for all candy to be consumed\n");
		sleep(1);
	}

	//8. Stop kids threads-------------------------------------------
	printf ("Stopping kids.\n");
	for(int i=0; i < kids; i++){
		pthread_cancel(threadsKids[i]);
		pthread_join(threadsKids[i], NULL);
	}

	//9. Print statistics -------------------------------------------
	printf ("Statistics:\n");
	stats_display();

	//10. Cleanup any allocated memory--------------------------------------
	stats_cleanup();

    return 0;
}
