#include "bbuff.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>

void* candyBuff[BUFFER_SIZE];

sem_t empty;
sem_t full;
sem_t mutex;
int index, tracker, outIndex = 0;

void bbuff_init(){
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);
	sem_init(&mutex, 0, 1);
}

void bbuff_blocking_insert(void* item){
	
	sem_wait(&empty);
	sem_wait(&mutex);
	tracker++;
	candyBuff[index] = item;
	index = (index+1) % BUFFER_SIZE;
	sem_post(&mutex);
	sem_post(&full);	
}

void* bbuff_blocking_extract(void){	
	
	void* candyPtr = NULL;
	sem_wait(&full); 
	sem_wait(&mutex);
	tracker--;
	candyPtr = candyBuff[outIndex];
	candyBuff[outIndex] = NULL;
	outIndex = (outIndex+1)%BUFFER_SIZE;
	sem_post(&mutex);
	sem_post(&empty);  
	return candyPtr;

}

_Bool bbuff_is_empty(void){
	_Bool flag = false;
	sem_wait(&mutex);
	sem_getvalue(&empty, &index );
	if (index == BUFFER_SIZE) flag= true; 
	sem_post(&mutex);
	return flag;
}
