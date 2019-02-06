//#define MP
//#define REMOVE_OUTLIERS

#define OUTPUT_MULTIPLE_PROBES

#include <string>
#include <map>
#include <time.h>
#include <iomanip>
#include <math.h>
#include <iostream>
#include <fstream>

//#include "ChenAstarPrediction.h"
#include "ChenAstarHashPrediction.h"
#include "../SolverProxy.h"

using namespace::std;

int main(int argc, char **argv)
{
	if( argc != 9 )
	{
		cout << "Invalid arguments" << endl;
		exit(-1);
	}

	int probes = atoi( argv[1] );
	int meta_probes = atoi( argv[2] );
	int search_nodes_per_type = atoi( argv[3] );
	int meta_nodes_per_type = atoi( argv[4] );
	int lookahead = atoi( argv[5] );
	string pdb( argv[6] );
	string problems_file( argv[7] );
	string solutions( argv[8] );

	AbstractionHeuristic * hf = new AbstractionHeuristic( );
	hf->read_abstraction_data( argv[6] );

	ChenAstarHashPrediction * prediction = new ChenAstarHashPrediction( hf, meta_nodes_per_type, search_nodes_per_type );
	SolverProxy proxy( solutions );

	double sPrediction, endPrediction;

	cout << setiosflags( ios::left );
	cerr << setiosflags( ios::left );

	FILE * file = fopen(problems_file.c_str(), "r");
	if( file == NULL )
	{
		cout << "Could not open file " << problems_file << endl;
		exit(-1);
	}

	State * s = new State();
	char input[10000];
	int problem_number = 1;

	while( fgets( input , 10000 , file ) != NULL )
	{
		state_t state;
		read_state( input, &state );
		s->setState(state);
		s->setPred(s);
		s->setHistory(init_history);
		s->setRule( -1 );

		//cout << "Problem: " << problem_number << endl;

		map<int, NodesTime> nodes_expanded = proxy.solve();

		map<int, NodesTime>::iterator it = nodes_expanded.begin();
		for( ; it != nodes_expanded.end(); ++it )
		{
			if( it->second.time >= 5 )
			{
#ifdef OUTPUT_MULTIPLE_PROBES
				cout << it->first << " " << it->second.nodes << " " << it->second.time << endl;
#endif

				sPrediction = clock();
				double predicted = prediction->predict( s, it->first, probes, meta_probes, lookahead, 0 );
				endPrediction = clock();

#ifndef OUTPUT_MULTIPLE_PROBES
				cout << it->first << " " <<  predicted << " " << it->second.nodes << " "
										<< (endPrediction - sPrediction) / CLOCKS_PER_SEC << " " << it->second.time << endl;
#endif

				//cout << setw(15) << it->first << setw(15) <<  predicted << setw(15) << it->second.nodes << setw(15)
					//	<< (endPrediction - sPrediction) / CLOCKS_PER_SEC << setw(15) << it->second.time << endl;
			}
		}

		problem_number++;

		//cout << endl;
	}

	delete s;
	delete prediction;
	delete hf;

	return 0;
}
