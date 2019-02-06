/*
 * ChenAstarHashPrediction.h
 *
 *  Created on: Nov 5, 2013
 *      Author: levilelis
 */

#ifndef ChenAstarHashPrediction_H_
#define ChenAstarHashPrediction_H_

#include "../Type.h"
#include "../TypeSampler.h"
#include "../State.h"
#include "ChenCyclePrediction.h"
//#include "CycleSampler.h"
#include "CycleSamplerHashTable.h"
#include "../OutlierRemoval.h"

#include "../../randomc/randomc.h"
#include "../../randomc/mersenne.cpp"

#include <time.h>
#include <map>
#include <set>

class ChenAstarHashPrediction {

private:
	TypeSampler * sampler;
	CRandomMersenne * RanGen;
	map<Type, State*> output;
	map<Type, double> states_per_type;
	ChenCyclePrediction * cycle_prediction;
	//CycleSampler cycle_sampler;
	CycleSamplerHashTable cycle_sampler;
	AbstractionHeuristic * hf;
	state_map_t* hash_table;

	void probe( State* state, int depth, int meta_probes, int lookahead, int B );
	void computeNodesPerType( int i );
	void clearOutuput();
	void printOutput( map<Type, State*>& o );
	double computeNumberNodesExpanded();
	void printStatePerType();
	bool isTransposition( State * state, int depth, int probes );
	double computeNumberNodesExpandedOutput();
	void reset_state_map();

public:
	ChenAstarHashPrediction( AbstractionHeuristic * hf, int meta_nodes_per_type, int search_nodes_per_type );
	~ChenAstarHashPrediction();
	double predict( State* state, int depth, int probes, int meta_probes, int lookahead, int B );
};

ChenAstarHashPrediction::ChenAstarHashPrediction( AbstractionHeuristic * hf, int meta_nodes_per_type, int search_nodes_per_type )
{
	this->hf = hf;
	this->cycle_prediction = new ChenCyclePrediction( meta_nodes_per_type );
	this->sampler = new TypeSampler( hf, search_nodes_per_type );
	RanGen = new CRandomMersenne( ( unsigned )time( NULL ) );
	hash_table = new_state_map();
	//RanGen = new CRandomMersenne( 1234 );
}

ChenAstarHashPrediction::~ChenAstarHashPrediction()
{
	delete sampler;
	delete RanGen;
	delete cycle_prediction;
	delete hash_table;
}

void ChenAstarHashPrediction::reset_state_map()
{
  hash_table->avail_entries = (float)hash_table->max_entry * 0.75;
  for(int i = 0; i <= hash_table->max_entry; ++i ) {
	  hash_table->entries[ i ].state.vars[ 0 ] = -1;
  }
}

double ChenAstarHashPrediction::predict( State* state, int depth, int probes, int meta_probes, int lookahead, int B )
{
	double totalPrediction = 0;
	double sPrediction, endPrediction;
	vector<double> probe_results;

	reset_state_map();

	for( int i = 0; i < probes; i++ )
	{
		sPrediction = clock();
		probe( state, depth, meta_probes, lookahead, B );
		endPrediction = clock();

		double p = computeNumberNodesExpandedOutput();
		totalPrediction = totalPrediction + ( p - totalPrediction ) / ( i + 1 );

#ifdef OUTPUT_MULTIPLE_PROBES
		cout << p << " " << (endPrediction - sPrediction) / CLOCKS_PER_SEC << endl;
#endif

		clearOutuput();
	}

	return totalPrediction;
}

void ChenAstarHashPrediction::probe( State* state, int depth, int meta_probes, int lookahead, int B )
{
	map<Type, State*> queue;

	int h = hf->abstraction_data_lookup( state->getState() );
	Type object = sampler->getObject( state->getState(), h, lookahead );

	object.setLevel( 0 );
	state->setW( 1 );
	queue.insert( pair<Type, State*>( object, state ) );

	int current_level = 0;

	while( !queue.empty() )
	{
		//cout << "queue size: " << queue.size() << endl;
		Type out = queue.begin()->first;
		State * s = queue.begin()->second;
		queue.erase( out );

		int g = out.getLevel();
		double w = s->getW();

		if( g != current_level )
		{
		//	cout << "EXPANDING LEVEL:\t" << g << " with f: " << g + out.getH() <<  endl;
			current_level = g;
		}

		const int *ancestor_g = state_map_get( hash_table, s->getState() );
		if(ancestor_g != NULL && *ancestor_g < g)
		{
			delete s;
			continue;
		}

		State copy_s;
		copy_s = (*s);

		if( cycle_sampler.sample( &copy_s, g, meta_probes, B) )
		{
			delete s;
			continue;
		}

		state_map_add( hash_table, s->getState(), g );

		output.insert( pair<Type, State*> ( out, s ) );

		int rule_used;
		int c_history;
		func_ptr iter;

		init_fwd_iter( iter );
		while( ( rule_used = next_fwd_iter( iter, s->getState() ) ) >= 0 )
		{
#ifdef MP
            if( !fwd_rule_valid_for_history( s->getHistory(), rule_used ) )
            {
                    continue;
            }

            c_history = next_fwd_history( s->getHistory(), rule_used );
#endif

			State* child = new State();
			apply_fwd_rule( rule_used, s->getState(), child->getState() );
			child->setW( w );
			child->setPred( s );
			child->setRule( rule_used );
			child->setHistory( c_history );
#ifndef MP
			if( compare_states( child->getState(), s->getPred()->getState() ) == 0 )   // parent pruning
			{
				delete child;
				continue;
			}
#endif

			int h_child = hf->abstraction_data_lookup( child->getState() );

			if( h_child + g + 1 <= depth )
			{
				Type object = sampler->getObject( child->getState(), h_child, lookahead );
				object.setLevel( g + 1 );

				map<Type, State*>::iterator queueIt = queue.find( object );
				if( queueIt != queue.end() )
				{
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


void ChenAstarHashPrediction::printOutput( map<Type, State*>& o )
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


void ChenAstarHashPrediction::printStatePerType()
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

void ChenAstarHashPrediction::computeNodesPerType( int i )
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

void ChenAstarHashPrediction::clearOutuput()
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

double ChenAstarHashPrediction::computeNumberNodesExpanded()
{
	double nodes_expanded = 0;
	double nodes_per_level = 0;
	int current_level = 0;

	map<Type, double>::iterator it = states_per_type.begin();
	for( ; it != states_per_type.end(); ++it )
	{
		if( it->first.getLevel() != current_level )
		{
			current_level = it->first.getLevel();
			nodes_per_level = 0;
		}

		nodes_per_level += it->second;
		nodes_expanded += it->second;
	}

	return nodes_expanded;
}

double ChenAstarHashPrediction::computeNumberNodesExpandedOutput()
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
