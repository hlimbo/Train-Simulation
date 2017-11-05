#ifndef TRAIN_STATION
#define TRAIN_STATION

#include <vector>
#include <queue>

enum class StationStates { IDLE, BUSY };

class Train;
class Crew;

//The environment that trains and crew members arrive and depart from
class TrainStation
{
public:
	Train* dockedTrain;
	//Used to free up trains that haven't arrived in the queue due to the simulation ending.
	std::queue<Train*> trainsToArriveQueue;
	//Used to free up replacement crews that haven't arrived to their designated trains due to the simulation ending.
	std::queue<Crew*> replacementCrews;
	std::queue<Train*> trainLineQueue;
	StationStates state;
	
	float startIdleTime;
	float startBusyTime;
	float startHoggedOutTime;
	float startHoggedOutTimeInQueue;

	TrainStation();
	~TrainStation();

	Train* PopFromQueue();
	void PushToQueue(Train* train);
	Train* Front();
	bool IsEmpty();

	Train* getDockedTrain();

	//called at the end of the simulation if any trains are still waiting in the queue.
	void RemoveTrains();

	//called at the end of the simulation if any replacment crews are scheduled to arrive past max simulation time.
	void RemoveReplacementCrews();
};

#endif