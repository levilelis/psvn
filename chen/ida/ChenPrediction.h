/*
 * ChenPrediction.h
 *
 *  Created on: Oct 2, 2013
 *      Author: levilelis
 */

#ifndef CHENPREDICTION_H_
#define CHENPREDICTION_H_

#include "../Type.h"
#include "../TypeSampler.h"
#include "../State.h"

#include "../../randomc/randomc.h"
#include "../../randomc/mersenne.cpp"

#include <time.h>
#include <map>

class ChenPrediction {

private:
	AbstractionHeuristic * hf;
	TypeSampler * sampler;
	CRandomMersenne * RanGen;
	map<Type, State> output;

	long generated;

	void probe( State state, int depth, int lookahead );

	long getProbingResult();

public:
	ChenPrediction( AbstractionHeuristic * hf );
	~ChenPrediction();
	double predict( State state, int depth, int probes, int lookahead );
};

ChenPrediction::ChenPrediction( AbstractionHeuristic * hf )
{
	this->hf = hf;
	this->sampler = new TypeSampler( hf );
	srand ( ( unsigned )time( NULL ) );
	//	RanGen = new CRandomMersenne( ( unsigned )time( NULL ) );
	RanGen = new CRandomMersenne( 1234 );
}

ChenPrediction::~ChenPrediction()
{
	delete sampler;
}

double ChenPrediction::predict( State state, int depth, int probes, int lookahead )
{
	double totalPrediction = 0;
	double totalPredictionGenerated = 0;

	for( int i = 0; i < probes; i++ )
	{
		output.clear();
		generated = 0;

		probe( state, depth, lookahead );
		long p = getProbingResult();

		//cout << "probing = " << p << endl;

		totalPrediction = totalPrediction + ( p - totalPrediction ) / ( i + 1 );
		totalPredictionGenerated = totalPredictionGenerated + ( generated - totalPredictionGenerated ) / ( i + 1 );
	}

	return totalPrediction;
	//return totalPredictionGenerated;
}

void ChenPrediction::probe( State state, int depth, int lookahead )
{
	map<Type, State> queue;

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

		init_fwd_iter( iter );
		while( ( rule_used = next_fwd_iter( iter, s.getState() ) ) >= 0 )
		{
			apply_fwd_rule( rule_used, s.getState(), child.getState() );

			if( compare_states( child.getState(), s.getPred()->getState() ) == 0 )   // parent pruning
			{
				continue;
			}

			int h = hf->abstraction_data_lookup( child.getState() );

			generated += w;

			if( h + g + 1 <= depth )
			{
				Type object = sampler->getObject( child.getState(), h, lookahead );

				object.setLevel( g + 1 );
				child.setW( w );
				child.setPred(s.getState());

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

long ChenPrediction::getProbingResult()
{
	long expansions = 0;

	map<Type, State>::iterator it = output.begin();
	for( ; it != output.end(); ++it )
	{
		expansions += it->second.getW();
	}

	return expansions;
}


#endif /* CHENPREDICTION_H_ */
