#ifndef TRAIN_STATION_OBJECTS
#define TRAIN_STATION_OBJECTS
#include <vector>

class TrainStation;
class Train;
class SimEvent;

class Entity
{
public:
	int ID;
	float arrivalTimeDuration;
	float startArrivalTime;
	float departureTime;
	float startUnloadTime;
	float unloadTimeDuration;
	float unloadTimeLeft;

	Entity();
	
	Entity(int ID);
	Entity(int ID,float arrivalTimeDuration);

	virtual ~Entity();

};

class Crew : public Entity
{
public:
	float startWorkTime;
	float endWorkTime;
	float workTimeDuration;
	float workTimeLeft;
	//bool isHoggedOut;

	bool isReplacementCrew;

	Train* assignedTrain;
	

	Crew();
	Crew(int ID);

	virtual ~Crew();
};

class Train : public Entity
{
public:
	float startWaitTime;
	float waitTimeDuration;
	int hogOutCount;	
	bool isHoggedOut;

	Crew* workingCrew;

	Train();
	Train(int ID);
	Train(int ID, Crew* newCrew);
	Train(int ID,Crew* newCrew,float arrivalTime);
	

	virtual ~Train();
};

#endif