#ifndef SIM_EVENT_HANDLER
#define SIM_EVENT_HANDLER

#include <queue>
#include "TrainSimEvents.h"

class TrainStation;

//Class responsible for holding and managing the event list.
class SimEventHandler
{
public:
	SimEventHandler();
	SimEventHandler(TrainStation* trainStation);
	~SimEventHandler();

	int GetSize();
	bool IsEmpty();
	void Push(SimEvent* newEvent);
	SimEvent* Pop();
	SimEvent* Top();

	void Schedule(SimEvent* simEvent,Entity* entity, float timeStamp);

	//when there are events that get scheduled beyond the MAX_SIM_TIME, 
	//we want to delete:
	//1. The entities implicitly or explicitly attached to the event.
	//2. delete the event itself.
	//Called at the End of the Simulation to free memory obtained from the heap.
	void CleanUp();

public:
	std::priority_queue<SimEvent*,std::vector<SimEvent*>,Compare > pq;//event-list
	TrainStation* trainStation;
};

#endif