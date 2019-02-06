/*
Copyright (C) 2013 by the PSVN Research Group, University of Alberta
 */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/time.h>
#include "../AbstractionHeuristic.h"
#include "RadiationSimulator.h"
#include "CorruptionDetector.h"


/* if MOVE_PRUNING is undefined, parent pruning is done.
   if MOVE_PRUNING is defined, parent pruning is not done.*/
//#define MOVE_PRUNING

#define myMAX(x,y) (((x)>(y))?(x):(y))
#define myMIN(x,y) (((x)<(y))?(x):(y))


/* GLOBAL VARIABLES */
int64_t nodes_expanded_for_bound ;    // number of nodes expanded for a given cost bound
int64_t nodes_generated_for_bound ;   // number of nodes generated for a given cost bound
int64_t nodes_expanded_for_startstate ;   // number of nodes expanded until solution found for a given start state
int64_t nodes_generated_for_startstate ;  // number of nodes generated until solution found for a given start state
int best_soln_sofar = INT_MAX;


int dfs_heur( AbstractionHeuristic * heuristic, RadiationSimulator * radiation, CorruptionDetector * detection,
		const state_t *state,
		const state_t *parent_state, // for parent pruning
		const int bound, int *next_bound, int current_g, int state_h )
{
	int rule_used;
	func_ptr iter;
	state_t child;

	//cout << current_g << endl;

	nodes_expanded_for_bound++;
	radiation->simulate();

	init_fwd_iter( iter );
	while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 ) {

		apply_fwd_rule( rule_used, state, &child );
		nodes_generated_for_bound++;

		if( compare_states( &child, parent_state ) == 0 )   // parent pruning
			continue;

		const int move_cost = fwd_rule_costs[ rule_used ];

		if (is_goal(&child)) {
			best_soln_sofar = myMIN(best_soln_sofar, current_g + move_cost);
			if (best_soln_sofar <= bound) {
				return 1;
			} else {
				continue;
			}
		} else {
			int child_h = heuristic->abstraction_data_lookup( &child );

			if(detection->isCorrupted(&child, state, child_h, state_h)) {
		//		cout << "replacing: \t" << child_h << " by: \t";
				child_h = detection->getCorrectedValue(&child, state, child_h, state_h);
		//		cout << child_h << endl;
			}

			if (current_g + move_cost + child_h > bound) {
				*next_bound = myMIN( *next_bound, current_g + move_cost + child_h );
			} else {
				if( dfs_heur( heuristic, radiation, detection, &child, state, bound, next_bound, current_g + move_cost, child_h ) )
				{
					return 1;
				}
			}
		}
	}
	assert( *next_bound > bound );
	return 0;
}



int idastar( AbstractionHeuristic * heuristic, RadiationSimulator * radiation, CorruptionDetector * detection, const state_t *state )
{
	int next_bound, bound, done;

	nodes_expanded_for_startstate  = 0;
	nodes_generated_for_startstate = 0;

	if (is_goal(state)) { return 0; }

	bound = heuristic->abstraction_data_lookup( state ); // initial bound = h(start)
	int h_start = bound;

	best_soln_sofar = INT_MAX;
	while (1) {
		next_bound = INT_MAX;
		nodes_expanded_for_bound  = 0;
		nodes_generated_for_bound = 0;
		done = dfs_heur( heuristic, radiation, detection, state,
				state,         // parent pruning
				bound, &next_bound, 0, h_start );
		printf( "bound: %d, expanded: %" PRId64 ", generated: %" PRId64 "\n", bound, nodes_expanded_for_bound, nodes_generated_for_bound );
		nodes_expanded_for_startstate  += nodes_expanded_for_bound;
		nodes_generated_for_startstate += nodes_generated_for_bound;
		if( done ) {
			break;
		}
		assert( next_bound > bound );
		bound = next_bound;
		if ( best_soln_sofar <= bound ) { // will always be true if bound == INT_MAX
			break;
		}
	}

	return best_soln_sofar;
}



int main( int argc, char **argv )
{
	state_t state;
	int64_t total_expanded;  // the total number of nodes expanded over all the start states
	int64_t total_generated; // the total number of nodes generated over all the start states
	int trials, d, total_d;
	//abstraction_data_t* abst;

	char line[ 4096 ];
	//struct timeval start, end, total;
	//total.tv_sec = 0;
	//total.tv_usec = 0;

	AbstractionHeuristic * heuristic;
	RadiationSimulator * radiation;
	CorruptionDetector * detection;

	if( argc < 3 ) {
		printf("There must at least 2 arguments, pdb, corruption frequency, correction strategy\n");
		return EXIT_FAILURE;
	} else {
		heuristic = new AbstractionHeuristic();
		heuristic->read_abstraction_data( argv[1] );
		if (heuristic == NULL) {
			return EXIT_FAILURE;
		}

		int frequency = atoi(argv[2]);

		radiation = new RadiationSimulator(heuristic->getStateMap(), frequency);

		if(argc == 4)
		{
			if(strcmp(argv[3], "Vote") == 0)
			{
				detection = new CorruptionDetectorSingleVoting(radiation, heuristic);
			}

			if(strcmp(argv[3], "BoundedVote") == 0)
			{
				detection = new CorruptionDetectorBoundedVoting(radiation, heuristic);
			}


			if(strcmp(argv[3], "BoundedVoteUP") == 0)
			{
				detection = new CorruptionDetectorBoundedVotingUP(radiation, heuristic);
			}

			if(strcmp(argv[3], "Full") == 0)
			{
				detection = new CorruptionDetectorFull(radiation, heuristic);
			}

			if(strcmp(argv[3], "Single") == 0)
			{
				detection = new CorruptionDetectorSingle(radiation, heuristic);
			}

			if(strcmp(argv[3], "Parent") == 0)
			{
				detection = new CorruptionDetectorParent(radiation, heuristic);
			}

			if(strcmp(argv[3], "Bounded") == 0)
			{
				detection = new CorruptionDetectorBoundedParent(radiation, heuristic);
			}

			if(strcmp(argv[3], "BoundedHamming") == 0)
			{
				detection = new CorruptionDetectorBoundedParentHamming(radiation, heuristic);
			}

			if(strcmp(argv[3], "BoundedMinus") == 0)
			{
				detection = new CorruptionDetectorBoundedParentMinus(radiation, heuristic);
			}

			if(strcmp(argv[3], "BoundedUP") == 0)
			{
				detection = new CorruptionDetectorBoundedParentUP(radiation, heuristic);
			}

			if(strcmp(argv[3], "BoundedVoteAbstraction") == 0)
			{
				detection = new CorruptionDetectorVotingAbstraction(radiation, heuristic);
			}

			if(strcmp(argv[3], "None") == 0)
			{
				detection = new CorruptionDetector(radiation, heuristic);
			}
		}
		else
		{
			detection = new CorruptionDetectorSingle(radiation, heuristic);
		}
	}

	total_d = 0;
	total_expanded = 0;
	total_generated = 0;

	long start, end;
	//printf("Enter a start state (empty input line to quit####): ");
	for( trials = 0;
			fgets( line, 4096, stdin ) != NULL
					&& read_state( line, &state ) > 0;
			++trials ) {

		//printf( "problem %d: ", trials + 1 );
		//print_state( stdout, &state );
		//printf( "\n" );
		//gettimeofday( &start, NULL );

		radiation->reset();

		start = clock();
		d = idastar( heuristic, radiation, detection, &state );
		end = clock();
/*
		gettimeofday( &end, NULL );
		end.tv_sec -= start.tv_sec;
		end.tv_usec -= start.tv_usec;
		if( end.tv_usec < 0 ) {
			end.tv_usec += 1000000;
			--end.tv_sec;
		}

		if (end.tv_usec + total.tv_usec >= 1000000) {
			total.tv_usec = end.tv_usec + total.tv_usec - 1000000;
			total.tv_sec = total.tv_sec + 1;
		} else {
			total.tv_usec = end.tv_usec + total.tv_usec;
		}
		total.tv_sec = total.tv_sec + end.tv_sec;
*/
		if ( d == INT_MAX ) {
			printf( "no solution found. expanded nodes: %" PRId64 ", generated nodes: %" PRId64 "\n",
					nodes_expanded_for_startstate, nodes_generated_for_startstate );
		} else {
			printf( "cost: %d, expanded: %" PRId64 ", generated: %" PRId64 "\n",
					d, nodes_expanded_for_startstate, nodes_generated_for_startstate );
		}
		total_d += d;
		total_expanded  += nodes_expanded_for_startstate;
		total_generated += nodes_generated_for_startstate;

		//printf("Enter a start state (empty input line to quit): ");
	}


	printf( "total cost: %d\ntotal expanded: %" PRId64 "\ntotal generated: %" PRId64 "\ntotal time: %f\n",
			total_d, total_expanded, total_generated, ((float) end - start) / CLOCKS_PER_SEC );

	//destroy_abstraction_data( abst );

	/* delete detection;
    delete radiation;
    delete heuristic;
	 */
	return EXIT_SUCCESS;
}



