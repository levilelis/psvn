#ifndef TypeSampler_h__
#define TypeSampler_h__

#include <vector>
#include <string>

#include "Type.h"
#include "../AbstractionHeuristic.h"
#include "../randomc/randomc.h"

using namespace::std;


class TypeSampler {
	private:
		CRandomMersenne * rand_gen;
		int max_samples_per_type;
		AbstractionHeuristic * hf;

		void sample(state_t * state, int parentHeuristic, TypeChildren &children, int lookahead, int currentLevel);
	public:
		TypeSampler(AbstractionHeuristic * hf, int max);
		~TypeSampler();

		Type getObject(state_t * state, int heuristic, int lookahead);
		Type getChenObject(state_t * state, int lookahead);
};

TypeSampler::TypeSampler(AbstractionHeuristic * hf, int max)
{
	rand_gen = new CRandomMersenne( ( unsigned )time( NULL ) );
	this->max_samples_per_type = max;
	this->hf = hf;
}

TypeSampler::~TypeSampler()
{
	delete rand_gen;
	//delete hf;
}

void TypeSampler::sample(state_t * state, int parentHeuristic, TypeChildren& children, int lookahead, int currentLevel)
{
	if(currentLevel == lookahead)
	{
		return;
	}

	int rule_used;
	func_ptr iter;
	state_t child;

	init_fwd_iter( iter );
	while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 )
	{
		apply_fwd_rule( rule_used, state, &child );

		if( compare_states( &child, state ) == 0 )   // parent pruning
		{
			continue;
		}

//		const int move_cost = fwd_rule_costs[ rule_used ];
		int h = hf->abstraction_data_lookup( &child );

		Child c(currentLevel, parentHeuristic, h);
		children.addChild(c);

		sample(&child, h, children, lookahead, currentLevel + 1);
	}
}

Type TypeSampler::getObject(state_t * state, int heuristic, int lookahead)
{
	TypeChildren children;
	int rand_type = rand_gen->IRandom( 1, max_samples_per_type );
	Type obj(rand_type, heuristic);

	sample(state, heuristic, children, lookahead, 0);

	obj.setChildren(children);

	return obj;
}

Type TypeSampler::getChenObject(state_t * state, int lookahead)
{
	int num_children = 0;
	int rule_used;
	func_ptr iter;
	state_t child;

	init_fwd_iter( iter );
	while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 )
	{
		apply_fwd_rule( rule_used, state, &child );
		num_children++;
	}

	//cout << "number of children \t" << num_children << endl;

	int rand_type = rand_gen->IRandom( 1, max_samples_per_type );

	//TypeChildren children;
	Type obj(rand_type, num_children);

	//obj.setChildren(children);

	return obj;
}

#endif

