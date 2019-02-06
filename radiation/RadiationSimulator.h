/*
 * RadiationSimulator.h
 *
 *  Created on: Jan 19, 2014
 *      Author: levilelis
 */

#ifndef RADIATIONSIMULATOR_H__
#define RADIATIONSIMULATOR_H__

#include "../randomc/randomc.h"
#include "../randomc/mersenne.cpp"
#include <iostream>

using namespace std;

class RadiationSimulator {

private:
    state_map_t* map;
    double percentage; //informs the percentage of pdb which will be corrupted
    int frequency;
    CRandomMersenne * RandGen;
    int number_stes;
    int64_t total_corruptions;

public:
    RadiationSimulator(state_map_t* map, int frequency);
    ~RadiationSimulator();

    void simulate();
    void reset();
    int64_t getTotalCorruptions();

};

RadiationSimulator::RadiationSimulator(state_map_t* map, int frequency)
{
	this->percentage = percentage;
	this->frequency = frequency;
	this->map = map;
	this->RandGen = new CRandomMersenne( ( unsigned )time( NULL ) );
	this->number_stes = 0;
	this->total_corruptions = 0;
}

RadiationSimulator::~RadiationSimulator()
{
	delete RandGen;
	//method should o nothing, memory clearance
	//happens at the AbstractionHeuristic class
}

void RadiationSimulator::reset() {
	this->number_stes = 0;
	this->total_corruptions = 0;

}

int64_t RadiationSimulator::getTotalCorruptions() {
	return this->total_corruptions;
}

void RadiationSimulator::simulate() {

	this->number_stes++;

	if(frequency > 0 && number_stes % frequency == 0)
	{
		total_corruptions++;

		// Chooses a bit at random and flips it
		int64_t random_entry;
		do {
			//cout << "HERE" << endl;
			random_entry = RandGen->IRandom(0, map->max_entry);
		} while( map->entries[ random_entry ].state.vars[ 0 ] < 0 );
		//cout <<"OUT" << endl;

		int shift = RandGen->IRandom(0,31);
		int num = 0;
		num = num | 1 << shift;

		int mask = map->entries[ random_entry ].value & num;
		//random bit is 0, must be set to 1
		if(mask == 0)
		{
			map->entries[ random_entry ].value = map->entries[ random_entry ].value + num;
		}
		else //random bit is 1, must be set to 0
		{
			num = ~num;
			map->entries[ random_entry ].value = map->entries[ random_entry ].value & num;
		}

		if(map->entries[ random_entry ].value < 0) map->entries[ random_entry ].value = 0;
	}
}

#endif /* RADIATIONSIMULATOR_H__ */
