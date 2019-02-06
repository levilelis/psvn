#include <string>
#include <map>
#include <time.h>
#include <iomanip>
#include <math.h>
#include <iostream>
#include <fstream>

//#include "ChenDuplicatePrediction.h"
#include "ChenCyclePrediction2.h"
#include "../Solver.h"

using namespace::std;

int main(int argc, char **argv)
{
	if( argc != 3 )
	{
		cout << "Invalid arguments" << endl;
		exit(-1);
	}

	int probes = atoi( argv[1] );
	int lookahead = atoi( argv[2] );

	//AbstractionHeuristic * hf = new AbstractionHeuristic();
	//hf->read_abstraction_data(argv[3]);

	ChenCyclePrediction2 * prediction = new ChenCyclePrediction2( );

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
	double predicted = prediction->predict( s, 5, probes, lookahead );
	endPrediction = clock();
	cout << "Total number of nodes predicted: " << setw(20) << predicted << setw(15) << (endPrediction - sPrediction) / CLOCKS_PER_SEC << " seconds." << endl;
	//			cout << setw(15) << it->first << setw(15) << it->second << setw(15) << predicted << setw(15) << endPrediction - sPrediction << endl;

	delete s;
	delete prediction;
	//delete hf;

	return 0;
}
