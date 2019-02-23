/*
Copyright (C) 2013 by the PSVN Research Group, University of Alberta
*/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/time.h>
#include <math.h>
#include "../AbstractionHeuristic.h"


/* if MOVE_PRUNING is undefined, parent pruning is done.
   if MOVE_PRUNING is defined, parent pruning is not done.*/
//#define MOVE_PRUNING

#define myMAX(x,y) (((x)>(y))?(x):(y))
#define myMIN(x,y) (((x)<(y))?(x):(y))


/* GLOBAL VARIABLES */
int64_t nodes_expanded_for_bound ;    // number of nodes expanded for a given cost bound
int64_t leaf_nodes_for_bound ;    // number of leaf nodes for a given cost bound
int64_t nodes_generated_for_bound ;   // number of nodes generated for a given cost bound
int64_t nodes_expanded_for_startstate ;   // number of nodes expanded until solution found for a given start state
int64_t nodes_generated_for_startstate ;  // number of nodes generated until solution found for a given start state
int best_soln_sofar = INT_MAX;
long budget = 0;
/*
int ida_dfs( const AbstractionHeuristic * heuristic,
              const state_t *state,
              const state_t *parent_state, // for parent pruning
              const long bound, long *next_bound, long current_g )
{
    int rule_used;
    func_ptr iter;
    state_t child;

    nodes_expanded_for_bound++;

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

            if (current_g + move_cost + child_h > bound) {
               *next_bound = myMIN( *next_bound, current_g + move_cost + child_h );
            } else {
               if( dfs_heur( heuristic, &child, state,      // parent pruning
                             bound, next_bound, current_g + move_cost ) )
               {
                   return 1;
               }
            }
        }
    }
    assert( *next_bound > bound );
    return 0;
}*/

int dfs_heur( const AbstractionHeuristic * heuristic,
              const state_t *state,
              const state_t *parent_state, // for parent pruning
              const long bound, long *next_bound, long current_g, int optimal )
{
    int rule_used;
    func_ptr iter;
    state_t child;

    nodes_expanded_for_bound++;

    init_fwd_iter( iter );
    while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 ) {
        apply_fwd_rule( rule_used, state, &child );
        nodes_generated_for_bound++;

        if( compare_states( &child, parent_state ) == 0 )   // parent pruning
            continue;

        const int move_cost = fwd_rule_costs[ rule_used ];

        if (is_goal(&child) && current_g + move_cost <= bound) {
            best_soln_sofar = myMIN(best_soln_sofar, current_g + move_cost);
            if(!optimal)
            	return 1;
        } else {
            int child_h = heuristic->abstraction_data_lookup( &child );
            int child_f = current_g + move_cost + child_h;

            if (current_g + move_cost + child_h > bound) {
               *next_bound = myMIN( *next_bound, current_g + move_cost + child_h );
            } else if(child_f < best_soln_sofar) {
            	budget -= 1;
                if (budget == 0) {
                	return -1; //out of search budget
                }

                int res = dfs_heur( heuristic, &child,
                        state,      // parent pruning
                        bound, next_bound, current_g + move_cost, optimal );
                if (res == -1)
                	return -1; //out of search budget
                if (res == 1)
                	return 1; //finding suboptimal solutions
            }
        }
    }

    return 0;
}

int zoomer( const AbstractionHeuristic * heuristic, const state_t *state )
{
	if (is_goal(state)) { return 0; }

	long bound;
	int done;

    best_soln_sofar = INT_MAX;
    nodes_expanded_for_startstate  = 0;
    nodes_generated_for_startstate = 0;
    nodes_expanded_for_bound  = 0;
    nodes_generated_for_bound = 0;

    long upper;
    long up_min = INT_MAX;
    long lower = heuristic->abstraction_data_lookup( state );

    budget = INT_MAX; //infinity search budget
    if(dfs_heur(heuristic, state, state, lower, &up_min, 0, 0)) //regular IDA* search, no budget
    	return best_soln_sofar;

    nodes_expanded_for_startstate  += nodes_expanded_for_bound;
    nodes_generated_for_startstate += nodes_generated_for_bound;

    long N0 = nodes_expanded_for_bound;

    int k = 0;
    while (1) {
    	k += 1;
    	upper = INT_MAX;
    	long bound = -1;

    	while(upper > up_min) {
    		if (upper == INT_MAX)
    			bound = lower * 2;
    		else
    			bound = (upper + lower)/2;
    		bound = myMAX(bound, up_min);

            nodes_expanded_for_bound  = 0;
            nodes_generated_for_bound = 0;
            long t = INT_MAX;

            budget = N0 * pow(2, k);
            done = dfs_heur( heuristic, state, state, bound, &t, 0, 1 );

            nodes_expanded_for_startstate  += nodes_expanded_for_bound;
            nodes_generated_for_startstate += nodes_generated_for_bound;

            //A solution was found within the bound and it was proven to be optimal
            if ( best_soln_sofar <= bound && done != -1)
            	return best_soln_sofar;
            if( done == -1) { //We have exhausted the search budget
            	upper = bound;
            } else {
            	lower = bound;
            	up_min = t;
            }
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
    struct timeval start, end, total;
    total.tv_sec = 0;
    total.tv_usec = 0;

    AbstractionHeuristic * heuristic;

    if( argc != 2 ) {
        printf("There must 1 command line argument, the prefix of the abstraction and pattern database to use.\n");
        return EXIT_FAILURE;
    } else {
 /* read the abstraction and pattern database (state_map) */
    	heuristic = new AbstractionHeuristic();
        heuristic->read_abstraction_data( argv[1] );
        if (heuristic == NULL) {
            return EXIT_FAILURE;
        }
    }

    total_d = 0;
    total_expanded = 0;
    total_generated = 0;
    //printf("Enter a start state (empty input line to quit####): ");
    for( trials = 0;
         fgets( line, 4096, stdin ) != NULL
             && read_state( line, &state ) > 0;
         ++trials ) {

//        printf( "problem %d: ", trials + 1 );
  //      print_state( stdout, &state );
  //      printf( "\n" );
        gettimeofday( &start, NULL );

        d = zoomer( heuristic, &state );

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

        if ( d == INT_MAX ) {
        	cout << "d: " << d << " INTMAX: " << INT_MAX << endl;
            printf( "no solution found. expanded nodes: %" PRId64 ", generated nodes: %" PRId64 "\n",
		      nodes_expanded_for_startstate, nodes_generated_for_startstate );
        } else {
            printf( "cost: %d, expanded: %" PRId64 ", nodes: %" PRId64 "\n",
		      d, nodes_expanded_for_startstate, nodes_generated_for_startstate );
        }
        total_d += d;
        total_expanded  += nodes_expanded_for_startstate;
        total_generated += nodes_generated_for_startstate;

        //printf("Enter a start state (empty input line to quit): ");
    }


    printf( "total: depth: %d, expansion: %" PRId64 ", generation: %" PRId64 ", %zd.%06zd seconds\n",
            total_d, total_expanded, total_generated, total.tv_sec, total.tv_usec );

    delete heuristic;

    return EXIT_SUCCESS;
}


