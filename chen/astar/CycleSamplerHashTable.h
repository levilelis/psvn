/*
 * CycleSamplerHashTable.h
 *
 *  Created on: Dec 13, 2013
 *      Author: levilelis
 */


#ifndef CycleSamplerHashTable_H_
#define CycleSamplerHashTable_H_

#include "../Type.h"
#include "../State.h"

#include "../../randomc/randomc.h"
//#include "../../randomc/mersenne.cpp"

#include <time.h>
#include <map>
#include <set>
#include <stack>

class CycleSamplerHashTable {

private:
	CRandomMersenne * RanGen;
	int depth_current_node;
	bool isTransposition;
	vector<const char*> current_path;
	state_map_t* path;

	bool random_walk( State* state, int depth );
	bool beam_random_walk( State* state, int depth, int B );
	int hasCycle( State* state, int depth );
	vector<const char*> getPath( State* state, int level );
	vector<const char*> getFlippedPath( State* state, int depth );
	int compare_paths( vector<const char*>& p1 ); // compares some path with the current path
	void clear( State * s, int depth );
	void clear( vector<State*>& states );
	void reset_state_map();

public:
	CycleSamplerHashTable();
	~CycleSamplerHashTable();
	bool sample( State* state, int depth, int probes, int B );
};

CycleSamplerHashTable::CycleSamplerHashTable()
{
	RanGen = new CRandomMersenne( ( unsigned )time( NULL ) );
	//RanGen = new CRandomMersenne( 1234 );
	path = new_state_map();
}

CycleSamplerHashTable::~CycleSamplerHashTable()
{
	delete RanGen;
}

/* reset a map of states to values */
void CycleSamplerHashTable::reset_state_map()
{
  path->avail_entries = (float)path->max_entry * 0.75;
  for(int i = 0; i <= path->max_entry; ++i ) {
	  path->entries[ i ].state.vars[ 0 ] = -1;
  }
}

bool CycleSamplerHashTable::sample( State* state, int depth, int probes, int B )
{
	isTransposition = false;
	depth_current_node = depth;
	//path = new_state_map();
	reset_state_map();

	current_path = getFlippedPath( state, depth );

	for( int i = 0; i < probes; i++ )
	{
		//if( beam_random_walk( state, depth, B ) )
		if( random_walk( state, depth ) )
		{
			//destroy_state_map(path);
			return true;
		}
	}

	//destroy_state_map(path);
	return false;
}

void CycleSamplerHashTable::clear( State * s, int depth )
{
	for( int i = 0; i < depth; i++ )
	{
		State * to_delete = s;
		s = s->getPred();

		delete to_delete;
	}
}

void CycleSamplerHashTable::clear( vector<State*>& states )
{
	for( int i = 0; i < states.size(); i++ )
	{
		delete states[i];
	}
}

bool CycleSamplerHashTable::beam_random_walk( State* s, int depth, int B )
{
	vector<int> total_actions;
	vector<State*> total_states;

	vector<int> next_total_actions;
	vector<State*> next_total_states;

	vector<State*> expanded_states;
	//expanded_states.push_back(s);

	State * state = s;

	//Storing the pairs <state, actions> to then later choose a subset of pairs randomly
	int rule_used;
	func_ptr iter;
	init_fwd_iter( iter );
	while( ( rule_used = next_fwd_iter( iter, state->getState() ) ) >= 0 )
	{
		total_actions.push_back( rule_used );
		total_states.push_back( state );
	}

	for( int i = 0; i < depth; i++ )
	{
		//selecting the B nodes to be expanded next
		for(int j = 0; j < B; j++)
		{
			State* child = new State();
			expanded_states.push_back( child );
			int chosen_action;
			bool is_parent = false;

			do
			{
				//hit a deadend
				if(total_actions.size() == 0)
				{
					clear( expanded_states );
					return false;
				}

				is_parent = false;

				chosen_action = RanGen->IRandom( 0, total_actions.size() - 1 );
				apply_fwd_rule( total_actions[ chosen_action ], total_states[ chosen_action ]->getState(), child->getState() );

				if( compare_states( child->getState(), total_states[ chosen_action ]->getPred()->getState() ) == 0 )   // parent pruning
				{
				//	total_states.erase(total_states.begin() + chosen_action);
				//	total_actions.erase(total_actions.begin() + chosen_action);
					is_parent = true;
				}
			} while( is_parent );

		//	int chosen_action = RanGen->IRandom( 0, total_actions.size() - 1 );
		//	apply_fwd_rule( total_actions[ chosen_action ], total_states[ chosen_action ]->getState(), child->getState() );

			child->setPred( total_states[ chosen_action ] );
			child->setRule( total_actions[ chosen_action ] );

			int res = hasCycle( child, i + depth + 1 );

			if( res == 0 ) //proved the state to be a transposition
			{
			//	cout << "found transposition" << endl;
				clear( expanded_states );
				return true;
			}
			if( res == 1 ) //found a cycle in child but did not prove the state to be a transposition
			{
				clear( expanded_states );
				return false;
			}

			init_fwd_iter( iter );
			while( ( rule_used = next_fwd_iter( iter, child->getState() ) ) >= 0 )
			{
				next_total_actions.push_back( rule_used );
				next_total_states.push_back( child );
			}
		}

		total_states.clear();
		total_actions.clear();

		total_states = next_total_states;
		total_actions = next_total_actions;

		next_total_states.clear();
		next_total_actions.clear();
	}

	clear( expanded_states );
	return false;
}

bool CycleSamplerHashTable::random_walk( State* s, int depth )
{
	State * state = s;

	for( int i = 0; i < depth; i++ )
	{
		int rule_used;
		func_ptr iter;

		init_fwd_iter( iter );
		vector<int> actions;

		while( ( rule_used = next_fwd_iter( iter, state->getState() ) ) >= 0 )
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
			apply_fwd_rule( actions[ chosen_action ], state->getState(), child->getState() );

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

		//state_map_add( path, child->getState(), i + depth + 1 );

		state = child;
	}

	clear( state, depth );

	return false;
}

int CycleSamplerHashTable::compare_paths( vector<const char*>& p1 )
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

vector<const char*> CycleSamplerHashTable::getFlippedPath( State* state, int g )
{
	stack<const char*> reverse_op;

	//we go all the way to the root of the search tree
	do
	{
		if( state->getRule() != -1 )
		{
			reverse_op.push( fwd_rule_names[ state->getRule() ] );
			state_map_add( path, state->getState(), g );
			g--;
			state = state->getPred();
		}
	} while( compare_states( state->getState(), state->getPred()->getState() ) != 0 );

	state_map_add( path, state->getState(), g );

	//cout << "value of g: " << g << endl;

	vector<const char*> path;

	while( !reverse_op.empty() )
	{
		path.push_back( reverse_op.top() );
		reverse_op.pop();
	}

	return path;
}


vector<const char*> CycleSamplerHashTable::getPath( State* state, int level )
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

			init_fwd_iter( iter );
			while( ( rule_used = next_fwd_iter( iter, state->getState() ) ) >= 0 )
			{
				apply_fwd_rule( rule_used, state->getState(), &child );

				if( compare_states( &child, state->getPred()->getState() ) == 0 )
				{
					path.push_back( fwd_rule_names[ rule_used ] );
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
int CycleSamplerHashTable::hasCycle( State* state, int depth )
{

	const int *ancestor_g = state_map_get( path, state->getState() );
	int count = -1;

	if(ancestor_g != NULL)
	{
		count = depth - *ancestor_g;

		//the cycle is of even length
		if( count % 2 == 0 )
		{
			int transposition_level = depth - ( count / 2 );

			if( transposition_level < depth_current_node )
			{
				//cout << count << " " << count_hash << endl;
				return 0;
			}

			if( transposition_level > depth_current_node )
			{
				//cout << "returning here!!" << endl;
				return 1;
			}

			vector<const char*> path = getPath( state, depth - transposition_level );

			int cmp = compare_paths( path );
			if( cmp < 0 )
			{
	//			cout << count << " " << count_hash << endl;
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
				//cout << count << " " << count_hash << endl;
				return 0;
			}
		}

		return 1;
	}

	return 2;


/*
	State* current = state;
	int count = 0;

	const int *ancestor_g = state_map_get( path, state->getState() );
	int count_hash = -1;

	if(ancestor_g != NULL)
	{
		count_hash = depth - *ancestor_g;
	}

	//when the state is the same as pred we reached the root
	while( compare_states( state->getState(), state->getPred()->getState() ) != 0 )
	{
		state = state->getPred();
		count++;
		if( compare_states( current->getState(), state->getState() ) == 0 )
		{
			if(count != count_hash)
			{
				cout << "ERROR COUNT HASH!" << endl;
			}

			//cout << count << " " << count_hash << endl;

			//the cycle is of even length
			if( count % 2 == 0 )
			{
				int transposition_level = depth - ( count / 2 );

				if( transposition_level < depth_current_node )
				{
					//cout << count << " " << count_hash << endl;
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
					//cout << count << " " << count_hash << endl;
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
					//cout << count << " " << count_hash << endl;
					return 0;
				}
			}

			//cout << "RETURNING 1" << endl;
			return 1;
		}
	}

	return 2;
*/

}

#endif /* CycleSamplerHashTable_H_ */

