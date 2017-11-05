#include "TrainStation.h"
#include "TrainStationObjects.h"
#include "TrainSimEvents.h"
#include "SimEventHandler.h"
#include "TrainSimStats.h"
#include "RNGUtils.h"
#include <iostream>
#include <stdlib.h>
#include <functional>

using namespace std;

//Note: I am using std::C++11 to compile this assignment.

//In the simulation a train can hog out at:
/*
	1. The end of its service
	2. The start of its service
	3. Exit of Queue
*/

int main(int argc,char** argv)
{
	TrainStation trainStation;
	SimEventHandler simEventHandler(&trainStation);
	TrainSimStats trainStats;

	trainStats.r = 10.0f;//temp code
	trainStats.max_sim_time = MAX_SIM_TIME;//tempcode

	//linux code.
	/*if(argc < 2)
	{
		trainStats.r = 10;
		trainStats.max_sim_time = MAX_SIM_TIME;
	}
	else if(argc == 2)
	{
		trainStats.r = atof(argv[1]);
		trainStats.max_sim_time = atof(argv[2]);
	}
	else
	{
		cout << "Error: Too many arguments passed" << endl;
		return -1;
	}*/
	
	trainStats.u = dist(rand_device);
	trainStats.t = -trainStats.r * log(1 - trainStats.u); 
	
	Crew* crew = new Crew(trainStats.crewID);
	Train* train = new Train(trainStats.trainID,crew);
	
	train->arrivalTimeDuration = trainStats.t;
	train->startArrivalTime = trainStats.currentTime + train->arrivalTimeDuration;

	TrainArrivalEvent* initialEvent = new TrainArrivalEvent();
	EndSimEvent* endEvent = new EndSimEvent();

	trainStation.trainsToArriveQueue.push(train);

	
	simEventHandler.Schedule(initialEvent,train, train->startArrivalTime);
	simEventHandler.Schedule(endEvent,nullptr, trainStats.max_sim_time);

	//using the command pattern to delegate events (all events derive from SimEvent).
	while(trainStats.currentTime < trainStats.max_sim_time)
	{
		SimEvent* simEvent = simEventHandler.Pop();
		trainStats.prevTime = trainStats.currentTime;
		trainStats.currentTime = simEvent->getTimeStamp();
		simEvent->execute(&trainStats);
		delete simEvent;
	}

	//deletes events that are scheduled at time >= max_sim_time.
	simEventHandler.CleanUp();
	//deletes trains that were scheduled to arrive at time >= max_sim_time.
	trainStation.RemoveTrains();
	//deletes crews who where scheduled to arrive to their trains at time >= max_sim_time.
	trainStation.RemoveReplacementCrews();

 //visual studio stuff ~ remove after submitting final build. 
	system("PAUSE");


	return 0;
}

