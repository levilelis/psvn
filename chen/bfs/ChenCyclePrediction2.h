/*
 * ChenCyclePrediction2.h
 *
 *  Created on: Nov 5, 2013
 *      Author: levilelis
 */

#ifndef ChenCyclePrediction2_H_
#define ChenCyclePrediction2_H_

#include "../Type.h"
#include "../TypeSampler2.h"
#include "../State.h"
#include "Cycle.h"

#include "../../randomc/randomc.h"
#include "../../randomc/mersenne.cpp"

#include <time.h>
#include <map>
#include <set>

class ChenCyclePrediction2 {

private:
	TypeSampler2 * sampler;
	CRandomMersenne * RanGen;
	map<Type, State*> output;
	//state_map_t * transpositions; // checks for the transpositons
	map<Type, double> states_per_type;
	map<Type, map<Cycle, double> > cycles;
	map<Type, map<Cycle, double> > cycles_avg;
	map<Type, map<Cycle, double> > cycles_per_type;
	map<Type, map<Type, double> > B; //type branching factor
	map<Type, map<Type, double> > B_avg;
	map<Type, map<Type, double> > B_per_type;
	map<Type, map<Type, double> > transpositions;
	map<Type, double> tree_size;
	map<Type, long> number_samples;
	set<Type> types_sampled;

	void probe( State* state, int depth, int lookahead );
	void computeNodesPerType( int i );
	void computeNodesPerType2();
	void clearOutuput();
	Cycle hasCycle( State* state, int lookahead, int depth );
	void updateCycles(int i);
	void updateBranchingFactor(int i);
	void printCycles(map<Type, map<Cycle, double> >& c);
	void computeTranspositions();
	void printTranspositions();
	void printOutput( map<Type, State*>& o );
	long computeNumberNodesExpanded();
	void computeCyclesPerType();
	void averageNodesPerType(int probes);
	void printStatePerType();
	int getNumberChildrenPredecessor( State * state );
	void computeBPerType();
	void buildTreeSizes();
	void printB( map<Type, map<Type, double> >& c );

public:
	ChenCyclePrediction2( );
	~ChenCyclePrediction2();
	double predict( State* state, int depth, int probes, int lookahead );
};

ChenCyclePrediction2::ChenCyclePrediction2( )
{
	this->sampler = new TypeSampler2( );
	srand ( ( unsigned )time( NULL ) );
	RanGen = new CRandomMersenne( ( unsigned )time( NULL ) );
	//RanGen = new CRandomMersenne( 1234 );
}

ChenCyclePrediction2::~ChenCyclePrediction2()
{
	delete sampler;
	delete RanGen;
}

double ChenCyclePrediction2::predict( State* state, int depth, int probes, int lookahead )
{
	double totalPrediction = 0;
	double totalPredictionGenerated = 0;

	for( int i = 0; i < probes; i++ )
	{
		clearOutuput();
		probe( state, depth, lookahead );
		computeNodesPerType( i );
		updateCycles( i );
		updateBranchingFactor( i );
	}

	computeCyclesPerType();
	computeBPerType();

	/*
	cout << "Total number of cycles in the tree: " << endl;
	printCycles(cycles_avg);

	cout << endl << endl;

	cout << "Number of cycles per type: " << endl;
	printCycles( cycles_per_type );

	cout << endl << endl;

	double nodes_expanded = computeNumberNodesExpanded();
	cout << "Nodes expanded: " << nodes_expanded << endl;

	cout << endl << endl;

	cout << "States per Type: " << endl;
	printStatePerType();

	printB( B_per_type );
*/

	cout << "Number of cycles per type: " << endl;
	printCycles( cycles_per_type );

	buildTreeSizes();

//	cout << "States per type: " << endl;
//	printStatePerType();

	double nodes_expanded = computeNumberNodesExpanded();
	cout << "Nodes expanded: " << nodes_expanded << endl;

	return totalPrediction;
	//return totalPredictionGenerated;
}

void ChenCyclePrediction2::probe( State* state, int depth, int lookahead )
{
	map<Type, State*> queue;
	//state_map_add( transpositions, state->getState(), 0 );

	int number_children_parent = getNumberChildrenPredecessor( state );
	Type object = sampler->getObject( state->getState(), number_children_parent, lookahead );

	object.setLevel( 0 );
	state->setW( 1 );
	queue.insert( pair<Type, State*>( object, state ) );

//	bool isCycle = false;

	while( !queue.empty() )
	{
//		printOutput(queue);

		Type out = queue.begin()->first;
		State* s = queue.begin()->second;
		queue.erase( out );

		Cycle cycle = hasCycle( s, lookahead, out.getLevel() );
		if(cycle.getL() > 0)
		{
			Type cycle_object = cycle.getType();
			cycle.setD( out.getLevel() );
			cycle_object.setLevel( out.getLevel() - cycle.getL() );

			//State * ancestor = output.find( cycle_object )->second;
			//double adjusted_weight = s->getW() / ancestor->getW();
			double adjusted_weight = s->getW();

			//double adjusted_weight = w;
			//cout << "current weight: " << w << " ancestor's weight: " << ancestor->getW() << endl;

			map<Type, map<Cycle, double> >::iterator itt = cycles.find( cycle_object );
			if( itt != cycles.end() )
			{
				map<Cycle, double>::iterator it = itt->second.find( cycle );
				if( it != itt->second.end() )
				{
					cycles[cycle_object][cycle] = cycles[cycle_object][cycle] + adjusted_weight;
				}
				else
				{
					cycles[cycle_object][cycle] = adjusted_weight;
				}
			}
			else
			{
				cycles[cycle_object][cycle] = adjusted_weight;
			}

	//		isCycle = true;

	//		delete s;
	//		continue;
		}

		types_sampled.insert( out );


		//state_t * pred = s.getPred();

//		char ss[100];
//		sprint_state(ss, 100, s->getState());
//		cout << "\t Current" << "\t" <<  ss << " \t level \t\t" << out.getLevel() << endl;

		output.insert( pair<Type, State*> ( out, s ) );

		if( number_samples.find( out ) == number_samples.end() )
		{
			number_samples[out] = 1;
		}
		else
		{
			number_samples[out] = number_samples[out] + 1;
		}

		int g = out.getLevel();
		long w = s->getW();

		int rule_used;
		func_ptr iter;

		init_bwd_iter( iter );
		while( ( rule_used = next_bwd_iter( iter, s->getState() ) ) >= 0 )
		{
			State* child = new State();
			apply_bwd_rule( rule_used, s->getState(), child->getState() );
			child->setW( w );
			child->setPred( s );

			if( compare_states( child->getState(), s->getPred()->getState() ) == 0 )   // parent pruning
			{
				delete child;
				continue;
			}

			int number_children_parent = getNumberChildrenPredecessor( child );
			Type object = sampler->getObject( child->getState(), number_children_parent, lookahead );
			object.setLevel( g + 1 );

			if( B[out].find( object ) == B[out].end() )
			{
				B[out][object] = w;
			}
			else
			{
				B[out][object] = B[out][object] + w;
			}

			//the case in which depth < 0 represents a search without a cost bound
			if( g + 1 <= depth )
			{
				map<Type, State*>::iterator queueIt = queue.find( object );
				if( queueIt != queue.end() )
				{
					long wa = queueIt->second->getW();
					queueIt->second->setW( wa + w );

					double prob = ( double )w / ( wa + w );
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

void ChenCyclePrediction2::computeTranspositions()
{
	/*
	map<Type, map<Cycle, double> >::iterator out_it = cycles_per_type.begin();
	for( ; out_it != cycles_per_type.end(); ++out_it )
	{

	}
	*/
}

void ChenCyclePrediction2::buildTreeSizes()
{
	map<Type, map<Type, double> >::reverse_iterator it = B_per_type.rbegin();
	for(; it != B_per_type.rend(); ++it)
	{
		double type_tree_size = 1;

		map<Type, double>::iterator it_children = it->second.begin();
		for(; it_children != it->second.end(); it_children++)
		{
			double children_type_tree_size = 0;

			if( tree_size.find( it_children->first ) != tree_size.end() )
			{
				children_type_tree_size = tree_size[it_children->first];
			}

			type_tree_size = type_tree_size + it_children->second * children_type_tree_size;
		}

		double total_to_subtract = 0;

		if( cycles_per_type.find( it->first ) != cycles_per_type.end() )
		{
			map<Cycle, double> cycles_under = cycles_per_type[it->first];

			map<Cycle, double>::iterator it_cycles = cycles_under.begin();
			for( ; it_cycles != cycles_under.end(); ++it_cycles )
			{
				if( it_cycles->first.getL() == 2 )
				{
					Type t1 = it_cycles->first.getType1();

					if( tree_size.find( t1 ) != tree_size.end() )
					{
						total_to_subtract += tree_size[t1] * it_cycles->second;
					}

					continue;
				}

				if( it_cycles->first.getL() % 2 == 0 ) //even length
				{
					Type t1 = it_cycles->first.getType1();
					Type t2 = it_cycles->first.getType2();

					if( tree_size.find( t1 ) != tree_size.end() )
					{
						total_to_subtract += ( tree_size[t1] * ( it_cycles->second / 2 ) );
					}

					if( tree_size.find( t2 ) != tree_size.end() )
					{
						total_to_subtract += ( tree_size[t2] * ( it_cycles->second / 2 ) );
					}
				}
				else //odd length
				{
					Type t1 = it_cycles->first.getType1();

					if( tree_size.find( t1 ) != tree_size.end() )
					{
						total_to_subtract += tree_size[t1] * it_cycles->second;
						//total_to_subtract += ( tree_size[t1] * ( it_cycles->second / 2 ) ) + ( tree_size[t2] * ( it_cycles->second / 2 ) );
					}
				}
			}
		}

		cout << "Tree size: \t" << type_tree_size << " \t total sub: \t" << total_to_subtract << endl;

		double size = type_tree_size - total_to_subtract;
		if( size < 0 ) size = 1;
		tree_size[it->first] = size;

		it->first.print();
		cout << "TREE SIZE: " << size << endl;
		cout << "====================" << endl;
		//cout << endl << endl;
	}
}

Cycle ChenCyclePrediction2::hasCycle( State* state, int lookahead, int depth )
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

			//the cycle is of even length
			if( count % 2 == 0 )
			{
				if( count == 2 ) //special treatment for parent pruning
				{
					int number_children_parent = getNumberChildrenPredecessor( state );
					t = sampler->getObject( state->getState(), number_children_parent, lookahead );

					number_children_parent = getNumberChildrenPredecessor( current );
					t1 = sampler->getObject( current->getState(), number_children_parent, lookahead );
					t1.setLevel( depth );
				}
				else
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


int ChenCyclePrediction2::getNumberChildrenPredecessor( State * state )
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


void ChenCyclePrediction2::printOutput( map<Type, State*>& o )
{
	map<Type, State*>::iterator it = o.begin();
	for( ; it != o.end(); ++it )
	{
		cout << "level: \t" << it->first.getLevel() << " number children \t" << it->first.getH() << " nodes: \t" << it->second->getW() << endl;
	}

	cout << endl << endl;
}

void ChenCyclePrediction2::computeNodesPerType2()
{
	map<Type, State*>::iterator it = output.begin();
	for( ; it != output.end(); ++it )
	{
		Type t = it->first;

		if( states_per_type.find( t ) == states_per_type.end() )
		{
			states_per_type[t] = it->second->getW();
		}
		else
		{
			states_per_type[t] = states_per_type[t] + it->second->getW();
		}
	}
}


void ChenCyclePrediction2::averageNodesPerType(int probes)
{
	map<Type, double>::iterator it1 = states_per_type.begin();
	for( ; it1 != states_per_type.end(); ++it1 )
	{
		states_per_type[it1->first] = it1->second / probes;
	}
}

void ChenCyclePrediction2::printStatePerType()
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

void ChenCyclePrediction2::computeNodesPerType( int i )
{
	map<Type, double>::iterator it1 = states_per_type.begin();
	for( ; it1 != states_per_type.end(); ++it1 )
	{
		if( output.find( it1->first ) == output.end() )
		{
			State * s = new State();
			s->setW( 0 );

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

void ChenCyclePrediction2::computeCyclesPerType()
{
	map<Type, map<Cycle, double> >::iterator outiter = cycles_avg.begin();
	for( ; outiter != cycles_avg.end(); ++outiter )
	{
		map<Cycle, double>::iterator it1 = outiter->second.begin();
		for( ; it1 != outiter->second.end(); ++it1 )
		{
			cycles_per_type[outiter->first][it1->first] = it1->second / states_per_type[outiter->first];
		}
	}
}

void ChenCyclePrediction2::computeBPerType()
{
	map<Type, map<Type, double> >::iterator outiter = B_avg.begin();
	for( ; outiter != B_avg.end(); ++outiter )
	{
		map<Type, double>::iterator it1 = outiter->second.begin();
		for( ; it1 != outiter->second.end(); ++it1 )
		{
			B_per_type[outiter->first][it1->first] = it1->second / states_per_type[outiter->first];
		}
	}
}

void ChenCyclePrediction2::updateBranchingFactor(int i)
{
	map<Type, map<Type, double> >::iterator outiter = B_avg.begin();
	for( ; outiter != B_avg.end(); ++outiter )
	{
		map<Type, double>::iterator it1 = outiter->second.begin();
		for( ; it1 != outiter->second.end(); ++it1 )
		{
			if( B[outiter->first].find( it1->first ) == B[outiter->first].end() )
			{
				B[outiter->first][it1->first] = 0;
			}
		}
	}

	outiter = B.begin();
	for( ; outiter != B.end(); ++outiter )
	{
		map<Type, double>::iterator it = outiter->second.begin();
		for( ; it != outiter->second.end(); ++it )
		{
			if( B_avg[outiter->first].find(it->first) == B_avg[outiter->first].end() )
			{
				B_avg[outiter->first][it->first] = it->second / ( i + 1 );
			}
			else
			{
				B_avg[outiter->first][it->first] = B_avg[outiter->first][it->first] + ( it->second - B_avg[outiter->first][it->first] ) / ( i + 1 );
			}
		}
	}

	B.clear();
}

void ChenCyclePrediction2::updateCycles(int i)
{
	map<Type, map<Cycle, double> >::iterator outiter = cycles_avg.begin();
	for( ; outiter != cycles_avg.end(); ++outiter )
	{
		map<Cycle, double>::iterator it1 = outiter->second.begin();
		for( ; it1 != outiter->second.end(); ++it1 )
		{
			if( cycles[outiter->first].find( it1->first ) == cycles[outiter->first].end() )
			{
				cycles[outiter->first][it1->first] = 0;
			}
		}
	}

/*
	map<Type, map<Cycle, double> >::iterator outiter = cycles_avg.begin();
	for( ; outiter != cycles_avg.end(); ++outiter )
	{
		if( types_sampled.find( outiter->first ) != types_sampled.end() )
		{
			map<Cycle, double>::iterator it = outiter->second.begin();
			for( ; it != outiter->second.end(); ++it )
			{
				if( cycles[outiter->first].find( it->first ) == cycles[outiter->first].end() )
				{
					cycles[outiter->first][it->first] = 0;
				}
			}
		}
	}
*/
	/*
	//map<Type, map<Cycle, double> >::iterator
	outiter = cycles.begin();
	for( ; outiter != cycles.end(); ++outiter )
	{
		if( number_samples.find( outiter->first ) == number_samples.end() )
		{
			number_samples[outiter->first] = 1;
		}
		else
		{
			number_samples[outiter->first] = number_samples[outiter->first] + 1;
		}
	}
*/
	//printCycles(cycles);

	outiter = cycles.begin();
	for( ; outiter != cycles.end(); ++outiter )
	{
		map<Cycle, double>::iterator it = outiter->second.begin();
		for( ; it != outiter->second.end(); ++it )
		{
			if( cycles_avg[outiter->first].find(it->first) == cycles_avg[outiter->first].end() )
			{
				cycles_avg[outiter->first][it->first] = it->second / ( i + 1 );
				//cycles_avg[outiter->first][it->first] = it->second;
			}
			else
			{
				//cycles_avg[outiter->first][it->first] = cycles_avg[outiter->first][it->first] + ( it->second - cycles_avg[outiter->first][it->first] ) / ( number_samples[outiter->first] );
				cycles_avg[outiter->first][it->first] = cycles_avg[outiter->first][it->first] + ( it->second - cycles_avg[outiter->first][it->first] ) / ( i + 1 );
			}
		}
	}

	cycles.clear();
	types_sampled.clear();
}

void ChenCyclePrediction2::printCycles(map<Type, map<Cycle, double> >& c)
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
	map<Type, long>::iterator it_samples = number_samples.begin();
	for(; it_samples != number_samples.end(); ++it_samples )
	{
		cout << "level: " << it_samples->first.getLevel() << " " << it_samples->second << endl;
	}
*/

}

void ChenCyclePrediction2::printB(map<Type, map<Type, double> >& c)
{
	map<Type, map<Type, double> >::iterator outiter = c.begin();
	for( ; outiter != c.end(); ++outiter )
	{
		cout << "TYPE" << endl;
		outiter->first.print();
		cout << endl;

		cout << "BRANCHING FACTOR" << endl;
		map<Type, double>::iterator it = outiter->second.begin();
		for( ; it != outiter->second.end(); ++it )
		{
			it->first.print();
			cout << it->second << endl;
		}

		cout << "================================" << endl;

		cout << endl << endl;
	}
}

void ChenCyclePrediction2::clearOutuput()
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

long ChenCyclePrediction2::computeNumberNodesExpanded()
{
	double nodes_expanded = 0;

	map<Type, double>::iterator it = states_per_type.begin();
	for( ; it != states_per_type.end(); ++it )
	{
		nodes_expanded += it->second;
	}

	return nodes_expanded;
}


#endif /* ChenCyclePrediction2_H_ */
