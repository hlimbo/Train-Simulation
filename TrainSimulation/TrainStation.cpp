#include "TrainStation.h"
#include "TrainStationObjects.h"

TrainStation::TrainStation()
{
	dockedTrain = nullptr;
	state = StationStates::IDLE;
	startIdleTime = 0.0f;
	startBusyTime = 0.0f;
	startHoggedOutTime = 0.0f;
	startHoggedOutTimeInQueue = 0.0f;
}

TrainStation::~TrainStation()
{
}

Train* TrainStation::PopFromQueue()
{
	if(!trainLineQueue.empty())
	{
		Train* train = trainLineQueue.front();
		trainLineQueue.pop();
		return train;
	}

	return nullptr;
}

void TrainStation::PushToQueue(Train* train)
{
	trainLineQueue.push(train);
}

Train* TrainStation::Front()
{
	if(!trainLineQueue.empty())
		return trainLineQueue.front();

	return nullptr;
}

bool TrainStation::IsEmpty()
{
	return trainLineQueue.empty();
}

Train* TrainStation::getDockedTrain()
{
	return dockedTrain;
}

void TrainStation::RemoveTrains()
{
	if(dockedTrain != nullptr)
	{
		delete dockedTrain;
		dockedTrain = nullptr;
	}

	while(!IsEmpty())
	{
		Train* train = PopFromQueue();
		delete train;
	}

	while(!trainsToArriveQueue.empty())
	{
		Train* train = trainsToArriveQueue.front();
		trainsToArriveQueue.pop();
		delete train;
	}
}

void TrainStation::RemoveReplacementCrews()
{

	while(!replacementCrews.empty())
	{
		Crew* crew = replacementCrews.front();
		replacementCrews.pop();
		delete crew;
	}
}