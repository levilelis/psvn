#ifndef Solver_h__
#define Solver_h__

#include <time.h>
#include <iostream>
#include <iomanip>
#include <map>

#include "../../AbstractionHeuristic.h"

using namespace::std;

#define getNodesExpandedByLevel 1
//#define getNodesExpandedByLevelByIteration 1
//#define MAX_SEARCH_DEPTH 63
//#define VERBOSE

class Solver {

	private:
		AbstractionHeuristic * hf;
		
		long nodesExpanded;
		long nodesGenerated;
		long s, end;
		bool finished;
		int cost;
		bool searchAfterGoal;
		map<int, long> nodesExpandedByLevel;
		map<int, long> nodesExpandedByLevelByIteration;

	public:
		Solver( AbstractionHeuristic * hf, bool searchAfterGoal );
		void idaStar( const state_t * start );
		int costLimitedBFS( const state_t * currentState, const state_t * prev, int g, int thresh, int history );
		int getCost();
		int getNodesExpanded( int level );
		map<int, long>& getNodesExpanded();
		void clearNodesExpandedByLevel();
		void updateNodesExpandedByLevelByIteration( int g );
};

Solver::Solver( AbstractionHeuristic * hf, bool searchAfterGoal )
{
	this->hf = hf;
	this->nodesExpanded = 0;
	this->nodesGenerated = 0;
	this->cost = 0;
	this->finished = false;
	this->searchAfterGoal = searchAfterGoal;
}

void Solver::idaStar( const state_t * start )
{
	this->nodesExpanded = 0;
	this->nodesGenerated = 0;
	this->finished = false;
	this->end = this->s = 0;

	s = clock();

	//int thresh = abstraction_lookup( hf, start );
    int thresh = hf->abstraction_data_lookup( start );

	int startHeuristic = thresh;
#ifdef MAX_SEARCH_DEPTH
	while(!finished && thresh <= MAX_SEARCH_DEPTH)
#else
	while(!finished)
#endif
	{
		this->nodesExpanded = 0;
		this->nodesGenerated = 0;
#ifdef VERBOSE
		cout << "Depth " << thresh << " "; 
#endif

		int newThresh = costLimitedBFS(start, start, 0, thresh, init_history);
#ifdef VERBOSE
		cout << "Nodes expanded " << this->nodesExpanded <<" nodes generated "<< this->nodesGenerated << endl;
#endif
#if (getNodesExpandedByLevel == 1)
		nodesExpandedByLevel[thresh] = nodesExpanded;
#endif

#if (getNodesExpandedByLevelByIteration == 1)
		cout << "Threshold: " << thresh << endl;
		cout << setw(10) << "Depth" << setw(10) << "Nodes Exp." << endl;
		map<int, int>::iterator it = nodesExpandedByLevelByIteration.begin();
		for(; it != nodesExpandedByLevelByIteration.end(); ++it)
		{
			cout << setw(10) << it->first << setw(10) << it->second << endl;
		}

		nodesExpandedByLevelByIteration.clear();
#endif

		thresh = newThresh;
	}
#ifdef VERBOSE
	cout << "Execution time " << (double)(end - s) / (double)CLOCKS_PER_SEC << endl;
	cout << "Number of nodes generated was " << this->nodesGenerated << endl;
	cout << "Number of nodes expanded was " << this->nodesExpanded << endl << endl << endl;
#endif
}

int Solver::costLimitedBFS( const state_t * currentState, const state_t * pred, int g, int thresh, int history )
{
	//int heuristic = abstraction_lookup( hf, currentState );
	int heuristic = hf->abstraction_data_lookup( currentState );
    
	//cout << "h = " << heuristic << endl;

	int newThresh = 23049358; //a large number
	int funcReturn;

	if (g + heuristic > thresh) 
	{
		return (g + heuristic);
	}

	if( is_goal( currentState ) )
	{
		end = clock();
		finished = true;
		cost = g;

		if(!searchAfterGoal)
		{
			return newThresh;
		}
	}

	this->nodesExpanded++;

#if(getNodesExpandedByLevelByIteration == 1)
	updateNodesExpandedByLevelByIteration(g);
#endif			
	state_t child;
	func_ptr iter;
	int rule_used;
	int c_history;

	init_fwd_iter( iter );

        while( ( rule_used = next_fwd_iter( iter, currentState ) ) >= 0 ) 
        {
#ifdef USE_HISTORY
                if( !fwd_rule_valid_for_history( history, rule_used ) )
                {
                        continue;
                }

                c_history = next_fwd_history( history, rule_used );
#endif
                apply_fwd_rule( rule_used, currentState, &child );

		//parent pruning
		if( !compare_states( pred, &child ) )
		{
			continue;
		}

		funcReturn = costLimitedBFS(&child, currentState, g + 1, thresh, c_history);           
		newThresh = newThresh < funcReturn ? newThresh : funcReturn;
	}

	return newThresh;
}

void Solver::updateNodesExpandedByLevelByIteration( int g )
{
	if(nodesExpandedByLevelByIteration.find(g) == nodesExpandedByLevelByIteration.end())
	{
		nodesExpandedByLevelByIteration[g] = 1;
	}
	else
	{
		nodesExpandedByLevelByIteration[g] += 1;
	}
}

int Solver::getCost()
{
	return cost;
}

int Solver::getNodesExpanded(int level)
{
	return nodesExpandedByLevel[level];
}

map<int, long>& Solver::getNodesExpanded()
{
	return nodesExpandedByLevel;
}

void Solver::clearNodesExpandedByLevel()
{
	nodesExpandedByLevel.clear();
}

#endif
