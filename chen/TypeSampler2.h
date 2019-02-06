#ifndef TypeSampler2_h__
#define TypeSampler2_h__

#include <vector>
#include <string>

#include "Type.h"
#include "../AbstractionHeuristic.h"
#include "../randomc/randomc.h"

using namespace::std;


class TypeSampler2 {
	private:
		CRandomMersenne * rand_gen;
		int max_samples_per_type;

		void sample( state_t * state, TypeChildren& children, int lookahead, int currentLevel );

	public:
		TypeSampler2( int max );
		~TypeSampler2();

		Type getObject( state_t * state, int num_children_parent, int lookahead );
};

TypeSampler2::TypeSampler2( int max )
{
	rand_gen = new CRandomMersenne( ( unsigned )time( NULL ) );
	//rand_gen = new CRandomMersenne( 1234 );
	max_samples_per_type = max;
}

TypeSampler2::~TypeSampler2()
{
	delete rand_gen;
}

void TypeSampler2::sample( state_t * state, TypeChildren& children, int lookahead, int currentLevel )
{
	int rule_used;
	func_ptr iter;
	state_t child;

	int number_children = 0;

	init_bwd_iter( iter );
	while( ( rule_used = next_bwd_iter( iter, state ) ) >= 0 )
	{
/*
		apply_bwd_rule( rule_used, state, &child );

		if( compare_states( &child, state ) == 0 )   // parent pruning
		{
			continue;
		}
*/
		number_children++;

		if( currentLevel + 1 < lookahead )
		{
			sample(&child, children, lookahead, currentLevel + 1);
		}
	}

	Child c( currentLevel, -1, number_children );
	children.addChild( c );
}

Type TypeSampler2::getObject( state_t * state, int num_children_parent, int lookahead )
{
	//Type obj2(-1, -1);
	//return obj2;
/*
	int num_children = 0;
	int rule_used;
	func_ptr iter;
	state_t child;

	init_bwd_iter( iter );
	while( ( rule_used = next_bwd_iter( iter, state ) ) >= 0 )
	{
		/*
		apply_bwd_rule( rule_used, state, &child );

		if( compare_states( &child, state ) == 0 )   // parent pruning
		{
			continue;
		}
*/
	/*
		num_children++;
	}*/

	int rand_type = rand_gen->IRandom( 1, max_samples_per_type );

	Type obj( rand_type, 1/*num_children*/ );
	//Type obj( -1, 1/*num_children*/ );

	TypeChildren children;

	if( lookahead > 0 )
	{
		sample( state, children, lookahead, 0 );
	}

	obj.setChildren( children );

	return obj;
}


#endif

