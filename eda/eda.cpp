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
int64_t max_time_seconds; //maximum running time in seconds
int best_soln_sofar = INT_MAX;
float bound_factor;
int min_cost = INT_MAX;
int max_branching_fator_for_bound; //largest branching factor for a given cost bound
struct timeval start, end_time, total;

void find_minimum_operator_cost()
{
	for (int i = 0; i < NUM_FWD_RULES; i++)
	{
		if(fwd_rule_costs[i] < min_cost)
			min_cost= fwd_rule_costs[i];
	}
}

int dfs_heur( const AbstractionHeuristic * heuristic,
              const state_t *state,
              const state_t *parent_state, // for parent pruning
              const float bound, int current_g, int optimal )
{
    int rule_used;
    func_ptr iter;
    state_t child;

    gettimeofday( &end_time, NULL );
    if(end_time.tv_sec - start.tv_sec > max_time_seconds)
    	return INT_MAX;

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

            if(child_f <= bound && child_f < best_soln_sofar) {

            	int res = dfs_heur( heuristic, &child,
                        state,      // parent pruning
                        bound, current_g + move_cost, optimal );
               if( res == INT_MAX ) //timeout
            	   return INT_MAX;
               if( res ) //finding sub-optimal solutions
                   return 1;
            }
        }
    }

    return 0;
}



int optimisticidastar( const AbstractionHeuristic * heuristic, const state_t *state, const float bound_factor )
{
    int done;
    float bound;

    nodes_expanded_for_startstate  = 0;
    nodes_generated_for_startstate = 0;

    if (is_goal(state)) { return 0; }

    find_minimum_operator_cost();

    best_soln_sofar = INT_MAX;
    float N0 = heuristic->abstraction_data_lookup( state ); // initial bound = h(start)
    int k = 0;
    while (1) {
        nodes_expanded_for_bound  = 0;
        nodes_generated_for_bound = 0;
        leaf_nodes_for_bound = 0;
        max_branching_fator_for_bound = 0;
        bound = N0 * pow(bound_factor, k);
        done = dfs_heur( heuristic, state,
                             state,         // parent pruning
                             bound, 0,
							 1 ); // optimal search
        //printf( "bound: %d, expanded: %" PRId64 ", generated: %" PRId64 "\n", bound, nodes_expanded_for_bound, nodes_generated_for_bound );
        nodes_expanded_for_startstate  += nodes_expanded_for_bound;
        nodes_generated_for_startstate += nodes_generated_for_bound;

        gettimeofday( &end_time, NULL );
        if(end_time.tv_sec - start.tv_sec > max_time_seconds)
        	return INT_MAX;

        if( done == 1 )
            break;

        if ( best_soln_sofar <= bound )
            break;

        k++;
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

    total.tv_sec = 0;
    total.tv_usec = 0;

    AbstractionHeuristic * heuristic;

    if( argc != 4 ) {
        printf("There must 2 command line arguments, the prefix of the pattern database to use and the time limit in seconds.\n");
        return EXIT_FAILURE;
    } else {
 /* read the abstraction and pattern database (state_map) */
    	heuristic = new AbstractionHeuristic();
        heuristic->read_abstraction_data( argv[1] );

        if (heuristic == NULL) {
            return EXIT_FAILURE;
        }

        max_time_seconds = atoi(argv[2]);
        bound_factor = atof(argv[3]);
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
 //       print_state( stdout, &state );
  //      printf( "\n" );
        gettimeofday( &start, NULL );

        d = optimisticidastar( heuristic, &state, bound_factor );

        gettimeofday( &end_time, NULL );
        end_time.tv_sec -= start.tv_sec;
        end_time.tv_usec -= start.tv_usec;
        if( end_time.tv_usec < 0 ) {
        	end_time.tv_usec += 1000000;
            --end_time.tv_sec;
        }

        if (end_time.tv_usec + total.tv_usec >= 1000000) {
            total.tv_usec = end_time.tv_usec + total.tv_usec - 1000000;
            total.tv_sec = total.tv_sec + 1;
        } else {
            total.tv_usec = end_time.tv_usec + total.tv_usec;
        }
        total.tv_sec = total.tv_sec + end_time.tv_sec;

        if ( d == INT_MAX ) {
            printf( "no solution found. expanded nodes: %" PRId64 ", generated nodes: %" PRId64 "\n",
		      nodes_expanded_for_startstate, nodes_generated_for_startstate );
        } else {
            printf( "cost: %d, expanded: %" PRId64 ", nodes: %" PRId64 "\n",
		      d, nodes_expanded_for_startstate, nodes_generated_for_startstate );
        }
        if(d != INT_MAX)
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



