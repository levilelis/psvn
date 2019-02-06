/*
 * CorruptionDetector.h
 *
 *  Created on: Jan 25, 2015
 *      Author: levilelis
 */

#ifndef CORRUPTIONDETECTOR_H_
#define CORRUPTIONDETECTOR_H_

#include "RadiationSimulator.h"
#include <map>
#include <vector>
#include <set>

using namespace std;

struct CompareState_t
{
  bool operator() (const state_t* lhs, const state_t* rhs) const
  {
	  for(int i = 0; i < NUMVARS; i++)
	  {
		  if((*lhs).vars[i] != (*rhs).vars[i])
		  {
			  return (*lhs).vars[i] < (*rhs).vars[i];
		  }
	  }

	  return false;
  }
};


class CorruptionDetector {

protected:
	RadiationSimulator * radiation;
	AbstractionHeuristic * heuristic;

	bool dfs(const state_t *state, const state_t *parent_state, const int bound, int current_g, int parent_h );
	int hamming_distance(int x, int y);

public:
	CorruptionDetector() {}
	CorruptionDetector(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	virtual bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);
	virtual int getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h);
};

CorruptionDetector::CorruptionDetector(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

/*
 * Default always returns false
 */
bool CorruptionDetector::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	return false;
}

/*
 * The default fix for CorruptionDetector is to return the heuristic value of the parent minus the move cost
 */
int CorruptionDetector::getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h)
{
	int h = parent_h - 1;
	if(h < 0) h = 0;
	heuristic->setHValue(state, h);
	return h;
}

int CorruptionDetector::hamming_distance(int x, int y)
{
	int dist;
	int val;

	dist = 0;
	val = x ^ y;    // XOR

	// Count the number of bits set
	while (val != 0)
	{
		// A bit is set, so increment the count and clear the bit
		dist++;
		val &= val - 1;
	}

	// Return the number of differing bits
	return dist;
}


bool CorruptionDetector::dfs(const state_t *state, const state_t *parent_state, const int bound, int current_g, const int parent_h )
{
    int rule_used;
    func_ptr iter;
    state_t child;

//    radiation->simulate();

    init_fwd_iter( iter );
    while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 ) {

    	apply_fwd_rule( rule_used, state, &child );

    	if( compare_states( &child, parent_state ) == 0 )   // parent pruning
    		continue;

    	const int move_cost = fwd_rule_costs[ rule_used ];
    	int child_h = heuristic->abstraction_data_lookup( &child );

    	if(child_h < parent_h - move_cost || child_h > parent_h + move_cost)
    	{
    	//	cout << "[" << parent_h - move_cost << ", " << child_h << ", " << parent_h + move_cost << ", " << bound << "]" << endl;
    		return true;
    	}

    	if (current_g + move_cost > bound) {
    		continue;
    	} else {
    		if( dfs(&child, state, bound, current_g + move_cost, child_h ) )
    		{
    			return true;
    		}
    	}
    }

    return false;
}

/*
 * Makes the full delta search to detect corruption
 */
class CorruptionDetectorFull : public CorruptionDetector {

public:
	CorruptionDetectorFull(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);

};

CorruptionDetectorFull::CorruptionDetectorFull(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorFull::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	int64_t totalCorruptions = radiation->getTotalCorruptions();

	//cout << "Checking with: " << totalCorruptions << endl;

	//int parent_h = heuristic->abstraction_data_lookup( parent_state );

	if(h < parent_h - 1 || h > parent_h + 1)
		return true;

	return dfs(state, state, totalCorruptions, 0, h);
	//return dfs(state, parent_state, 1, 0, h);
}

/*
 * Looks for inconsistency amongst the children of n to detect corruption of h(n)
 */
class CorruptionDetectorSingle : public CorruptionDetector {

public:
	CorruptionDetectorSingle(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);

};

CorruptionDetectorSingle::CorruptionDetectorSingle(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorSingle::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	int64_t totalCorruptions = radiation->getTotalCorruptions();

	//int parent_h = heuristic->abstraction_data_lookup( parent_state );

	if(h < parent_h - 1 || h > parent_h + 1)
		return true;

	return dfs(state, parent_state, 1, 0, h);
}

/*
 * Uses a voting system amongst the h-values of the children of n to detect whether h(n) is corrupted
 */
class CorruptionDetectorSingleVoting : public CorruptionDetector {

public:
	CorruptionDetectorSingleVoting(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);
	int getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h);


};

CorruptionDetectorSingleVoting::CorruptionDetectorSingleVoting(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorSingleVoting::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{

	//int parent_h = heuristic->abstraction_data_lookup( parent_state );

	if(h < parent_h - 1 || h > parent_h + 1)
	{
		return true;
	}
	return false;

/*
    int rule_used;
    func_ptr iter;
    state_t child;
    int num_inconsistent = 0;
    int num_consistent = 0;

    init_fwd_iter( iter );
    while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 )
    {
    	apply_fwd_rule( rule_used, state, &child );

    	if( compare_states( &child, parent_state ) == 0 )   // parent pruning
    		continue;

    	const int move_cost = fwd_rule_costs[ rule_used ];
    	int child_h = heuristic->abstraction_data_lookup( &child );

    	if(child_h < h - move_cost || child_h > h + move_cost)
    	{
    		num_inconsistent++;
    	}
    	else
    	{
    		num_consistent++;
    	}
    }

    if(num_consistent > num_inconsistent)
    {
    	return false;
    }

    return true;
    */
}



int CorruptionDetectorSingleVoting::getCorrectedValue(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
    int rule_used;
    func_ptr iter;
    state_t child;

    map<int, int> votes; //maps heuristic value to number of votes
    //int h = heuristic->abstraction_data_lookup( state );

    init_fwd_iter( iter );
    while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 )
    {
    	apply_fwd_rule( rule_used, state, &child );

    	if( compare_states( &child, parent_state ) == 0 )   // parent pruning
    		continue;

    	const int move_cost = fwd_rule_costs[ rule_used ];
    	int child_h = heuristic->abstraction_data_lookup( &child );

    	//one vote for each possible heuristic value
    	for(int i = child_h - move_cost; i <= child_h + move_cost; i++)
    	{
    		if(votes.find(i) == votes.end())
    		{
    			votes[i] = 1;
    		}
    		else
    		{
    			votes[i] = votes[i] + 1;
    		}
    	}
    }

    map<int, int>::iterator it_best = votes.begin();
	for(map<int, int>::iterator it = votes.begin(); it != votes.end(); it++)
	{
	//	cout << it->first << "\t\t" << it->second << endl;

		//if it is more voted than it_best than replace
		if(it->second > it_best->second)
		{
			it_best = it;
		}

		//if they are equally voted but it has a lower heuristic value than replace
		if(it->second == it_best->second && hamming_distance(it->first, h) < hamming_distance(it_best->first, h))
		{
			it_best = it;
		}
	}



	int to_replace = it_best->first;

	//cout << "to replace: " << to_replace << endl;

	if(to_replace < 0) to_replace = 0;

	heuristic->setHValue(state, to_replace);

	//return the most voted heuristic value
	return to_replace;
}


/*
 * Uses a voting system amongst the h-values of the children of n to detect whether h(n) is corrupted
 * This corrector is bounded by the 3*C rule
 */
class CorruptionDetectorBoundedVoting : public CorruptionDetector {

public:
	CorruptionDetectorBoundedVoting(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);
	int getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h);


};

CorruptionDetectorBoundedVoting::CorruptionDetectorBoundedVoting(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorBoundedVoting::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	if(h < parent_h - 1 || h > parent_h + 1)
	{
		return true;
	}
	return false;
}

int CorruptionDetectorBoundedVoting::getCorrectedValue(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
    int rule_used;
    func_ptr iter;
    state_t child;

    map<int, int> votes; //maps heuristic value to number of votes
    //int h = heuristic->abstraction_data_lookup( state );

    init_fwd_iter( iter );
    while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 )
    {
    	apply_fwd_rule( rule_used, state, &child );

    	if( compare_states( &child, parent_state ) == 0 )   // parent pruning
    		continue;

    	const int move_cost = fwd_rule_costs[ rule_used ];
    	int child_h = heuristic->abstraction_data_lookup( &child );

    	//one vote for each possible heuristic value
    	for(int i = child_h - move_cost; i <= child_h + move_cost; i++)
    	{
    		if(votes.find(i) == votes.end())
    		{
    			votes[i] = 1;
    		}
    		else
    		{
    			votes[i] = votes[i] + 1;
    		}
    	}
    }

    map<int, int>::iterator it_best = votes.begin();
	for(map<int, int>::iterator it = votes.begin(); it != votes.end(); it++)
	{
	//	cout << it->first << "\t\t" << it->second << endl;

		//if it is more voted than it_best than replace
		if(it->second > it_best->second)
		{
			it_best = it;
		}

		//if they are equally voted but it has a lower heuristic value than replace
		if(it->second == it_best->second && hamming_distance(it->first, h) < hamming_distance(it_best->first, h))
		{
			it_best = it;
		}
	}

	int to_replace = it_best->first;

	if(to_replace < parent_h - 1 || to_replace > parent_h + 1)
	{
		to_replace = parent_h + 1;
	}

	//return the most voted heuristic value (considering the bound provided by the parent)
	return to_replace;

}


/*
 * Uses a voting system amongst the h-values of the children of n to detect whether h(n) is corrupted
 * This corrector is bounded by the 3*C rule
 */
class CorruptionDetectorBoundedVotingUP : public CorruptionDetector {

public:
	CorruptionDetectorBoundedVotingUP(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);
	int getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h);


};

CorruptionDetectorBoundedVotingUP::CorruptionDetectorBoundedVotingUP(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorBoundedVotingUP::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	if(h < parent_h - 1 || h > parent_h + 1)
	{
		return true;
	}
	return false;
}

int CorruptionDetectorBoundedVotingUP::getCorrectedValue(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
    int rule_used;
    func_ptr iter;
    state_t child;

    map<int, int> votes; //maps heuristic value to number of votes
    //int h = heuristic->abstraction_data_lookup( state );

    init_fwd_iter( iter );
    while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 )
    {
    	apply_fwd_rule( rule_used, state, &child );

    	if( compare_states( &child, parent_state ) == 0 )   // parent pruning
    		continue;

    	const int move_cost = fwd_rule_costs[ rule_used ];
    	int child_h = heuristic->abstraction_data_lookup( &child );

    	//one vote for each possible heuristic value
    	for(int i = child_h - move_cost; i <= child_h + move_cost; i++)
    	{
    		if(votes.find(i) == votes.end())
    		{
    			votes[i] = 1;
    		}
    		else
    		{
    			votes[i] = votes[i] + 1;
    		}
    	}
    }

    map<int, int>::iterator it_best = votes.begin();
	for(map<int, int>::iterator it = votes.begin(); it != votes.end(); it++)
	{
	//	cout << it->first << "\t\t" << it->second << endl;

		//if it is more voted than it_best than replace
		if(it->second > it_best->second)
		{
			it_best = it;
		}

		//if they are equally voted but it has a lower heuristic value than replace
		if(it->second == it_best->second && hamming_distance(it->first, h) < hamming_distance(it_best->first, h))
		{
			it_best = it;
		}
	}

	int to_replace = it_best->first;

	if(to_replace < parent_h - 1 || to_replace > parent_h + 1)
	{
		to_replace = parent_h + 1;
	}

	heuristic->setHValue(state, to_replace);

	//return the most voted heuristic value (considering the bound provided by the parent)
	return to_replace;

}

class CorruptionDetectorParent : public CorruptionDetector {

public:
	CorruptionDetectorParent(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);
	int getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h);
};

CorruptionDetectorParent::CorruptionDetectorParent(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorParent::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	if(h < parent_h - 1 || h > parent_h + 1)
	{
		return true;
	}
	return false;
}

int CorruptionDetectorParent::getCorrectedValue(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
    int rule_used;
    func_ptr iter;
    state_t child;

    set<int> possible_h; //possible heuristic values *state can assume
	possible_h.insert(parent_h - 1);
	possible_h.insert(parent_h + 1);
	possible_h.insert(parent_h);

   // int h = heuristic->abstraction_data_lookup( state );

    int closest = INT_MAX;

    set<int>::iterator it = possible_h.begin();
    int to_replace = *it;
	for(set<int>::iterator it = possible_h.begin(); it != possible_h.end(); it++)
	{
		int dist = hamming_distance(*it, h);

		if(dist < closest)
		{
			closest = dist;
			to_replace = *it;
		}
	}

	if(to_replace < 0) to_replace = 0;

	heuristic->setHValue(state, to_replace);

	//return the most voted heuristic value
	return to_replace;
}


class CorruptionDetectorBoundedParentUP : public CorruptionDetector {

public:
	CorruptionDetectorBoundedParentUP(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);
	int getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h);
};

CorruptionDetectorBoundedParentUP::CorruptionDetectorBoundedParentUP(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorBoundedParentUP::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	if(h < parent_h - 1 || h > parent_h + 1)
	{
		return true;
	}
	return false;
}

int CorruptionDetectorBoundedParentUP::getCorrectedValue(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	heuristic->setHValue(state, parent_h + 1);

	return parent_h + 1;
}

class CorruptionDetectorBoundedParent : public CorruptionDetector {

public:
	CorruptionDetectorBoundedParent(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);
	int getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h);
};

CorruptionDetectorBoundedParent::CorruptionDetectorBoundedParent(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorBoundedParent::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	if(h < parent_h - 1 || h > parent_h + 1)
	{
		return true;
	}
	return false;
}

int CorruptionDetectorBoundedParent::getCorrectedValue(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	return parent_h + 1;
}

class CorruptionDetectorBoundedParentMinus : public CorruptionDetector {

public:
	CorruptionDetectorBoundedParentMinus(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);
	int getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h);
};

CorruptionDetectorBoundedParentMinus::CorruptionDetectorBoundedParentMinus(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorBoundedParentMinus::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	if(h < parent_h - 1 || h > parent_h + 1)
	{
		return true;
	}
	return false;
}

int CorruptionDetectorBoundedParentMinus::getCorrectedValue(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	if(parent_h - 1 > 0)
	{
		return parent_h - 1;
	}
	return 0;
}


class CorruptionDetectorBoundedParentHamming : public CorruptionDetector {

public:
	CorruptionDetectorBoundedParentHamming(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);
	int getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h);
};

CorruptionDetectorBoundedParentHamming::CorruptionDetectorBoundedParentHamming(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorBoundedParentHamming::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	if(h < parent_h - 1 || h > parent_h + 1)
	{
		return true;
	}
	return false;
}

int CorruptionDetectorBoundedParentHamming::getCorrectedValue(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	int dist_minus = hamming_distance(h, parent_h - 1);
	int dist_same = hamming_distance(h, parent_h);
	int dist_plus = hamming_distance(h, parent_h + 1);

	int closest = dist_minus;
	int h_to_replace = parent_h - 1;

	if(dist_same < closest || dist_same == closest)
	{
		closest = dist_same;
		h_to_replace = parent_h;
	}

	if(dist_plus < closest || dist_plus == closest)
	{
		closest = dist_plus;
		h_to_replace = parent_h + 1;
	}
/*
	cout << "Distance from " << parent_h - 1 << " to " << h  << ": \t" << dist_minus << endl;
	cout << "Distance " << parent_h << " to " << h << ": \t" << dist_same << endl;
	cout << "Distance " << parent_h + 1 << " to " << h << ": \t" << dist_plus << endl;
	cout << "Distance returned: \t" << h_to_replace << endl;
	cout << endl;
*/
	return h_to_replace;
}



/*
 * Uses a voting system amongst the h-values of the children of n to detect whether h(n) is corrupted
 */
class CorruptionDetectorVotingAbstraction : public CorruptionDetector {

public:
	CorruptionDetectorVotingAbstraction(RadiationSimulator * radiation, AbstractionHeuristic * heuristic);
	bool isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h);
	int getCorrectedValue(const state_t *state, const state_t *parent_state, int child_h, int parent_h);


};

CorruptionDetectorVotingAbstraction::CorruptionDetectorVotingAbstraction(RadiationSimulator * radiation, AbstractionHeuristic * heuristic)
{
	this->heuristic = heuristic;
	this->radiation = radiation;
}

bool CorruptionDetectorVotingAbstraction::isCorrupted(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
	//int parent_h = heuristic->abstraction_data_lookup( parent_state );
	if(h < parent_h - 1 || h > parent_h + 1)
	{
		return true;
	}
	return false;
}

int CorruptionDetectorVotingAbstraction::getCorrectedValue(const state_t *state, const state_t *parent_state, int h, int parent_h)
{
    int rule_used;
    func_ptr iter;

    map<int, int> votes; //maps heuristic value to number of votes
    //int h = heuristic->abstraction_data_lookup( state );
    set<state_t*, CompareState_t> abstracted_states_voted;
    vector<state_t*> to_clean;

    init_fwd_iter( iter );
    while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 )
    {
    	state_t child;
    	apply_fwd_rule( rule_used, state, &child );

    	if( compare_states( &child, parent_state ) == 0 )   // parent pruning
    		continue;

    	state_t * abstract_child = new state_t();
    	heuristic->get_abstracted_state(&child, abstract_child);

    	if(abstracted_states_voted.find(abstract_child) != abstracted_states_voted.end())
    	{
    		delete abstract_child;
    		continue;
    	}

    	abstracted_states_voted.insert(abstract_child);
    	to_clean.push_back(abstract_child);

    	const int move_cost = fwd_rule_costs[ rule_used ];
    	int child_h = heuristic->abstraction_data_lookup( &child );

    	//one vote for each possible heuristic value
    	for(int i = child_h - move_cost; i <= child_h + move_cost; i++)
    	{
    		if(votes.find(i) == votes.end())
    		{
    			votes[i] = 1;
    		}
    		else
    		{
    			votes[i] = votes[i] + 1;
    		}
    	}
    }

    for(int i = 0; i < to_clean.size(); i++)
    	delete to_clean[i];

//    for(set<state_t*, CompareState_t>::iterator it = abstracted_states_voted.begin(); it != abstracted_states_voted.end(); it++) {
//    	delete *it;
 //   }

    map<int, int>::iterator it_best = votes.begin();
	for(map<int, int>::iterator it = votes.begin(); it != votes.end(); it++)
	{
	//	cout << it->first << "\t\t" << it->second << endl;

		//if it is more voted than it_best than replace
		if(it->second > it_best->second)
		{
			it_best = it;
		}

		//if they are equally voted but it has a lower heuristic value than replace
		if(it->second == it_best->second && hamming_distance(it->first, h) < hamming_distance(it_best->first, h))
		{
			it_best = it;
		}
	}

	int to_replace = it_best->first;

	if(to_replace < parent_h - 1 || to_replace > parent_h + 1)
	{
		to_replace = parent_h + 1;
	}

	if(to_replace < 0)
	{
		to_replace = 0;
	}

	//return the most voted heuristic value
	return to_replace;
}


#endif /* CORRUPTIONDETECTOR_H_ */
