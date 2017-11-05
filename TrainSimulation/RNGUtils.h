#ifndef RNG_UTILS
#define RNG_UTILS

#include <random>
#include <math.h>
#include <time.h>


//Global Variables------------------------------------------------------------

const float MAX_WORK_TIME_LIMIT = 12.0f;
const float MAX_SIM_TIME = 7200.0f;

const float MIN_UNLOAD_TIME = 3.5f;
const float MAX_UNLOAD_TIME = 4.5f;
const float MIN_CREW_ARRIVAL_TIME = 2.5f;
const float MAX_CREW_ARRIVAL_TIME = 3.5f;
const float MIN_CREW_WORK_TIME = 6.0f;
const float MAX_CREW_WORK_TIME = 11.0f;


//Random Number Generators and distributions------------------------------------------------

static std::random_device rand_device;
//static std::mt19937::result_type seed = time(0);
static std::mt19937 generator(rand_device());
static std::uniform_real_distribution<float> dist;
static std::uniform_real_distribution<float> unload_time_distribution(MIN_UNLOAD_TIME,MAX_UNLOAD_TIME);
static std::uniform_real_distribution<float> crew_arrival_time_distribution(MIN_CREW_ARRIVAL_TIME,MAX_CREW_ARRIVAL_TIME);
static std::uniform_real_distribution<float> crew_work_time_distribution(MIN_CREW_WORK_TIME,MAX_CREW_WORK_TIME);


//example usage:
//int estimated_train_arrival_time = poisson(generator);
//float estimated_train_unload_time = unload_time_distribution(generator);
//float estimated_crew_arrival_time = crew_arrival_time_distribution(generator);
//train_arrival_time_distribution(generator);


#endif