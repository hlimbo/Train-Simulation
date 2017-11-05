#include "SimEventHandler.h"
#include "TrainStationObjects.h"
#include "TrainStation.h"


SimEventHandler::SimEventHandler()
{
}

SimEventHandler::SimEventHandler(TrainStation* trainStation)
{
	this->trainStation = trainStation;
}

SimEventHandler::~SimEventHandler()
{
}

int SimEventHandler::GetSize()
{
	return pq.size();
}

bool SimEventHandler::IsEmpty()
{
	return pq.empty();
}

void SimEventHandler::Push(SimEvent* newEvent)
{
		pq.push(newEvent);

}

SimEvent* SimEventHandler::Pop()
{
	if(!pq.empty())
	{
		SimEvent* simEvent = pq.top();
		pq.pop();
		return simEvent;
	}

	return nullptr;
}

SimEvent* SimEventHandler::Top()
{
	if(pq.empty())
		return nullptr;

	return pq.top();
}

void SimEventHandler::Schedule(SimEvent* simEvent,Entity* entity, float timeStamp)
{
	simEvent->setOwner(entity);
	simEvent->setEventHandler(this);
	simEvent->setTimeStamp(timeStamp);
	Push(simEvent);
}

void SimEventHandler::CleanUp()
{
	while(!IsEmpty())
	{
		SimEvent* simEvent = Pop();
		delete simEvent;
	}
}

