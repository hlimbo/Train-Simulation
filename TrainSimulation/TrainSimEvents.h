#ifndef TRAIN_SIM_EVENTS
#define TRAIN_SIM_EVENTS
#include <string>

class SimEventHandler;
class SimEvent;
class Entity;
class TrainSimStats;
class TrainStation;


//used to sort values in the event list "pq" by the time the event begins in ascending order.
class Compare
{
public:
	bool operator()(SimEvent* s1, SimEvent* s2);
};



//Class that follows the command pattern where each event derived from SimEvent
//executes its own functionality depending on the type of event it is.
class SimEvent
{
protected:
	//time when scheduled event is expected to begin.
	float timeStamp;

	//the entity that executes this event.
	Entity* owner;

	//used to schedule new events in execute()
	SimEventHandler* eventHandler;

public:
	std::string name;

	SimEvent();
	virtual ~SimEvent();

	//used to override event functionality by SimEvent derived type.
	virtual void execute(TrainSimStats* stats);

	void setTimeStamp(float newTimeStamp);
	
	float getTimeStamp();

	Entity* getOwner();

	void setOwner(Entity* newOwner);

	SimEventHandler* getEventHandler();

	void setEventHandler(SimEventHandler* eventHandler);

};


class TrainArrivalEvent : public SimEvent
{
public:
	TrainArrivalEvent();
	virtual ~TrainArrivalEvent();

	virtual void execute(TrainSimStats* stats) override;
	void executeTrainArrivalEvent(Entity* entity,TrainSimStats* stats);
};

class TrainDepartureEvent : public SimEvent
{
public:
	TrainDepartureEvent();
	virtual ~TrainDepartureEvent();

	virtual void execute(TrainSimStats* stats) override;
	void executeTrainDepartureEvent(Entity* entity, TrainSimStats* stats);
};

class TrainEnterQEvent : public SimEvent
{
public:
	TrainEnterQEvent();
	virtual ~TrainEnterQEvent();

	virtual void execute(TrainSimStats* stats) override;
	void executeTrainEnterQ(Entity* entity, TrainSimStats* stats);

};

class TrainExitQEvent : public SimEvent
{
public:
	TrainExitQEvent();
	virtual ~TrainExitQEvent();
	
	virtual void execute(TrainSimStats* stats) override;
	void executeTrainExitQ(Entity* entity, TrainSimStats* stats);

};

class TrainStartService : public SimEvent
{
public:
	TrainStartService();
	virtual ~TrainStartService();

	virtual void execute(TrainSimStats* stats) override;
	void executeTrainStartService(Entity* entity, TrainSimStats* stats);
};

class TrainEndService : public SimEvent
{
public:
	TrainEndService();
	virtual ~TrainEndService();

	virtual void execute(TrainSimStats* stats) override;
	void executeTrainEndService(Entity* entity, TrainSimStats* stats);
};

//Called when replacement crew arrives in Queue and schedules TrainExitQueueEvent.
class CrewArrivalEvent2 : public SimEvent
{
public:
	CrewArrivalEvent2();
	virtual ~CrewArrivalEvent2();

	virtual void execute(TrainSimStats* stats) override;
	void executeCrewArrival2(Entity* owner,TrainSimStats* stats);
};

//Called when train hogs out in TrainExitQueueEvent.
class CrewDepartureEvent2 : public SimEvent
{
public:
	CrewDepartureEvent2();
	virtual ~CrewDepartureEvent2();

	virtual void execute(TrainSimStats* stats) override;
	void executeCrewDeparture2(Entity* owner,TrainSimStats* stats);
};

//Scheduled when Replacement Crew arrives and schedules a TrainEndServiceEvent
class CrewArrivalEvent3 : public SimEvent
{
public:
	CrewArrivalEvent3();
	virtual ~CrewArrivalEvent3();

	virtual void execute(TrainSimStats* stats) override;
	void executeCrewArrival3(Entity* owner, TrainSimStats* stats);
};

//Called when train hogs out in TrainEndServiceEvent.
class CrewDepartureEvent3 : public SimEvent
{
public:
	CrewDepartureEvent3();
	virtual ~CrewDepartureEvent3();

	virtual void execute(TrainSimStats* stats) override;
	void executeCrewDeparture3(Entity* owner, TrainSimStats* stats);
};

//Called when replacement crew arrives and schedules a TrainStartServiceEvent.
class CrewArrivalEvent4 : public SimEvent
{
public:
	CrewArrivalEvent4();
	virtual ~CrewArrivalEvent4();

	virtual void execute(TrainSimStats* stats) override;
	void executeCrewArrival4(Entity* owner, TrainSimStats* stats);
};

//Called when train hogs out in TrainStartServiceEvent
class CrewDepartureEvent4 : public SimEvent
{
public:
	CrewDepartureEvent4();
	virtual ~CrewDepartureEvent4();

	virtual void execute(TrainSimStats* stats) override;
	void executeCrewDeparture4(Entity* owner, TrainSimStats* stats);

};

//-------------------------------------------------------------------------------------------------------------------------

//PseudoEvent that is used to signal the end of simulation, calculate statistics, and print out results of simulation
class EndSimEvent : public SimEvent
{
public:
	EndSimEvent();
	virtual ~EndSimEvent();

	virtual void execute(TrainSimStats* stats) override;
	void executeEndSim(Entity* entity, TrainSimStats* stats);
};

#endif