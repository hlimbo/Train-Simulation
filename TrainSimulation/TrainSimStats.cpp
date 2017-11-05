#include "TrainSimStats.h"
#include <iostream>

TrainSimStats::TrainSimStats()
{
	totalTrainsServed = 0;
	totalTrainTimeInSystem = 0.0f;
	avgTimeInSystem = 0.0f;
	maxTimeInSystem = 0.0f;

	trainLineSum = 0;
	timeAverage = 0.0f;
	maxTrainsInQueue = 0;

	timeSpentBusy = 0.0f;
	timeSpentIdle = 0.0f;
	timeSpentHoggedOut = 0.0f;
	timeSpentHoggedOutInQueue = 0.0f;
	currentTime = 0.0f;


	timeSpentBusyPercent = 0.0f;
	timeSpentIdlePercent = 0.0f;
	timeSpentHoggedOutPercent = 0.0f;
	timeSpentHoggedOutInQueuePercent = 0.0f;

	expected_total_percentage = 1.0f;
	actual_total_percentage = 0.0f;


	r = 0.0f;
	u = 0.0f;
	t = 0.0f;
	crewID = 0;
	trainID = 0;

	max_sim_time = 0.0f;
	
	observedAvgArrivalTime = 0.0f;
	trainEnterSystemCount = 0;
}

TrainSimStats::~TrainSimStats()
{
}

void TrainSimStats::DisplayHistogram()
{
	std::cout << "# of hogouts : # of trains" << std::endl;
	for(std::map<int,int>::iterator it = hogOutHistogram.begin();it != hogOutHistogram.end();++it)
	{
		std::cout << it->first << " : " << it->second << std::endl;
	}

	std::cout << std::endl;
}