#ifndef TRAIN_SIM_STATS
#define TRAIN_SIM_STATS
#include <map>


class TrainSimStats
{
public:
	int totalTrainsServed;
	float totalTrainTimeInSystem;
	float avgTimeInSystem;
	float maxTimeInSystem;
	

	//The total number of trains in queue per TrainDepartureEvent call.
	int trainLineSum;
	float timeAverage;
	unsigned int maxTrainsInQueue;

	float timeSpentBusy;
	float timeSpentIdle;
	//time spent Hogged Out on Loading Dock.
	float timeSpentHoggedOut;

	float timeSpentHoggedOutInQueue;

	float timeSpentBusyPercent;
	float timeSpentIdlePercent;
	float timeSpentHoggedOutPercent;
	float timeSpentHoggedOutInQueuePercent;

	float expected_total_percentage;
	float actual_total_percentage;

	std::map<int,int> hogOutHistogram;

	float currentTime;
	float prevTime;

	float max_sim_time;

	float observedAvgArrivalTime;
	int trainEnterSystemCount;

	//IDs generated in increasing order.
	int crewID;
	int trainID;

	//average train arrival time.
	float r;
	//uniformly distributed number between 0 and 1.
	float u;
	//interarrival time.
	float t;

	TrainSimStats();
	~TrainSimStats();


	void DisplayHistogram();
};


#endif