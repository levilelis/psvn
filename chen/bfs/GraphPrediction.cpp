#include <string>
#include <map>
#include <time.h>
#include <iomanip>
#include <math.h>
#include <iostream>
#include <fstream>

//#include "ChenDuplicatePrediction.h"
#include "ChenGraphPrediction.h"
//#include "../Solver.h"

using namespace::std;

int main(int argc, char **argv)
{
	if( argc != 7 )
	{
		cout << "Invalid arguments" << endl;
		exit(-1);
	}

	int probes = atoi( argv[1] );
	int meta_probes = atoi( argv[2] );
	int search_nodes_per_type = atoi( argv[3] );
	int meta_nodes_per_type = atoi( argv[4] );
	int lookahead = atoi( argv[5] );
	int max_depth_search = atoi( argv[6] );

	//AbstractionHeuristic * hf = new AbstractionHeuristic();
	//hf->read_abstraction_data(argv[3]);

	ChenGraphPrediction * prediction = new ChenGraphPrediction( meta_nodes_per_type, search_nodes_per_type );

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
	s->setRule( -1 );
	double predicted = prediction->predict( s, max_depth_search, probes, meta_probes, lookahead );
	endPrediction = clock();
	//cout << "Total number of nodes predicted: " << setw(20) << predicted << setw(15) << (endPrediction - sPrediction) / CLOCKS_PER_SEC << " seconds." << endl;
	//			cout << setw(15) << it->first << setw(15) << it->second << setw(15) << predicted << setw(15) << endPrediction - sPrediction << endl;

	cout << predicted << endl;
	cout << (endPrediction - sPrediction) / CLOCKS_PER_SEC << endl;

	delete s;
	delete prediction;
	//delete hf;

	return 0;
}
