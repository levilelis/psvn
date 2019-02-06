/*
 * CycleCount.cpp
 *
 *  Created on: Nov 21, 2013
 *      Author: levilelis
 */
#include <string>
#include <map>
#include <time.h>
#include <iomanip>
#include <math.h>
#include <iostream>
#include <fstream>

#include "DFSCountCycles.h"

using namespace::std;

int main(int argc, char **argv)
{
	if( argc != 2 )
	{
		cout << "Invalid arguments" << endl;
		exit(-1);
	}

	int lookahead = atoi( argv[1] );

	DFSCountCycles * counter = new DFSCountCycles( );

	long sPrediction, endPrediction;

	cout << setiosflags( ios::left );
	cerr << setiosflags( ios::left );

	state_t state;
	int d;

	first_goal_state( &state, &d );

	sPrediction = clock();
	State * s = new State();
	s->setState(state);
	s->setPred(s);
	s->setHistory(init_history);
	counter->countCycles( s, 8, lookahead );
	endPrediction = clock();

	//delete s;
	delete counter;

	return 0;
}


