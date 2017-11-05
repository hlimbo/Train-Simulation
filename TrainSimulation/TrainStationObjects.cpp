#include "TrainStationObjects.h"
#include "TrainStation.h"//technically only needed if accessing the data members or functions of this class.

//Entity------------------------------------------

Entity::Entity()
{
	ID = -1;
}
	
Entity::Entity(int ID)
{
	this->ID = ID;
}

Entity::Entity(int ID, float arrivalTimeDuration)
{
	this->ID = ID;
	this->arrivalTimeDuration = arrivalTimeDuration;
}

Entity::~Entity()
{
}


//Crew--------------------------------------------------

Crew::Crew() : Entity() 
{
	isReplacementCrew = false;
	//isHoggedOut = false;
	assignedTrain = nullptr;
}
Crew::Crew(int ID) : Entity(ID)
{
	isReplacementCrew = false;
	//isHoggedOut = false;
	assignedTrain = nullptr;
}

Crew::~Crew(){}


//Train-----------------------------------------------------------

Train::Train() : Entity() {}
Train::Train(int ID) : Entity(ID) 
{
	workingCrew = nullptr;
	startUnloadTime = 0.0f;
	unloadTimeDuration = 0.0f;
	startWaitTime = 0.0f;
	waitTimeDuration = 0.0f;
	hogOutCount = 0;
	isHoggedOut = false;

}
Train::Train(int ID, Crew* newCrew) : Entity(ID)
{
	workingCrew = newCrew;
	startUnloadTime = 0.0f;
	unloadTimeDuration = 0.0f;
	startWaitTime = 0.0f;
	waitTimeDuration = 0.0f;
	hogOutCount = 0;
	isHoggedOut = false;

}

Train::Train(int ID, Crew* newCrew,float arrivalTimeDuration) : Entity(ID,arrivalTimeDuration)
{
	workingCrew = newCrew;
	startUnloadTime = 0.0f;
	unloadTimeDuration = 0.0f;
	startWaitTime = 0.0f;
	waitTimeDuration = 0.0f;
	hogOutCount = 0;
	isHoggedOut = false;

}
	
Train::~Train()
{
	if(workingCrew)
		delete workingCrew;
}