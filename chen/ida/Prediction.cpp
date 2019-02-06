#include <string>
#include <map>
#include <time.h>
#include <iomanip>
#include <math.h>
#include <iostream>
#include <fstream>

#include "ChenPrediction.h"
#include "Solver.h"

using namespace::std;

int main(int argc, char **argv)
{

	if( argc != 6 )
	{
		cout << "Invalid arguments" << endl;
		exit(-1);
	}

	string inputProblemsFile( argv[1] );
	int probes = atoi( argv[2] );
	int lookahead = atoi( argv[3] );
	string proxyname( argv[5] );

	cout << "# Solving the following problems " << inputProblemsFile << endl << endl;

	AbstractionHeuristic * hf = new AbstractionHeuristic();
	hf->read_abstraction_data(argv[4]);

	ChenPrediction * prediction = new ChenPrediction( hf );

	Solver * solver = NULL;
//	IDAProxy * proxy = NULL;

	if(strcmp(proxyname.c_str(), "noproxy") == 0)
	{
		solver = new Solver( hf, false );
	}
/*	else
	{
		proxy = new IDAProxy(proxyname);
	}
*/
	long sIda, endIda, sPrediction, endPrediction, s, e;

	cout << setiosflags( ios::left );
	cerr << setiosflags( ios::left );

	long nodesExpandedIDA = 0;
	char input[10000];

	FILE * file = fopen(inputProblemsFile.c_str(), "r");
	if( file == NULL )
	{
		cout << "Could not open file " << inputProblemsFile << endl;
		exit(-1);
	}

	s = clock();
	state_t state;

	while( fgets( input , 10000 , file ) != NULL )
	{
		read_state( input, &state );

		map<int, long> nodesExpandedByLevel;

//		if(proxy == NULL)
//		{
			sIda = clock();
			solver->idaStar( &state );
			endIda = clock();

			nodesExpandedByLevel = solver->getNodesExpanded();
//		}
//		else
//		{
//			sIda = clock();
//			proxy->idaStar();
//			endIda = clock();

	//		nodesExpandedByLevel = proxy->getNodesExpanded();
	//	}

		int h = hf->abstraction_data_lookup( &state );

		if( nodesExpandedByLevel.size() > 0 )
		{
			cout << setw(15) << "# Depth" << setw(15) << "Nodes Expanded" << setw(15) << "Prediction" << setw(15) << "CPU Cycles" << endl;
		}

		map<int, long>::iterator it = nodesExpandedByLevel.begin();
		for(; it != nodesExpandedByLevel.end(); ++it)
		{
			int depth = it->first;

			if(h > depth)
			{
				continue;
			}

			sPrediction = clock();
			State s;
			s.setState(state);
			s.setPred(&s);
			s.setHistory(init_history);
			double predicted = prediction->predict( s, depth, probes, lookahead );
			endPrediction = clock();

			cout << setw(15) << it->first << setw(15) << it->second << setw(15) << predicted << setw(15) << endPrediction - sPrediction << endl;
		}

		if(solver != NULL)
		{
			solver->clearNodesExpandedByLevel();
		}
	}
	e = clock();

	delete solver;
	delete prediction;
	delete hf;

	return 0;
}
