/*
 * DFSCountCycles.h
 *
 *  Created on: Nov 21, 2013
 *      Author: levilelis
 */

#ifndef DFSCOUNTCYCLES_H_
#define DFSCOUNTCYCLES_H_

#include "../Type.h"
#include "../TypeSampler2.h"
#include "../State.h"
#include "Cycle.h"

#include "../../randomc/randomc.h"
#include "../../randomc/mersenne.cpp"

#include <time.h>
#include <map>
#include <set>

class DFSCountCycles {

private:

	TypeSampler2 * sampler;
	map<Type, map<Cycle, double> > cycles;
	map<Type, long> state_per_type;
	set<Type> types_sampled;
	long nodes_expanded;

	void dfs( State* state, int depth, int lookahead, int level );
	Cycle hasCycle( State * state, int lookahead, int depth );
	void printCycles(map<Type, map<Cycle, double> >& c);
	void printStatePerType();
	int getNumberChildrenPredecessor( State * state );

public:
	DFSCountCycles(){ this->sampler = new TypeSampler2( ); }
	void countCycles( State* state, int depth, int lookahead );

};

void DFSCountCycles::countCycles( State* state, int depth, int lookahead )
{
	nodes_expanded = 0;

	dfs( state, depth, lookahead, 0 );

	cout << "The number of nodes expanded was: " << nodes_expanded << endl;

	printCycles( cycles );

	cout << "States Per Type:" << endl;
	printStatePerType();
	cout << endl << endl;
}

void DFSCountCycles::printStatePerType()
{
	map<Type, long>::iterator it = state_per_type.begin();
	for(; it != state_per_type.end(); ++it)
	{
		//cout << "[" << it->first.getLevel() << "] \t" << it->second << endl;
		//cout << "[" << it->first.getLevel() << ", " << it->first.getP() << ", " << it->first.getH() <<  "] \t" << it->second << endl;
		cout << "TYPE" << endl;
		it->first.print();

		cout << "NUMBER: " << it->second << endl << endl;
	}
	cout << endl << endl;
}

void DFSCountCycles::dfs( State * state, int depth, int lookahead, int level )
{
	int rule_used;
	func_ptr iter;

	state->setG( level );

	Cycle cycle = hasCycle( state, lookahead, depth );

	if(cycle.getL() > 0)
	{
		//int number_children_predecessor = getNumberChildrenPredecessor( state );
		//Type cycle_object = sampler->getObject( state->getState(), number_children_predecessor, lookahead );

		Type cycle_object = cycle.getType();
		//cycle_object.setLevel( level - cycle.getL() );
		cycle_object.setLevel( level - cycle.getL() );

		cycle.setD( level );

		map<Type, map<Cycle, double> >::iterator itt = cycles.find( cycle_object );
		if( itt != cycles.end() )
		{
			map<Cycle, double>::iterator it = itt->second.find( cycle );
			if( it != itt->second.end() )
			{
				cycles[cycle_object][cycle] = cycles[cycle_object][cycle] + 1;
			}
			else
			{
				cycles[cycle_object][cycle] = 1;
			}
		}
		else
		{
			cycles[cycle_object][cycle] = 1;
		}

//		delete state;
//		return;
	}

	nodes_expanded++;

	int number_children_predecessor = getNumberChildrenPredecessor( state );
	Type t = sampler->getObject( state->getState(), number_children_predecessor, lookahead );
	t.setLevel( level );
	if( state_per_type.find( t ) == state_per_type.end() )
	{
		state_per_type[ t ] = 1;
	}
	else
	{
		state_per_type[ t ] = state_per_type[ t ] + 1;
	}

	init_bwd_iter( iter );
	while( ( rule_used = next_bwd_iter( iter, state->getState() ) ) >= 0 )
	{
		State * child = new State();
		apply_bwd_rule( rule_used, state->getState(), child->getState() );

		if( compare_states( child->getState(), state->getPred()->getState() ) == 0 )   // parent pruning
		{
			delete child;
			continue;
		}

		child->setPred( state );

		if( level + 1 <= depth )
		{
			dfs( child, depth, lookahead, level + 1 );
		}
		else
		{
			delete child;
		}
	}

	delete state;
}

Cycle DFSCountCycles::hasCycle( State* state, int lookahead, int depth )
{
	State* current = state;
	int count = 0;
	Cycle cycle(-1, -1);

	//when the state is the same as pred we reached the root
	while( compare_states( state->getState(), state->getPred()->getState() ) != 0 )
	{
		state = state->getPred();
		count++;
		if( compare_states( current->getState(), state->getState() ) == 0 )
		{
			Type t, t1, t2;

			//cout << "Found Cycle from level " << current->getG() << " to level: " << state->getG() << " of length: " << count << endl;

			//the cycle is of even length
			if( count % 2 == 0 )
			{
				int number_children_parent = getNumberChildrenPredecessor( state );
				t = sampler->getObject( state->getState(), number_children_parent, lookahead );

				int transposition_level = depth - ( count / 2 );

				state = current;
				for( int i = 0; i < transposition_level; i++ )
				{
					state = state->getPred();
				}

				number_children_parent = getNumberChildrenPredecessor( state );
				t1 = sampler->getObject( state->getState(), number_children_parent, lookahead );
				t1.setLevel( transposition_level );

				state = state->getPred();
				number_children_parent = getNumberChildrenPredecessor( state );
				t2 = sampler->getObject( state->getState(), number_children_parent, lookahead );
				t2.setLevel( transposition_level + 1 );
			}
			else //cycle is of odd length
			{
				int number_children_parent = getNumberChildrenPredecessor( state );
				t = sampler->getObject( state->getState(), number_children_parent, lookahead );

				int transposition_level = depth - ( ( count + 1 ) / 2 );

				state = current;
				for( int i = 0; i < transposition_level; i++ )
				{
					state = state->getPred();
				}

				number_children_parent = getNumberChildrenPredecessor( state );
				t1 = sampler->getObject( state->getState(), number_children_parent, lookahead );
				t1.setLevel( transposition_level );
			}

			cycle.setType( t );
			cycle.setType1( t1 );
			cycle.setType2( t2 );
			cycle.setL( count );
			return cycle;
		}
	}

	return cycle;
}

int DFSCountCycles::getNumberChildrenPredecessor( State * state )
{
	//return -1;
	int number_children_parent = 0;

	//it is the root of the search tree
//	if( compare_states( state->getPred()->getState(), state->getState() ) == 0 )
//	{
		/*
		char ss[100];
		char ss1[100];
		sprint_state(ss, 100, state->getState());
		sprint_state(ss1, 100, state->getPred()->getState());
		cout << "\t Parent" << "\t" <<  ss1 << " \t Child \t" << ss << endl;

		cout << endl << endl;
*/
//		number_children_parent = -1;
//	}
//	else
//	{
		//getting the number of children of the predecessor
		int rule_used;
		func_ptr iter;
		init_bwd_iter( iter );
		while( ( rule_used = next_bwd_iter( iter, state->getPred()->getState() ) ) >= 0 )
		{
			number_children_parent++;
		}
//	}

	return number_children_parent;
}
/*
int DFSCountCycles::getNumberChildrenPredecessor( State * state )
{
	int number_children_parent = 0;
	State * pred = state->getPred();

	//it is the root of the search tree
	if( compare_states( pred->getState(), state->getState() ) == 0 )
	{
		number_children_parent = -1;
	}
	else
	{
		//getting the number of children of the predecessor
		int rule_used;
		func_ptr iter;
		init_bwd_iter( iter );
		while( ( rule_used = next_bwd_iter( iter, state->getState() ) ) >= 0 )
		{
			number_children_parent++;
		}
	}

	return number_children_parent;
}
*/


void DFSCountCycles::printCycles(map<Type, map<Cycle, double> >& c)
{
	map<Type, map<Cycle, double> >::iterator outiter = c.begin();
	for( ; outiter != c.end(); ++outiter )
	{
	//	if(outiter->first.getLevel() == 0)
	//	{
			//cout << "[" <<  outiter->first.getLevel() << ", " << outiter->first.getH() << ", " << outiter->first.getP() << "]" << endl;
		cout << "TYPE" << endl;
		outiter->first.print();
		cout << endl;
			//cout << "Level of Type: \t" << outiter->first.getLevel() << " Value of Type \t" << outiter->first.getH() << endl;

		cout << "CYCLES" << endl;
			map<Cycle, double>::iterator it = outiter->second.begin();
			for( ; it != outiter->second.end(); ++it )
			{
				cout << "<" << it->first.getD() << ", " << it->first.getL() << ", " << it->second << "> " << endl;
				//<< "\t [" << it->first.getType1().getP() << ", " << it->first.getType1().getH()
				//		<< ", " << it->first.getType1().getLevel() << "] \t [" << it->first.getType2().getP() << ", " << it->first.getType2().getH()
				//		<< ", " << it->first.getType2().getLevel() << "]" << endl;
			}

			cout << endl << endl;
	//	}
	}


	/*
	map<Type, map<Cycle, double> >::iterator outiter = c.begin();
	for( ; outiter != c.end(); ++outiter )
	{
	//	if(outiter->first.getLevel() == 0)
	//	{
			cout << "Level of Type: \t" << outiter->first.getLevel() << " Value of Type \t" << outiter->first.getH() << endl;

			map<Cycle, double>::iterator it = outiter->second.begin();
			for( ; it != outiter->second.end(); ++it )
			{
				cout << "\t Depth: \t\t" << it->first.getD() << "\t Length: \t\t" << it->first.getL() << "\t Number: \t\t" << it->second << endl;
				cout << "Type of the derivated transpositions: " << "\t\t [t1:" << it->first.getType1().getH()
						<< ", " << it->first.getType1().getLevel() << "] \t\t [t2:" << it->first.getType2().getH()
						<< ", " << it->first.getType2().getLevel() << "]" << endl;
			}

			cout << endl << endl;
	//	}
	}
	*/


}

#endif /* DFSCOUNTCYCLES_H_ */
