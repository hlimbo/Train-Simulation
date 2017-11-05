#include "TrainSimEvents.h"
#include "SimEventHandler.h"
#include "TrainStationObjects.h"
#include "TrainSimStats.h"
#include "RNGUtils.h"
#include "TrainStation.h"
#include <iostream>

//-------PriorityQueue Comparison Function: Used to sort events by arrival time.

bool Compare::operator()(SimEvent* s1, SimEvent* s2)
{
	return s1->getTimeStamp() > s2->getTimeStamp();
}


//SimEvent------------------------------------------

SimEvent::SimEvent()
{
	timeStamp = -1.0f;
	eventHandler = nullptr;
	owner = nullptr;
	name = "SimEvent";
}

SimEvent::~SimEvent()
{
	//shouldn't delete the owner of this event~ entities can persist over events!
	//entity only gets destroyed when departure event is called.
}

void SimEvent::execute(TrainSimStats* stats){}

void SimEvent::setTimeStamp(float newTimeStamp)
{ 
	timeStamp = newTimeStamp;
}
float SimEvent::getTimeStamp()
{ 
	return timeStamp;
}

Entity* SimEvent::getOwner()
{
	return owner;
}

void SimEvent::setOwner(Entity* newOwner)
{
	owner = newOwner;
}

SimEventHandler* SimEvent::getEventHandler()
{
	return eventHandler;
}

void SimEvent::setEventHandler(SimEventHandler* eventHandler)
{
	this->eventHandler = eventHandler;
}


//TrainArrivalEvent------------------------------------------------

TrainArrivalEvent::TrainArrivalEvent() : SimEvent()
{
	name = "TrainArrivalEvent";
}
TrainArrivalEvent::~TrainArrivalEvent() {}

void TrainArrivalEvent::execute(TrainSimStats* stats)
{
	executeTrainArrivalEvent(owner,stats);
}

void TrainArrivalEvent::executeTrainArrivalEvent(Entity* owner, TrainSimStats* stats)//works
{
	Train* currentTrain = dynamic_cast<Train*>(owner);
	if(currentTrain)
	{
		//to verify if exponential distribution i used is correct
		stats->trainEnterSystemCount++;

		float unloadTimeDuration = unload_time_distribution(generator);
		float workTimeDuration = crew_work_time_distribution(generator);
		
		currentTrain->unloadTimeDuration = unloadTimeDuration;
		currentTrain->workingCrew->unloadTimeDuration = unloadTimeDuration;
		currentTrain->unloadTimeLeft = unloadTimeDuration;
		currentTrain->workingCrew->unloadTimeLeft = unloadTimeDuration;
		currentTrain->workingCrew->workTimeDuration = workTimeDuration;
		currentTrain->workingCrew->workTimeLeft = workTimeDuration;
		currentTrain->workingCrew->startArrivalTime = currentTrain->startArrivalTime;
		currentTrain->workingCrew->startWorkTime = currentTrain->startArrivalTime;
		currentTrain->workingCrew->arrivalTimeDuration = 0.0f;
		currentTrain->workingCrew->isReplacementCrew = false;
		currentTrain->workingCrew->assignedTrain = currentTrain;

		TrainEnterQEvent* enterQEvent = new TrainEnterQEvent();
		eventHandler->Schedule(enterQEvent,currentTrain,currentTrain->startArrivalTime);

		std::cout << "Time " << stats->currentTime << ": train " << owner->ID << " arrival for " << unloadTimeDuration << "h of unloading;" << " crew " << currentTrain->workingCrew->ID << " with " << workTimeDuration << "h" << " before hogout, Q=" << eventHandler->trainStation->trainLineQueue.size() << std::endl;		
	}
	else
		std::cout << "owner failed to cast to train!" << std::endl;


	//schedule the next train to arrive!
	Crew* nextCrew = new Crew(++stats->crewID);
	Train* nextTrain = new Train(++stats->trainID,nextCrew);

	TrainArrivalEvent* nextArrivalEvent = new TrainArrivalEvent();

	stats->u = dist(rand_device);
	stats->t = -stats->r * log(1 - stats->u);	
	nextTrain->arrivalTimeDuration = stats->t;
	nextTrain->startArrivalTime = stats->currentTime + nextTrain->arrivalTimeDuration;

	//Crew is already on the train, no need to wait for them to arrive.
	nextCrew->arrivalTimeDuration = 0.0f;


	stats->observedAvgArrivalTime += stats->t;
	std::cout << "Arrival Time: " << stats->t << std::endl;


	eventHandler->Schedule(nextArrivalEvent,nextTrain, nextTrain->startArrivalTime);
	eventHandler->trainStation->trainsToArriveQueue.push(nextTrain);


}

//TrainDepartureEvent---------------------------------------------------

TrainDepartureEvent::TrainDepartureEvent() : SimEvent() 
{
	name = "TrainDepartureEvent";
}
TrainDepartureEvent::~TrainDepartureEvent() {}

void TrainDepartureEvent::execute(TrainSimStats* stats)
{
	executeTrainDepartureEvent(owner,stats);
}

//called from TrainEndService;
void TrainDepartureEvent::executeTrainDepartureEvent(Entity* owner, TrainSimStats* stats)
{

	float timeInSystem = stats->currentTime - owner->startArrivalTime;
	stats->totalTrainTimeInSystem += timeInSystem;

	if(timeInSystem > stats->maxTimeInSystem)
		stats->maxTimeInSystem = timeInSystem;

	//stats->trainLineSum += eventHandler->trainStation->trainLineQueue.size();

	//std::cout << "TrainDepartureEvent::execute(): " << std::endl;// << "Time " << stats->currentTime << ": Train " << owner->ID << std::endl;
	//std:: cout << "Time IN system -> " << timeInSystem  << std::endl;


	Train* train = eventHandler->trainStation->dockedTrain;
	if(train)
	{
		//Time 3.72: train 0 departing, Q=1
		std::cout << "Time " << stats->currentTime << ": train " << train->ID << " departing, Q=" << eventHandler->trainStation->trainLineQueue.size() << std::endl;

		eventHandler->trainStation->dockedTrain = nullptr;
		eventHandler->trainStation->state = StationStates::IDLE;
		eventHandler->trainStation->startIdleTime = stats->currentTime;

		stats->hogOutHistogram[train->hogOutCount]++;

		stats->totalTrainsServed++;
		if(eventHandler->trainStation->trainLineQueue.size() > stats->maxTrainsInQueue)
		{
			stats->maxTrainsInQueue = eventHandler->trainStation->trainLineQueue.size();
		}

		delete train;
	}
	else
	{
		std::cout << "There is no docked train in the station" << std::endl;
	}
}

//TrainEnterQEvent-----------------------------------------------------------
TrainEnterQEvent::TrainEnterQEvent() 
{
	name = "TrainEnterQEvent";
}
TrainEnterQEvent::~TrainEnterQEvent() {}

void TrainEnterQEvent::execute(TrainSimStats* stats)
{
	executeTrainEnterQ(owner,stats);
}


//called from executeTrainArrival
void TrainEnterQEvent::executeTrainEnterQ(Entity* owner,TrainSimStats* stats)
{
	//removes the next train that had just arrived at the station.
	//Needs to be done to prevent deletion of the same train object twice after the end of simulation.
	eventHandler->trainStation->trainsToArriveQueue.pop();

	//std::cout << "TrainEnterQEvent::execute(): " << "Time " << stats->currentTime << ": Train " << owner->ID <<  std::endl;

	Train* train = dynamic_cast<Train*>(owner);
	if(train)
	{
		train->startWaitTime = stats->currentTime;
		
		//shortcut.
		if(eventHandler->trainStation->IsEmpty() 
			&& eventHandler->trainStation->dockedTrain == nullptr
			&& eventHandler->trainStation->state == StationStates::IDLE)
		{

			TrainExitQEvent* exitQEvent = new TrainExitQEvent();
			eventHandler->Schedule(exitQEvent,owner, stats->currentTime);
		}
		else
		{
			eventHandler->trainStation->PushToQueue(train);
			//stats->trainLineSum += eventHandler->trainStation->trainLineQueue.size();
		}
	}
	else
	{
		std::cout << "TrainEnterQEvent::execute(): failed to convert entity to train" << std::endl;
	}
}

//TrainExitQEvent--------------------------------------------------------------------------
TrainExitQEvent::TrainExitQEvent()
{
	name = "TrainExitQEvent";
}
TrainExitQEvent::~TrainExitQEvent() {}

void TrainExitQEvent::execute(TrainSimStats* stats)
{
	executeTrainExitQ(owner,stats);
}

//called from executeTrainEnterQ or executeTrainDeparture
void TrainExitQEvent::executeTrainExitQ(Entity* owner, TrainSimStats* stats)
{

	//std::cout << "TrainExitQEvent::execute() " << "Time " << stats->currentTime << ": Train " << owner->ID <<  std::endl;

	Train* train = dynamic_cast<Train*>(owner);
	if(train == 0)
		std::cout << "TrainExitQEvent::executeTrainExitQ(): owner failed to convert to train" << std::endl;
	else
	{
		
		train->waitTimeDuration = stats->currentTime - train->startWaitTime;
	

		if(!train->workingCrew->isReplacementCrew)
			train->workingCrew->workTimeLeft -= train->waitTimeDuration;
		else
			train->workingCrew->workTimeLeft -= train->workingCrew->arrivalTimeDuration;

		float workTimeLeft = train->workingCrew->workTimeLeft;

		if(workTimeLeft > 0.0f)
		{
						
			stats->trainLineSum += eventHandler->trainStation->trainLineQueue.size() * train->waitTimeDuration;

			//Train popped is the parameter Entity* owner passed. 
			//Remove train in front of line to ensure it gets serviced in the next event.
			eventHandler->trainStation->PopFromQueue();


			TrainStartService* startServiceEvent = new TrainStartService();
			eventHandler->Schedule(startServiceEvent,train, stats->currentTime);
		}
		else //if workingCrew hogged out
		{
			eventHandler->trainStation->startHoggedOutTimeInQueue = stats->currentTime;
			CrewDepartureEvent2* departureEvent2 = new CrewDepartureEvent2();
			eventHandler->Schedule(departureEvent2,train->workingCrew,stats->currentTime);
		}
	}

}


//TrainStartService----------------------------------------------------

TrainStartService::TrainStartService() : SimEvent()
{
	name = "TrainStartService";
}
TrainStartService::~TrainStartService(){}

void TrainStartService::execute(TrainSimStats* stats)
{
	executeTrainStartService(owner,stats);	
}

//called from TrainExitQEvent or CrewArrivalEvent4
void TrainStartService::executeTrainStartService(Entity* owner, TrainSimStats* stats)
{
	//std::cout << "TrainStartService::execute() " << "Time " << stats->currentTime << ": Train " << owner->ID <<  std::endl;
	
	Train* train = dynamic_cast<Train*>(owner);
	if(train == 0)
		std::cout << "TrainStartService::execute(): owner failed to convert to train" << std::endl;
	else
	{
		train->workingCrew->workTimeLeft -= stats->currentTime - stats->prevTime;		
				
		float workTimeLeft = train->workingCrew->workTimeLeft;
		if(workTimeLeft < train->workingCrew->unloadTimeLeft)//check for hogout
		{
			eventHandler->trainStation->startHoggedOutTime = stats->currentTime;
			eventHandler->trainStation->dockedTrain = train;
			eventHandler->trainStation->dockedTrain->unloadTimeLeft -= workTimeLeft;
			eventHandler->trainStation->dockedTrain->workingCrew->unloadTimeLeft -= workTimeLeft;
			eventHandler->trainStation->state = StationStates::IDLE;

			CrewDepartureEvent4* departureEvent = new CrewDepartureEvent4();
			eventHandler->Schedule(departureEvent,train->workingCrew,stats->currentTime);
		}
		else
		{
			if(!train->workingCrew->isReplacementCrew)
				stats->timeSpentIdle = stats->timeSpentIdle + (stats->currentTime - eventHandler->trainStation->startIdleTime);
			
			train->startUnloadTime = stats->currentTime;
			train->workingCrew->startUnloadTime = stats->currentTime;
		
			eventHandler->trainStation->dockedTrain = train;
			eventHandler->trainStation->state = StationStates::BUSY;

			eventHandler->trainStation->startBusyTime = stats->currentTime;
		
			//Time 0.00: train 0 entering dock for 3.72h of unloading, crew 0 with 7.80h before hogout
			std::cout << "Time " << stats->currentTime << ": train " << train->ID << " entering dock for " << train->unloadTimeDuration << "h of unloading; crew " << train->workingCrew->ID << " with " <<  train->workingCrew->workTimeLeft << "h before hogout" << std::endl;

			TrainEndService* endServiceEvent = new TrainEndService();
			eventHandler->Schedule(endServiceEvent,train, stats->currentTime + train->unloadTimeDuration);	
		}
	}

}

//TrainEndServiceEvent-----------------------------------------

TrainEndService::TrainEndService() : SimEvent()
{
	name = "TrainEndService";
}
TrainEndService::~TrainEndService() {}

void TrainEndService::execute(TrainSimStats* stats)
{
	executeTrainEndService(owner,stats);
}

//called from TrainStartService
void TrainEndService::executeTrainEndService(Entity* owner, TrainSimStats* stats)
{
	//std::cout << "TrainEndService::execute() " << "Time " << stats->currentTime << ": Train " << owner->ID <<  std::endl;

	Train* train = dynamic_cast<Train*>(owner);
	if(train)
	{

		train->workingCrew->workTimeLeft -= train->workingCrew->unloadTimeLeft;
		float workTimeLeft = train->workingCrew->workTimeLeft;

		//check if train is hogged out at this time!
		if(workTimeLeft <= 0.0f)
		{
			CrewDepartureEvent3* crewDepartureEvent = new CrewDepartureEvent3();
			eventHandler->Schedule(crewDepartureEvent,train->workingCrew,stats->currentTime);
		}
		else
		{
			train->departureTime = stats->currentTime;
			float timeSpentBusy;
			if(!train->workingCrew->isReplacementCrew)
			{
				timeSpentBusy = train->departureTime - train->startUnloadTime;
			}
			else
			{
				timeSpentBusy = train->departureTime - train->workingCrew->startArrivalTime;
			}

			stats->timeSpentBusy += timeSpentBusy;

			TrainDepartureEvent* departureEvent = new TrainDepartureEvent();
			eventHandler->Schedule(departureEvent,train,train->departureTime);

			//schedule another train to pull up to the loading dock if there is one in front of the queue.
			if(!eventHandler->trainStation->IsEmpty())
			{
				TrainExitQEvent* exitQEvent = new TrainExitQEvent();
				//Careful, we must check if the crew in the next train is hogged out!
				Train* nextTrain = eventHandler->trainStation->Front();
				eventHandler->Schedule(exitQEvent,nextTrain,stats->currentTime);
			}
		}
		
	}
}


//CrewArrivalEvent2---------------------------------------------------------------
CrewArrivalEvent2::CrewArrivalEvent2() : SimEvent()
{
	name = "CrewArrivalEvent2";
}

CrewArrivalEvent2::~CrewArrivalEvent2(){}

void CrewArrivalEvent2::execute(TrainSimStats* stats)
{
	executeCrewArrival2(owner,stats);
}

void CrewArrivalEvent2::executeCrewArrival2(Entity* owner, TrainSimStats* stats)
{
	Crew* newCrew = dynamic_cast<Crew*>(owner);
	if(newCrew)
	{
		std::cout << "Time " << stats->currentTime << ": train " << newCrew->assignedTrain->ID << ": replacement crew " << newCrew->ID << " arrives, SERVER UNHOGGED" << std::endl; 

		//if the time the replacement crew arrives the train is BEFORE MAX_SIM_TIME
		//then we want to remove the replacmentCrew from the queue to prevent
		//the same pointer to the replacmentCrew from being deleted twice.
		eventHandler->trainStation->replacementCrews.pop();

		//stats->timeSpentHoggedOut += stats->currentTime - eventHandler->trainStation->startHoggedOutTime;
		stats->timeSpentHoggedOutInQueue += stats->currentTime - eventHandler->trainStation->startHoggedOutTimeInQueue;

		Crew* oldCrew = newCrew->assignedTrain->workingCrew;
		delete oldCrew;
		
		//attach newCrew to designated train as working crew.
		newCrew->assignedTrain->workingCrew = newCrew;
		newCrew->assignedTrain->isHoggedOut = false;

		//book keeping
		newCrew->startArrivalTime = stats->currentTime;

		//newCrew->workTimeLeft = newCrew->workTimeLeft - newCrew->arrivalTimeDuration;

		//schedule another event
		TrainExitQEvent* exitQEvent = new TrainExitQEvent();
		eventHandler->Schedule(exitQEvent,newCrew->assignedTrain,newCrew->startArrivalTime);

		//test
	}
	else
	{
		std::cout << "CrewArrivalEvent2::execute() Failed to cast Entity owner to Crew" << std::endl;
	}
}


//CrewDepartureEvent2---------------------------------------------------------------
CrewDepartureEvent2::CrewDepartureEvent2() : SimEvent()
{
	name = "CrewDepartureEvent2";
}

CrewDepartureEvent2::~CrewDepartureEvent2(){}

void CrewDepartureEvent2::execute(TrainSimStats* stats)
{
	executeCrewDeparture2(owner,stats);
}
void CrewDepartureEvent2::executeCrewDeparture2(Entity* owner,TrainSimStats* stats)
{
	//std::cout << "CrewDepartureEvent2::execute() Crew " << owner->ID << std::endl;

	Crew* crew = dynamic_cast<Crew*>(owner);
	if(crew)
	{
		std::cout << "2";
		std::cout << "Time " << stats->currentTime << ": train " << crew->assignedTrain->ID << ": crew " << crew->ID << " hogged out during service; SERVER HOGGED" << std::endl;

		crew->assignedTrain->isHoggedOut = true;
		crew->assignedTrain->hogOutCount++;
		
		//request for a replacement crew to arrive
		Crew* newCrew = new Crew(++stats->crewID);		
		newCrew->isReplacementCrew = true;	
		//attach ourselves to the train that requested for us to know which train replacement crew goes to.
		newCrew->assignedTrain = crew->assignedTrain;
		newCrew->arrivalTimeDuration = crew_arrival_time_distribution(generator);
		newCrew->workTimeDuration = crew_work_time_distribution(generator);
		newCrew->workTimeLeft = newCrew->workTimeDuration;
		newCrew->startWorkTime = stats->currentTime;
		newCrew->unloadTimeDuration = crew->unloadTimeDuration;
		newCrew->unloadTimeLeft = crew->unloadTimeLeft;
				

		//if the time the replacement crew arrives the train exceeds MAX_SIM_TIME, 
		//then we want to free up their resources after the simulation is over using trainStation.RemoveReplacmentCrews();
		eventHandler->trainStation->replacementCrews.push(newCrew);
		
		CrewArrivalEvent2* arrivalEvent = new CrewArrivalEvent2();
		eventHandler->Schedule(arrivalEvent,newCrew,newCrew->startWorkTime + newCrew->arrivalTimeDuration);

	}
	else
	{
		std::cout << "CrewDepartureEvent2::execute() Failed to cast Entity owner to Crew" << std::endl;
	}
}

//CrewArrivalEvent3-------------------------------------------------------------------
CrewArrivalEvent3::CrewArrivalEvent3() : SimEvent()
{
	name ="CrewArrivalEvent3";
}
CrewArrivalEvent3::~CrewArrivalEvent3() {}

void CrewArrivalEvent3::execute(TrainSimStats* stats)
{
	executeCrewArrival3(owner,stats);
}
void CrewArrivalEvent3::executeCrewArrival3(Entity* owner, TrainSimStats* stats)
{

	//std::cout << "CrewArrivalEvent3::execute()  newCrew " << owner->ID << std::endl;
	Crew* newCrew = dynamic_cast<Crew*>(owner);
	if(newCrew)
	{
		//if the time the replacement crew arrives the train is BEFORE MAX_SIM_TIME
		//then we want to remove the replacmentCrew from the queue to prevent
		//the same pointer to the replacmentCrew from being deleted twice.
		eventHandler->trainStation->replacementCrews.pop();

		stats->timeSpentHoggedOut += stats->currentTime - eventHandler->trainStation->startHoggedOutTime;

		std::cout << "3";
		std::cout << "Time " << stats->currentTime << ": train " << newCrew->assignedTrain->ID << ": replacement crew " << newCrew->ID << " arrives, SERVER UNHOGGED" << std::endl; 

		Crew* oldCrew = newCrew->assignedTrain->workingCrew;
		delete oldCrew;
		newCrew->assignedTrain->workingCrew = newCrew;
		newCrew->assignedTrain->isHoggedOut = false;
		newCrew->workTimeLeft -= stats->currentTime - stats->prevTime;
		newCrew->startArrivalTime = stats->currentTime;
		
	
		TrainEndService* endServiceEvent = new TrainEndService();
		eventHandler->Schedule(endServiceEvent,newCrew->assignedTrain,stats->currentTime);
	}
	else
	{
		std::cout << "CrewArrivalEvent3::execute() Failed to cast Entity owner to Crew" << std::endl;
	}
}




//CrewDepartureEvent3--------------------------------------------------------------------
CrewDepartureEvent3::CrewDepartureEvent3() : SimEvent()
{
	name = "CrewDepartureEvent3";
}

CrewDepartureEvent3::~CrewDepartureEvent3() {}

void CrewDepartureEvent3::execute(TrainSimStats* stats)
{
	executeCrewDeparture3(owner,stats);
}
void CrewDepartureEvent3::executeCrewDeparture3(Entity* owner, TrainSimStats* stats)
{
	//std::cout << "CrewDepartureEvent3::execute() oldCrew " << owner->ID << std::endl;
	Crew* oldCrew = dynamic_cast<Crew*>(owner);
	if(oldCrew)
	{
		std::cout<< "3";
		std::cout << "Time " << stats->currentTime << ": train " << oldCrew->assignedTrain->ID << ": crew " << oldCrew->ID << " hogged out during service; SERVER HOGGED" << std::endl;


		oldCrew->assignedTrain->isHoggedOut = true;
		oldCrew->assignedTrain->hogOutCount++;

		//schedule arrival event
		Crew* newCrew = new Crew(++stats->crewID);
		newCrew->isReplacementCrew = true;
		newCrew->assignedTrain = oldCrew->assignedTrain;
		newCrew->arrivalTimeDuration = crew_arrival_time_distribution(generator);
		newCrew->startWorkTime = stats->currentTime;
		newCrew->workTimeDuration = crew_work_time_distribution(generator);
		newCrew->workTimeLeft = newCrew->workTimeDuration;
		newCrew->unloadTimeDuration = oldCrew->unloadTimeDuration;

		//if the time the replacement crew arrives the train exceeds MAX_SIM_TIME, 
		//then we want to free up their resources after the simulation is over using trainStation.RemoveReplacmentCrews();
		eventHandler->trainStation->replacementCrews.push(newCrew);
	
		CrewArrivalEvent3* arrivalEvent = new CrewArrivalEvent3();
		eventHandler->Schedule(arrivalEvent,newCrew,newCrew->startWorkTime + newCrew->arrivalTimeDuration);

	}
	else
	{
		std::cout << "CrewDepartureEvent3::execute() failed to cast Entity owner to Crew" << std::endl;
	}
}

//CrewArrivalEvent4-------------------------------------------------------------------------------------
CrewArrivalEvent4::CrewArrivalEvent4() : SimEvent()
{
	name = "CrewArrivalEvent4";
}
CrewArrivalEvent4::~CrewArrivalEvent4() {}

void CrewArrivalEvent4::execute(TrainSimStats* stats)
{
	executeCrewArrival4(owner,stats);
}
void CrewArrivalEvent4::executeCrewArrival4(Entity* owner, TrainSimStats* stats)
{
	//std::cout << "CrewArrivalEvent4::execute() Crew " << owner->ID << std::endl;
	Crew* newCrew = dynamic_cast<Crew*>(owner);
	if(newCrew)
	{
		std::cout << "Time " << stats->currentTime << ": train " << newCrew->assignedTrain->ID << ": replacement crew " << newCrew->ID << " arrives, SERVER UNHOGGED" << std::endl; 
	
		//if the time the replacement crew arrives the train is BEFORE MAX_SIM_TIME
		//then we want to remove the replacmentCrew from the queue to prevent
		//the same pointer to the replacmentCrew from being deleted twice.
		eventHandler->trainStation->replacementCrews.pop();
		newCrew->assignedTrain->isHoggedOut = false;

		stats->timeSpentHoggedOut += stats->currentTime - eventHandler->trainStation->startHoggedOutTime;

		Crew* oldCrew = newCrew->assignedTrain->workingCrew;
		delete oldCrew;

		newCrew->assignedTrain->workingCrew = newCrew;
		newCrew->startArrivalTime = stats->currentTime;

		newCrew->workTimeLeft -= newCrew->arrivalTimeDuration;

		TrainStartService* startService = new TrainStartService();
		eventHandler->Schedule(startService,newCrew->assignedTrain,newCrew->startArrivalTime);
	}
	else
	{
		std::cout << "CrewArrivalEvent4::execute() Entity owner failed to cast to Crew" << std::endl;
	}

}



//CrewDepartureEvent4----------------------------------------------------------------------------------
CrewDepartureEvent4::CrewDepartureEvent4() : SimEvent()
{
	name = "CrewDepartureEvent4";
}
CrewDepartureEvent4::~CrewDepartureEvent4() {}

void CrewDepartureEvent4::execute(TrainSimStats* stats)
{
	executeCrewDeparture4(owner,stats);
}
void CrewDepartureEvent4::executeCrewDeparture4(Entity* owner, TrainSimStats* stats)
{

	//std::cout << "CrewDepartureEvent4::execute() Crew  " <<  owner->ID << std::endl;
	Crew* oldCrew = dynamic_cast<Crew*>(owner);
	if(oldCrew)
	{
		std::cout <<"4";
		std::cout << "Time " << stats->currentTime << ": train " << oldCrew->assignedTrain->ID << ": crew " << oldCrew->ID << " hogged out during service; SERVER HOGGED" << std::endl;
		
		oldCrew->assignedTrain->isHoggedOut = true;
		oldCrew->assignedTrain->hogOutCount++;	

		Crew* newCrew = new Crew(++stats->crewID);
		newCrew->isReplacementCrew = true;
		newCrew->assignedTrain = oldCrew->assignedTrain;
		newCrew->unloadTimeDuration = oldCrew->unloadTimeDuration;
		newCrew->unloadTimeLeft = oldCrew->unloadTimeLeft;
		newCrew->workTimeDuration = crew_work_time_distribution(generator);
		newCrew->workTimeLeft = newCrew->workTimeDuration;
		newCrew->arrivalTimeDuration = crew_arrival_time_distribution(generator);
		newCrew->startWorkTime = stats->currentTime;


		//if the time the replacement crew arrives the train exceeds MAX_SIM_TIME, 
		//then we want to free up their resources after the simulation is over using trainStation.RemoveReplacmentCrews();
		eventHandler->trainStation->replacementCrews.push(newCrew);

		CrewArrivalEvent4* arrivalEvent = new CrewArrivalEvent4();
		eventHandler->Schedule(arrivalEvent,newCrew,newCrew->startWorkTime + newCrew->arrivalTimeDuration);
	}
	else
	{
		std::cout << "CrewDepartureEvent4::execute() Failed to cast Entity owner to Crew" << std::endl;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////


//EndSimEvent-----------------------------------------------------------

EndSimEvent::EndSimEvent() : SimEvent()
{
	name = "EndSimEvent";
}
EndSimEvent::~EndSimEvent() {}

void EndSimEvent::execute(TrainSimStats* stats)
{
	executeEndSim(owner,stats);
}

void EndSimEvent::executeEndSim(Entity* owner, TrainSimStats* stats)
{

	stats->avgTimeInSystem = stats->totalTrainTimeInSystem / stats->totalTrainsServed;

	stats->timeSpentBusyPercent = (stats->timeSpentBusy / stats->max_sim_time) * 100.0f;
	stats->timeSpentIdlePercent = (stats->timeSpentIdle / stats->max_sim_time) * 100.0f;
	stats->timeSpentHoggedOutPercent = (stats->timeSpentHoggedOut / stats->max_sim_time) * 100.0f;
	stats->timeSpentHoggedOutInQueuePercent = (stats->timeSpentHoggedOutInQueue / stats->max_sim_time) * 100.0f;

	stats->expected_total_percentage = 100.0f;
	//without timeSpentHoggedOutInQueue
	stats->actual_total_percentage = stats->timeSpentBusyPercent + stats->timeSpentIdlePercent + stats->timeSpentHoggedOutPercent;
	//with timeSpentHoggedOutInQueue calculated.
	float percentage = stats->actual_total_percentage + stats->timeSpentHoggedOutInQueuePercent;

	//this is now correct. ~ I have the correct solution on openlab!
	stats->timeAverage = (int)(stats->trainLineSum / stats->max_sim_time);

	float difference = stats->expected_total_percentage - stats->actual_total_percentage;

	stats->observedAvgArrivalTime = stats->observedAvgArrivalTime / stats->trainEnterSystemCount;

	std::cout << std::endl << "Train Simulation Stats: " << std::endl << std::endl;

	//std::cout <<"expected percentage: " << stats->expected_total_percentage << std::endl;
	//std::cout << "actual percentage without QueueHogPercentage: " << stats->actual_total_percentage << std::endl;
	//std::cout << "difference: " << difference << std::endl;
	//std::cout << "percentage with QueueHogPercentage: " << percentage << std::endl << std::endl;;
	//std::cout << "Queue Hogged Out: " << stats->timeSpentHoggedOutInQueuePercent << "%" << std::endl << std::endl;	

	std::cout << "Expected Average Arrival Time: " << stats->r << std::endl;
	std::cout << "Observed Average Arrival Time: " << stats->observedAvgArrivalTime << std::endl;
	std::cout << "Total Trains Served: " << stats->totalTrainsServed << std::endl;
	std::cout << "Average Time-In-System: " << stats->avgTimeInSystem << std::endl;
	std::cout << "Max Time-In-System: " << stats->maxTimeInSystem << std::endl << std::endl;
	std::cout << "Loading Dock Busy: " << stats->timeSpentBusyPercent << "%" << std::endl;
	std::cout << "Loading Dock Idle: " << stats->timeSpentIdlePercent << "%" << std::endl;
	std::cout << "Loading Dock Hogged Out: " << stats->timeSpentHoggedOutPercent << "%" << std::endl << std::endl;

	std::cout << "Time Average of Trains in Queue: " << stats->timeAverage << std::endl;
	std::cout << "Max Number of Trains in Queue: " << stats->maxTrainsInQueue << std::endl << std::endl;
	std::cout << "Histogram of Number of Times Trains Hogged Out: " << std::endl;
	
	stats->DisplayHistogram();
}