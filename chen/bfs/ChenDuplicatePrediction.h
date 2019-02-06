/*
 * ChenDuplicatePrediction.h
 *
 *  Created on: Oct 2, 2013
 *      Author: levilelis
 */

#ifndef CHENDUPLICATEPREDICTION_H_
#define CHENDUPLICATEPREDICTION_H_

#include "../Type.h"
#include "../TypeSampler.h"
#include "../State.h"

#include "../../randomc/randomc.h"
#include "../../randomc/mersenne.cpp"

#include <time.h>
#include <map>

class ChenDuplicatePrediction {

private:
	AbstractionHeuristic * hf;
	TypeSampler * sampler;
	CRandomMersenne * RanGen;
	map<Type, State> output;
	state_map_t * transpositions; // checks for the transpositons

	long generated;

	void probe( State state, int depth, int lookahead );

	long getProbingResult();

public:
	ChenDuplicatePrediction( AbstractionHeuristic * hf );
	~ChenDuplicatePrediction();
	double predict( State state, int depth, int probes, int lookahead );
};

ChenDuplicatePrediction::ChenDuplicatePrediction( AbstractionHeuristic * hf )
{
	this->hf = hf;
	this->sampler = new TypeSampler( hf );
	srand ( ( unsigned )time( NULL ) );
	//	RanGen = new CRandomMersenne( ( unsigned )time( NULL ) );
	RanGen = new CRandomMersenne( 1234 );
}

ChenDuplicatePrediction::~ChenDuplicatePrediction()
{
	delete sampler;
}

double ChenDuplicatePrediction::predict( State state, int depth, int probes, int lookahead )
{
	double totalPrediction = 0;
	double totalPredictionGenerated = 0;

	for( int i = 0; i < probes; i++ )
	{
		transpositions = new_state_map();
		output.clear();
		generated = 0;

		probe( state, depth, lookahead );
		long p = getProbingResult();

		//cout << "probing = " << p << endl;

		totalPrediction = totalPrediction + ( p - totalPrediction ) / ( i + 1 );
		totalPredictionGenerated = totalPredictionGenerated + ( generated - totalPredictionGenerated ) / ( i + 1 );
		delete transpositions;
	}

	return totalPrediction;
	//return totalPredictionGenerated;
}

void ChenDuplicatePrediction::probe( State state, int depth, int lookahead )
{
	map<Type, State> queue;
	state_map_add( transpositions, state.getState(), 0 );

	int h = hf->abstraction_data_lookup( state.getState() );

	Type object = sampler->getObject( state.getState(), h, lookahead );

	object.setLevel( 0 );
	state.setW( 1 );
	queue.insert( pair<Type, State>( object, state ) );

	while( !queue.empty() )
	{
		Type out = queue.begin()->first;
		State s = queue.begin()->second;
		queue.erase( out );
		//state_t * pred = s.getPred();

		output.insert( pair<Type, State> ( out, s ) );
		int g = out.getLevel();
		int w = s.getW();

		int rule_used;
		func_ptr iter;
		State child;

		init_bwd_iter( iter );
		while( ( rule_used = next_bwd_iter( iter, s.getState() ) ) >= 0 )
		{
			apply_bwd_rule( rule_used, s.getState(), child.getState() );

			if( compare_states( child.getState(), s.getPred() ) == 0 )   // parent pruning
			{
				continue;
			}

            const int *old_child_g = state_map_get( transpositions, child.getState() );
            if ( old_child_g == NULL || *old_child_g > (g + 1) )
            {
                state_map_add( transpositions, child.getState(), g + 1 );
            }
            else
            {
            	//cout << "found a transposition" << endl;
            	continue;
            }

			int h = hf->abstraction_data_lookup( child.getState() );

			generated += w;

			//the case in which depth < 0 represents a search without a cost bound
			if( depth < 0 || h + g + 1 <= depth )
			{
				Type object = sampler->getObject( child.getState(), h, lookahead );

				object.setLevel( g + 1 );
				child.setW( w );
				child.setPred(*(s.getState()));

				map<Type, State>::iterator queueIt = queue.find( object );
				if( queueIt != queue.end() )
				{
					int wa = queueIt->second.getW();
					queueIt->second.setW( wa + w );

					double prob = ( double )w / ( wa + w );
					int rand_100 = RanGen->IRandom(1,100);
					double a = ( double )rand_100 / 100;

					if( a < prob )
					{
						child.setW( wa + w );
						queue[object] = child;
					}
				}
				else
				{
					queue.insert( pair<Type, State>( object, child ) );
				}
			}
		}
	}
}

long ChenDuplicatePrediction::getProbingResult()
{
	long expansions = 0;
	long expansion_level = 0;

	int level = 0;

	cout << "output size: " << output.size() << endl;

	map<Type, State>::iterator it = output.begin();
	for( ; it != output.end(); ++it )
	{
		if(it->first.getLevel() == level)
		{
			expansion_level += it->second.getW();
		}
		else
		{
			cout << "level: " << level << " nodes: " << expansion_level << endl;
			level = it->first.getLevel();
			expansion_level = it->second.getW();;
		}

		expansions += it->second.getW();
	}

	cout << endl << endl;

	return expansions;
}


#endif /* CHENPREDICTION_H_ */
