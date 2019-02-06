/*
 * ChenCyclePrediction.h
 *
 *  Created on: Nov 5, 2013
 *      Author: levilelis
 */

#ifndef ChenCyclePrediction_H_
#define ChenCyclePrediction_H_

#include "../Type.h"
#include "../TypeSampler2.h"
#include "../State.h"
#include "SimpleCycle.h"

#include "../../randomc/randomc.h"
//#include "../../randomc/mersenne.cpp"

#include <time.h>
#include <map>
#include <set>
#include <stack>

class ChenCyclePrediction {

private:
	TypeSampler2 * sampler;
	CRandomMersenne * RanGen;
	map<Type, State*> output;
	map<SimpleCycle, double> cycles;
	map<SimpleCycle, double> cycles_avg;
	int depth_current_node;
	bool isTransposition;
	State* root;
	int MAX_LEVEL;
	vector<const char*> current_path;

	bool probe( State* state, int depth, int lookahead );
	void clearOutuput();
	int hasCycle( State* state, int depth );
	void updateCycles(int i);
	void printCycles(map<SimpleCycle, double>& c);
	void printOutput( map<Type, State*>& o );
	vector<const char*> getPath( State* state, int level );
	vector<const char*> getFlippedPath( State* state );
	int compare_paths( vector<const char*>& p1 ); // compares some path with the current path

public:
	ChenCyclePrediction( int max_nodes_per_type );
	~ChenCyclePrediction();
	bool predict( State* state, int depth, int probes, int lookahead );
};

ChenCyclePrediction::ChenCyclePrediction( int max_nodes_per_type )
{
	MAX_LEVEL = 100;
	this->sampler = new TypeSampler2( max_nodes_per_type );
	srand ( ( unsigned )time( NULL ) );
	RanGen = new CRandomMersenne( ( unsigned )time( NULL ) );
	//RanGen = new CRandomMersenne( 1234 );
}

ChenCyclePrediction::~ChenCyclePrediction()
{
	delete sampler;
	delete RanGen;
}

bool ChenCyclePrediction::predict( State* state, int depth, int probes, int lookahead )
{
	double totalPrediction = 0;
	double totalPredictionGenerated = 0;
	isTransposition = false;
	depth_current_node = depth;
	root = state;

	cycles.clear();
	cycles_avg.clear();

	//cout << "###CURRENT PATH###" << endl;
	current_path = getFlippedPath( state );
	//char ss[100];
	//sprint_state(ss, 100, state->getState());
	//cout << ss << endl;
	//for( int i = 0; i < current_path.size(); i++ )
	//{
	//	cout << current_path[ i ] << endl;
	//}
	//cout << endl << endl;

	for( int i = 0; i < probes; i++ )
	{
		//cout << "PROBING FOR CYCLES" << endl;

		clearOutuput();
		if( probe( state, depth, lookahead ) )
		{
			//cout << "ITS IS A TRANSPOSITION" << endl << endl;
			return true;
		}

	//	updateCycles( i );
	}

//	cout << "======================================" << endl;
//	cout << "Total number of cycles in the tree: " << endl;
//	printCycles(cycles_avg);
//	cout << "======================================" << endl;
//	cout << endl << endl;

//	return cycles_avg;

//	cout << "IT IS NOT A TRANSPOSITION" << endl << endl;

	return false;
}

bool ChenCyclePrediction::probe( State* state, int depth, int lookahead )
{
	map<Type, State*> queue;

	Type object = sampler->getObject( state->getState(), -1, lookahead );

	object.setLevel( 0 );
	state->setW( 1 );
	queue.insert( pair<Type, State*>( object, state ) );

	while( !queue.empty() )
	{
		//cout << "QUEUE SIZE: " << queue.size() << endl;

		Type out = queue.begin()->first;
		State* s = queue.begin()->second;
		queue.erase( out );

		int g = out.getLevel();
		double w = s->getW();

	//	if( hasCycle( s, out.getLevel() + depth ) )
	//	{
	//		return true;
	//	}

		/*
		//it has found that the state is already a transposition
		//no more work is required
		if( isTransposition )
		{
			cycles_avg.clear();
			cycles_avg[ cycle ] = 1;
			return;
		}

		if( cycle.getL() > 0 )
		{
			if( cycles.find( cycle ) != cycles.end() )
			{
				cycles[cycle] = cycles[cycle] + w;
			}
			else
			{
				cycles[cycle] = w;
			}

			output.insert( pair<Type, State*> ( out, s ) );
			continue;
		}*/

		output.insert( pair<Type, State*> ( out, s ) );

		int rule_used;
		func_ptr iter;

		init_bwd_iter( iter );
		while( ( rule_used = next_bwd_iter( iter, s->getState() ) ) >= 0 )
		{
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

			//cout << "CHILD BEING EXPANDED WITH OP \t" << bwd_rule_names[rule_used] << " and g: \t" << g + 1 << endl;
			//char ss[100];
			//sprint_state(ss, 100, child->getState());
			//cout << ss << endl;

			int res = hasCycle( child, out.getLevel() + depth + 1 );

			if( res == 0 ) //proved the state to be a transposition
			{
				delete child;
				return true;
			}
			if( res == 1 ) //found a cycle in child but did not prove the state to be a transposition
			{
				delete child;
				continue;
			}

			Type object = sampler->getObject( child->getState(), -1, lookahead );
			object.setLevel( g + 1 );

			int d;
			if( depth < MAX_LEVEL )
			{
				d = depth;
			}
			else
			{
				d = MAX_LEVEL;
			}

			//the case in which depth < 0 represents a search without a cost bound
			if( g + 1 <= d/*depth*/ )
			{
				map<Type, State*>::iterator queueIt = queue.find( object );
				if( queueIt != queue.end() )
				{
					double wa = queueIt->second->getW();
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

	return false;
}

int ChenCyclePrediction::compare_paths( vector<const char*>& p1 )
{
	int diff = current_path.size() - p1.size();
//	cout << "path length: " << p1.size() << " current path length: " << current_path.size() << " diff: " << diff << endl;

	for( int i = 0; i < p1.size(); i++ )
	{
//		cout << "Comparing: \t" << p1[i] << " \t" << current_path[i + diff] << endl;

		int cmp = strcmp( p1[i], current_path[i + diff] );

		if( cmp != 0 )
		{
			return cmp;
		}
	}

	//cout << endl << endl;

	return 0;
}

vector<const char*> ChenCyclePrediction::getFlippedPath( State* state )
{
//	cout << "IN GETPATH levels up: " << level << endl;

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

//	cout << "PATH GATHERED: ";

//	for( int i = 0; i < path.size(); i++ )
//	{
//		cout << path[ i ] << ", ";
//	}
//	cout << endl;


	return path;
}


vector<const char*> ChenCyclePrediction::getPath( State* state, int level )
{
//	cout << "IN GETPATH levels up: " << level << endl;

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

//			path.push_back( bwd_rule_names[ state->getRule() ] );
			state = state->getPred();
		}
	}

//	cout << "PATH GATHERED: ";

//	for( int i = 0; i < path.size(); i++ )
//	{
//		cout << path[ i ] << ", ";
//	}
//	cout << endl;


	return path;
}

/*
 * Returns:
 * 0 - if state is a transposition
 * 1 - if a cycle was found but state is not a transposition
 * 2 - if a cycle was not found
 */
int ChenCyclePrediction::hasCycle( State* state, int depth )
{
	State* current = state;
	int count = 0;
	SimpleCycle cycle(-1, false, false);

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
					//cout << "IT IS A TRANSPOSITION 1" << endl;
					return 0;
				}

				if( transposition_level > depth_current_node )
				{
					return 1;
				}

				//cout << "CALLING GETPATH depth: " << depth << " transposition level: " << transposition_level << endl;
				vector<const char*> path = getPath( current, depth - transposition_level );

				//cout << "CYCLE LENGTH: " << count << " TRANSPOSITION LEVEL: " << transposition_level << endl;
				int cmp = compare_paths( path );
				if( cmp < 0 )
				{
					//cout << "IT IS A TRANSPOSITION 2" << endl;
					return 0;
				}

				if( cmp == 0 )
				{
					cout << "ERROR: CMP EQUALS ZERO!" << endl;
				}

				/*
				//cout << "PRINTING CYCLE: " << endl;
				state = current;
				for( int i = 0; i < depth - transposition_level; i++ )
				{
					//char ss[100];
					//sprint_state(ss, 100, state->getState());
					//cout << ss << endl;

					state = state->getPred();
				}

				//cout << endl << endl;

				cycle.setState( state );

				if( transposition_level < depth_current_node )
				{
					cycle.setIsTransposition( true );
					this->isTransposition = true;
					cycle.setL( count );
					return cycle;
				}
				if( transposition_level == depth_current_node )
				{
				//	if( compare_states( root->getState(), state->getState() ) != 0 )   // parent pruning
				//	{
				//		cout << "DIFFERENT STATE!" << endl;
				//	}

					cycle.setIsDuplicate( true );
				}
				*/
			}
			else //cycle is of odd length
			{
				int transposition_level = depth - ( ( count - 1 ) / 2 );
				if( transposition_level <= depth_current_node )
				{
					return 0;
				}

			//	cout << "ODD CYCLE" << endl;
			//	cout << "CURRENT NODE: " << depth_current_node << endl;
			//	cout << "DEPTH: " << depth << endl;
			//	cout << "CYCLE LENGTH: " << count << endl;
			//	cout << "TRANSPOSITION LEVEL: " << transposition_level << endl;
			//	cout << endl << endl;
/*
				state = current;
				for( int i = 0; i < transposition_level; i++ )
				{
					state = state->getPred();
				}

				cycle.setState( state );
				if( transposition_level <= depth_current_node )
				{
					cycle.setIsTransposition( true );
					this->isTransposition = true;
					cycle.setL( count );
					return cycle;
				}
				//if( transposition_level == depth_current_node )
				//{
				//	cycle.setIsTransposition( true );
					//cout << "SOMETHING MIGHT BE WRONG!!" << endl;
			//	}
			 *
			 */
			}

			//cycle.setDepth( depth );
			//cycle.setL( count );
			//return cycle;
			return 1;
		}
	}

	return 2;
}

void ChenCyclePrediction::printOutput( map<Type, State*>& o )
{
	map<Type, State*>::iterator it = o.begin();
	for( ; it != o.end(); ++it )
	{
		cout << "level: \t" << it->first.getLevel() << " number children \t" << it->first.getH() << " nodes: \t" << it->second->getW() << endl;
	}

	cout << endl << endl;
}

void ChenCyclePrediction::updateCycles(int i)
{
	map<SimpleCycle, double>::iterator outiter = cycles_avg.begin();
	for( ; outiter != cycles_avg.end(); ++outiter )
	{
		if( cycles.find( outiter->first ) == cycles.end() )
		{
			cycles[outiter->first] = 0;
		}
	}

	outiter = cycles.begin();
	for( ; outiter != cycles.end(); ++outiter )
	{
		if( cycles_avg.find(outiter->first) == cycles_avg.end() )
		{
			cycles_avg[outiter->first] = outiter->second / ( i + 1 );
		}
		else
		{
			cycles_avg[outiter->first] = cycles_avg[outiter->first] + ( outiter->second - cycles_avg[outiter->first] ) / ( i + 1 );
		}
	}

	cycles.clear();
}

void ChenCyclePrediction::printCycles(map<SimpleCycle, double>& c)
{
	cout << "CYCLES" << endl;
	map<SimpleCycle, double>::iterator it = c.begin();
	for( ; it != c.end(); ++it )
	{
		cout << "<" << it->first.getL() << ", " << it->second << "> " << endl;
	}

	//cout << endl << endl;
}

void ChenCyclePrediction::clearOutuput()
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


#endif /* ChenCyclePrediction_H_ */
