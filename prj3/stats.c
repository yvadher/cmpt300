#include "stats.h"
#include <float.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
	int candyMade;
	int candyEaten;
	double totalDelay;
	double maxDelay;
	double minDelay;
} factoryStats;

factoryStats* stats;
int producers;

void stats_init(int producersNo){
	producers = producersNo;
	stats = malloc(sizeof(factoryStats)*producersNo);
	
	for (int i = 0; i < producersNo; i++)
	{
		stats[i].candyMade = 0;
		stats[i].candyEaten = 0;
		stats[i].totalDelay = 0;
		stats[i].maxDelay= 0;
		stats[i].minDelay= DBL_MAX;
	}
}

void stats_cleanup(void){
	if (stats) free(stats);
}

void stats_record_produced(int id){
	stats[id].candyMade++;
}

void stats_record_consumed(int id, double delayMs){
	stats[id].candyEaten++;
	stats[id].totalDelay = stats[id].totalDelay + delayMs;

	//Check if the delay is maximum or minumum 
	
	if (delayMs > stats[id].maxDelay) stats[id].maxDelay = delayMs;
	if (delayMs < stats[id].minDelay) stats[id].minDelay = delayMs;
}

void stats_display(void){
	
	_Bool flag = true;
	double avgDelay[producers]; 

	for (int i = 0; i <producers; i++)
	{
		if (stats[i].minDelay == DBL_MAX) stats[i].minDelay = 0;
		if (stats[i].candyEaten != 0) avgDelay[i] = (stats[i].totalDelay) / (stats[i].candyEaten);
		else avgDelay[i] = 0;
	}

	//Title Row
	printf("%8s%10s%30s%30s%30s%30s\n", "Factory#", "#Made", "#Eaten", "Min Delay[ms]", "Avg Delay[ms]", "Max Delay[ms]");
	//Data Row
	for (int i = 0; i<producers; i++)
	{
		printf("%8d%10d%30d%30f%30f%30f\n", i, stats[i].candyMade, stats[i].candyEaten, stats[i].minDelay, avgDelay[i], stats[i].maxDelay);
	}
	for (int i = 0; i < producers; i++)
	{
		if (stats[i].candyMade != stats[i].candyEaten){
			flag = false;
			break;
		}
	}
	
	if (!flag) printf("Error: This should not happen!\n");
	
}