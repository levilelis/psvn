/*
 * CycleSampler.h
 *
 *  Created on: Dec 13, 2013
 *      Author: levilelis
 */


#ifndef CycleSampler_H_
#define CycleSampler_H_

#include "../Type.h"
#include "../State.h"

#include "../../randomc/randomc.h"
//#include "../../randomc/mersenne.cpp"

#include <time.h>
#include <map>
#include <set>
#include <stack>

class CycleSampler {

private:
	CRandomMersenne * RanGen;
	int depth_current_node;
	bool isTransposition;
	vector<const char*> current_path;

	bool random_walk( State* state, int depth );
	int hasCycle( State* state, int depth );
	vector<const char*> getPath( State* state, int level );
	vector<const char*> getFlippedPath( State* state );
	int compare_paths( vector<const char*>& p1 ); // compares some path with the current path
	void clear( State * s, int depth );

public:
	CycleSampler();
	~CycleSampler();
	bool sample( State* state, int depth, int probes );
};

CycleSampler::CycleSampler()
{
	RanGen = new CRandomMersenne( ( unsigned )time( NULL ) );
	//RanGen = new CRandomMersenne( 1234 );
}

CycleSampler::~CycleSampler()
{
	delete RanGen;
}

bool CycleSampler::sample( State* state, int depth, int probes )
{
	isTransposition = false;
	depth_current_node = depth;

	current_path = getFlippedPath( state );

	for( int i = 0; i < probes; i++ )
	{
		if( random_walk( state, depth ) )
		{
			return true;
		}
	}

	return false;
}

void CycleSampler::clear( State * s, int depth )
{
	for( int i = 0; i < depth; i++ )
	{
		State * to_delete = s;
		s = s->getPred();

		delete to_delete;
	}
}

bool CycleSampler::random_walk( State* s, int depth )
{
	State * state = s;

	for( int i = 0; i < depth; i++ )
	{
		int rule_used;
		func_ptr iter;

		init_bwd_iter( iter );
		vector<int> actions;

		while( ( rule_used = next_bwd_iter( iter, state->getState() ) ) >= 0 )
		{
			actions.push_back( rule_used );
		}

		//hit a deadend
		if( actions.size() <= 1 )
		{
			clear( state, i );
			return false;
		}

		int chosen_action;
		State* child = new State();
		bool is_parent = false;

		do
		{
			is_parent = false;
			chosen_action = RanGen->IRandom( 0, actions.size() - 1 );
			apply_bwd_rule( actions[ chosen_action ], state->getState(), child->getState() );

			if( compare_states( child->getState(), state->getPred()->getState() ) == 0 )   // parent pruning
			{
				//cout << "FOUND PARENT ACTION SIZE: " << actions.size() << endl;
				is_parent = true;
			}
		} while( is_parent );

		child->setPred( state );
		child->setRule( actions[ chosen_action ] );

		int res = hasCycle( child, i + depth + 1 );
		if( res == 0 ) //proved the state to be a transposition
		{
			//cout << "FOUND A CYCLE" << endl;
			clear( child, i + 1 );
			return true;
		}
		if( res == 1 ) //found a cycle in child but did not prove the state to be a transposition
		{
			clear( child, i + 1 );
			return false;
		}

		state = child;
	}

	clear( state, depth );

	return false;
}

int CycleSampler::compare_paths( vector<const char*>& p1 )
{
	int diff = current_path.size() - p1.size();

	for( int i = 0; i < p1.size(); i++ )
	{
		int cmp = strcmp( p1[i], current_path[i + diff] );

		if( cmp != 0 )
		{
			return cmp;
		}
	}

	return 0;
}

vector<const char*> CycleSampler::getFlippedPath( State* state )
{
	stack<const char*> reverse_op;

	//we go all the way to the root of the search tree
	do
	{
		if( state->getRule() != -1 )
		{
			reverse_op.push( bwd_rule_names[ state->getRule() ] );
			state = state->getPred();
		}
	} while( compare_states( state->getState(), state->getPred()->getState() ) != 0 );

	vector<const char*> path;

	while( !reverse_op.empty() )
	{
		path.push_back( reverse_op.top() );
		reverse_op.pop();
	}

	return path;
}


vector<const char*> CycleSampler::getPath( State* state, int level )
{
	vector<const char*> path;

	for( int i = 0; i < level; i++ )
	{
		if( state->getRule() != -1 )
		{
			//getting the inverse of the rule used
			int rule_used;
			func_ptr iter;
			state_t child;

			init_bwd_iter( iter );
			while( ( rule_used = next_bwd_iter( iter, state->getState() ) ) >= 0 )
			{
				apply_bwd_rule( rule_used, state->getState(), &child );

				if( compare_states( &child, state->getPred()->getState() ) == 0 )
				{
					path.push_back( bwd_rule_names[ rule_used ] );
					break;
				}
			}

			state = state->getPred();
		}
	}

	return path;
}

/*
 * Returns:
 * 0 - if state is a transposition
 * 1 - if a cycle was found but state is not a transposition
 * 2 - if a cycle was not found
 */
int CycleSampler::hasCycle( State* state, int depth )
{
	State* current = state;
	int count = 0;

	//when the state is the same as pred we reached the root
	while( compare_states( state->getState(), state->getPred()->getState() ) != 0 )
	{
		state = state->getPred();
		count++;
		if( compare_states( current->getState(), state->getState() ) == 0 )
		{
			//the cycle is of even length
			if( count % 2 == 0 )
			{
				int transposition_level = depth - ( count / 2 );

				if( transposition_level < depth_current_node )
				{
					return 0;
				}

				if( transposition_level > depth_current_node )
				{
					return 1;
				}

				vector<const char*> path = getPath( current, depth - transposition_level );

				int cmp = compare_paths( path );
				if( cmp < 0 )
				{
					return 0;
				}

				if( cmp == 0 )
				{
					cout << "ERROR: CMP EQUALS ZERO!" << endl;
				}
			}
			else //cycle is of odd length
			{
				int transposition_level = depth - ( ( count - 1 ) / 2 );
				if( transposition_level <= depth_current_node )
				{
					return 0;
				}
			}

			return 1;
		}
	}

	return 2;
}

#endif /* CycleSampler_H_ */

