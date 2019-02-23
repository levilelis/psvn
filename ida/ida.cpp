/*
Copyright (C) 2013 by the PSVN Research Group, University of Alberta
*/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/time.h>
#include "../AbstractionHeuristic.h"


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
int64_t max_time_seconds; //maximum running time in seconds
int best_soln_sofar = INT_MAX;
struct timeval start, end_time, total;


int dfs_heur( const AbstractionHeuristic * heuristic,
              const state_t *state,
#ifdef MOVE_PRUNING
              const int history,           // for move pruning
#else
              const state_t *parent_state, // for parent pruning
#endif
              const int bound, int *next_bound, int current_g )
{
    int rule_used;
#ifdef MOVE_PRUNING
    int c_history;
#endif
    func_ptr iter;
    state_t child;

    gettimeofday( &end_time, NULL );
    if(end_time.tv_sec - start.tv_sec > max_time_seconds)
    	return INT_MAX;

    nodes_expanded_for_bound++;

    init_fwd_iter( iter );
    while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 ) {

#ifdef MOVE_PRUNING
        if( !fwd_rule_valid_for_history( history, rule_used ) )
            continue;
        c_history = next_fwd_history( history, rule_used );
#endif

        apply_fwd_rule( rule_used, state, &child );
        nodes_generated_for_bound++;

#ifndef MOVE_PRUNING
        if( compare_states( &child, parent_state ) == 0 )   // parent pruning
            continue;
#endif

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
               if( dfs_heur( heuristic, &child,
        #ifdef MOVE_PRUNING
                             c_history,  // move pruning
        #else
                             state,      // parent pruning
        #endif
                             bound, next_bound, current_g + move_cost ) )
               {
                   return 1;
                }
            }
        }
    }
    assert( *next_bound > bound );
    return 0;
}



int idastar( const AbstractionHeuristic * heuristic, const state_t *state )
{
    int next_bound, bound, done;

    nodes_expanded_for_startstate  = 0;
    nodes_generated_for_startstate = 0;

    if (is_goal(state)) { return 0; }

    best_soln_sofar = INT_MAX;
    bound = heuristic->abstraction_data_lookup( state ); // initial bound = h(start)
    while (1) {
        next_bound = INT_MAX;
        nodes_expanded_for_bound  = 0;
        nodes_generated_for_bound = 0;
        done = dfs_heur( heuristic, state,
#ifdef MOVE_PRUNING
                             init_history,  // move pruning
#else
                             state,         // parent pruning
#endif
                             bound, &next_bound, 0 );
        //printf( "bound: %d, expanded: %" PRId64 ", generated: %" PRId64 "\n", bound, nodes_expanded_for_bound, nodes_generated_for_bound );
        nodes_expanded_for_startstate  += nodes_expanded_for_bound;
        nodes_generated_for_startstate += nodes_generated_for_bound;
        if( done ) {
            break;
        }

        gettimeofday( &end_time, NULL );
        if(end_time.tv_sec - start.tv_sec > max_time_seconds)
        	return INT_MAX;

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
    total.tv_sec = 0;
    total.tv_usec = 0;

    AbstractionHeuristic * heuristic;

    if( argc != 3 ) {
        printf("There must 2 command line arguments, the prefix of the pattern database to use and the time limit in seconds.\n");
        return EXIT_FAILURE;
    } else {
 /* read the abstraction and pattern database (state_map) */
    	heuristic = new AbstractionHeuristic();
        heuristic->read_abstraction_data( argv[1] );
        if (heuristic == NULL) {
            return EXIT_FAILURE;
        }

        max_time_seconds = stoi(argv[2]);
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

        d = idastar( heuristic, &state );

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
        if ( d != INT_MAX ) {
        	total_d += d;
        }
        total_expanded  += nodes_expanded_for_startstate;
        total_generated += nodes_generated_for_startstate;

        //printf("Enter a start state (empty input line to quit): ");
    }


    printf( "total: depth: %d, expansion: %" PRId64 ", generation: %" PRId64 ", %zd.%06zd seconds\n",
            total_d, total_expanded, total_generated, total.tv_sec, total.tv_usec );

    delete heuristic;

    return EXIT_SUCCESS;
}



