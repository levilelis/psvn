/*
 * ChenGraphPrediction.h
 *
 *  Created on: Nov 5, 2013
 *      Author: levilelis
 */

#ifndef ChenGraphPrediction_H_
#define ChenGraphPrediction_H_

#include "../Type.h"
#include "../TypeSampler2.h"
#include "../State.h"
#include "Cycle.h"
#include "ChenCyclePrediction.h"
//#include "CycleSampler.h"
#include "CycleSamplerHashTable.h"

#include "../../randomc/randomc.h"
#include "../../randomc/mersenne.cpp"

#include <time.h>
#include <map>
#include <set>

class ChenGraphPrediction {

private:
	TypeSampler2 * sampler;
	CRandomMersenne * RanGen;
	map<Type, State*> output;
	map<Type, double> states_per_type;
	ChenCyclePrediction * cycle_prediction;
//	CycleSampler cycle_sampler;
	CycleSamplerHashTable cycle_sampler;


	void probe( State* state, int depth, int meta_probes, int lookahead );
	void computeNodesPerType( int i );
	void clearOutuput();
	Cycle hasCycle( State* state );
	void printOutput( map<Type, State*>& o );
	double computeNumberNodesExpanded();
	void printStatePerType();
	bool isTransposition( State * state, int depth, int probes );
	double computeNumberNodesExpandedOutput();

public:
	ChenGraphPrediction( int meta_nodes_per_type, int search_nodes_per_type );
	~ChenGraphPrediction();
	double predict( State* state, int depth, int probes, int meta_probes, int lookahead );
};

ChenGraphPrediction::ChenGraphPrediction( int meta_nodes_per_type, int search_nodes_per_type )
{
	this->cycle_prediction = new ChenCyclePrediction( meta_nodes_per_type );
	this->sampler = new TypeSampler2( search_nodes_per_type );
	//srand ( ( unsigned )time( NULL ) );
	RanGen = new CRandomMersenne( ( unsigned )time( NULL ) );
	//RanGen = new CRandomMersenne( 1234 );
}

ChenGraphPrediction::~ChenGraphPrediction()
{
	delete sampler;
	delete RanGen;
	delete cycle_prediction;
}

double ChenGraphPrediction::predict( State* state, int depth, int probes, int meta_probes, int lookahead )
{
	double totalPrediction = 0;

	for( int i = 0; i < probes; i++ )
	{
		probe( state, depth, meta_probes, lookahead );

		double p = computeNumberNodesExpandedOutput();
		totalPrediction = totalPrediction + ( p - totalPrediction ) / ( i + 1 );

		computeNodesPerType( i );
		clearOutuput();

		//cout << "FINISHED PROBE " << i + 1 << endl;
	}

	cout << "###" << endl;

	double nodes_expanded = computeNumberNodesExpanded();

	//cout << "TOTAL NUMBER NODES EXPANDED: " << totalPrediction << endl;

	//cout << "States per Type: " << endl;
	//printStatePerType();

	return nodes_expanded;
}

void ChenGraphPrediction::probe( State* state, int depth, int meta_probes, int lookahead )
{
	map<Type, State*> queue;
	//state_map_add( transpositions, state->getState(), 0 );

	Type object = sampler->getObject( state->getState(), -1, lookahead );

	object.setLevel( 0 );
	state->setW( 1 );
	queue.insert( pair<Type, State*>( object, state ) );

	int current_level = 0;

	while( !queue.empty() )
	{
		Type out = queue.begin()->first;
		State * s = queue.begin()->second;
		queue.erase( out );

		int g = out.getLevel();
		double w = s->getW();

		if( g != current_level )
		{
			//cout << "EXPANDING LEVEL:\t" << g << endl;
			current_level = g;
		}

		bool isTransposition = false;
		State copy_s;
		copy_s = (*s);

		//if( cycle_prediction->predict( &copy_s, g, meta_probes, lookahead ) )
		if( cycle_sampler.sample(&copy_s, g, meta_probes) )
		{
			delete s;
			continue;
		}

		output.insert( pair<Type, State*> ( out, s ) );

		int rule_used;
		func_ptr iter;

		init_bwd_iter( iter );
		while( ( rule_used = next_bwd_iter( iter, s->getState() ) ) >= 0 )
		{
			//cout << bwd_rule_names[ rule_used ] << endl;

			State* child = new State();
			apply_bwd_rule( rule_used, s->getState(), child->getState() );
			child->setW( w );
			child->setPred( s );
			child->setRule( rule_used );

			if( compare_states( child->getState(), s->getPred()->getState() ) == 0 )   // parent pruning
			{
				delete child;
				continue;
			}

			Type object = sampler->getObject( child->getState(), -1, lookahead );
			object.setLevel( g + 1 );

			//the case in which depth < 0 represents a search without a cost bound
			if( g + 1 <= depth )
			{
				map<Type, State*>::iterator queueIt = queue.find( object );
				if( queueIt != queue.end() )
				{
		//			queueIt->first.print();
		//			cout << "WEIGHT: " << queueIt->second->getW() + w << endl << endl;

					double wa = queueIt->second->getW();
					queueIt->second->setW( wa + w );

					double prob = ( double ) w / ( wa + w );
					int rand_100 = RanGen->IRandom(1,100);
					double a = ( double )rand_100 / 100;

					if( a < prob )
					{
						State * toDelete = queue[object];

						child->setW( wa + w );
						queue[object] = child;

						delete toDelete;
					}
					else
					{
						delete child;
					}
				}
				else
				{
					queue.insert( pair<Type, State*>( object, child ) );
				}
			}
			else
			{
				delete child;
			}
		}
	}
}


void ChenGraphPrediction::printOutput( map<Type, State*>& o )
{
	map<Type, State*>::iterator it = o.begin();
	for( ; it != o.end(); ++it )
	{
		//cout << "level: \t" << it->first.getLevel() << " number children \t" << it->first.getH() << " nodes: \t" << it->second->getW() << endl;
		it->first.print();
		cout << "\t: " << it->second->getW() << endl;
	}

	cout << endl << endl;
}


void ChenGraphPrediction::printStatePerType()
{
	map<Type, double>::iterator it = states_per_type.begin();
	for(; it != states_per_type.end(); ++it)
	{
		//cout << "[" << it->first.getLevel() << "] \t" << it->second << endl;
		//cout << "[" << it->first.getLevel() << ", " << it->first.getP() << ", " << it->first.getH() <<  "] \t" << it->second << endl;
		cout << "TYPE" << endl;
		it->first.print();

		cout << "NUMBER: " << it->second << endl << endl;
	}
	cout << endl << endl;
}

void ChenGraphPrediction::computeNodesPerType( int i )
{

	map<Type, double>::iterator it1 = states_per_type.begin();
	for( ; it1 != states_per_type.end(); ++it1 )
	{
		if( output.find( it1->first ) == output.end() )
		{
			State * s = new State();
			s->setW( 0.0 );

			output[it1->first] = s;
		}
	}

	map<Type, State*>::iterator it = output.begin();
	for( ; it != output.end(); ++it )
	{
		Type t = it->first;

		if( states_per_type.find( t ) == states_per_type.end() )
		{
			states_per_type[t] = it->second->getW() / (i + 1);
		}
		else
		{
			states_per_type[t] = states_per_type[t] + ( it->second->getW() - states_per_type[t] ) / ( i + 1 );
		}
	}
}

void ChenGraphPrediction::clearOutuput()
{
	map<Type, State*>::iterator it = output.begin();
	for( ; it != output.end(); ++it )
	{
		if(it->first.getLevel() != 0)
		{
			delete it->second;
		}
	}

	output.clear();
}

double ChenGraphPrediction::computeNumberNodesExpanded()
{
	double nodes_expanded = 0;
	double nodes_per_level = 0;
	int current_level = 0;

	map<Type, double>::iterator it = states_per_type.begin();
	for( ; it != states_per_type.end(); ++it )
	{
		if( it->first.getLevel() != current_level )
		{
			//cout << current_level << "," << nodes_per_level << endl;
			current_level = it->first.getLevel();
			nodes_per_level = 0;
		}

		nodes_per_level += it->second;
		nodes_expanded += it->second;
	}

//	cout << current_level << "," << nodes_per_level << endl;
	cout << current_level << endl;

	return nodes_expanded;
}

double ChenGraphPrediction::computeNumberNodesExpandedOutput()
{
	double nodes_expanded = 0;

	map<Type, State*>::iterator it = output.begin();
	for( ; it != output.end(); ++it )
	{
		nodes_expanded += it->second->getW();
	}

	return nodes_expanded;
}


#endif /* ChenGraphPrediction_H_ */
