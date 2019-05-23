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
float m_factor; //multiplicative factor for Zoomer
int best_soln_sofar = INT_MAX;
long budget = 0;
struct timeval start, end_time, total;

int dfs_heur( const AbstractionHeuristic * heuristic,
              const state_t *state,
              const state_t *parent_state, // for parent pruning
              const double bound, double *theta_minus, double *theta_plus, long current_g, int optimal )
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

        if ( is_goal(&child) && current_g + move_cost <= bound ) {
            best_soln_sofar = myMIN(best_soln_sofar, current_g + move_cost);
            if( !optimal )
            	return 1;
        } else {
            int child_h = heuristic->abstraction_data_lookup( &child );
            int child_f = current_g + move_cost + child_h;

            if (current_g + move_cost + child_h > bound) {
               *theta_plus = myMIN( *theta_plus, current_g + move_cost + child_h );
            } else if(child_f < best_soln_sofar) {
            	*theta_minus = myMAX( *theta_minus, current_g + move_cost + child_h );
            	budget -= 1;
                if (budget == 0) {
                	return -1; //out of search budget
                }

                int res = dfs_heur( heuristic, &child,
                        state,      // parent pruning
                        bound, theta_minus, theta_plus, current_g + move_cost, optimal );
                if ( res == INT_MAX ) //out of time
                	return INT_MAX;
                if ( res == -1 )
                	return -1; //out of search budget
                if ( res == 1 )
                	return 1; //finding suboptimal solutions
            }
        }
    }

    return 0;
}

int zoomer( const AbstractionHeuristic * heuristic, const state_t *state, const float m_factor)
{
	if (is_goal(state)) { return 0; }

	double bound;
	int done;

    best_soln_sofar = INT_MAX;
    nodes_expanded_for_startstate  = 0;
    nodes_generated_for_startstate = 0;
    nodes_expanded_for_bound  = 0;
    nodes_generated_for_bound = 0;

    double upper;
    double up_min = INT_MAX;
    double dummy = -INT_MAX;
    double lower = heuristic->abstraction_data_lookup( state );

    budget = INT_MAX; //infinity search budget
    done = dfs_heur(heuristic, state, state, lower, &dummy, &up_min, 0, 0);
    if ( done == INT_MAX ) //Timeout
    	return INT_MAX;
    if( best_soln_sofar < INT_MAX ) { //found a solution with regular IDA*, no budget
    	nodes_expanded_for_startstate  += nodes_expanded_for_bound;
    	nodes_generated_for_startstate += nodes_generated_for_bound;
    	return best_soln_sofar;
    }

    nodes_expanded_for_startstate  += nodes_expanded_for_bound;
    nodes_generated_for_startstate += nodes_generated_for_bound;

    long N0 = nodes_expanded_for_bound;

    int k = 0;
    while (1) {
    	k += 1;
    	upper = INT_MAX;
    	double bound = -1;

    	while(upper > up_min) {
    		if (upper == INT_MAX)
    			bound = lower * 2;
    		else
    			bound = (upper + lower)/2;
    		bound = myMAX(bound, up_min);

            nodes_expanded_for_bound  = 0;
            nodes_generated_for_bound = 0;
            double theta_plus = INT_MAX;
            double theta_minus = -INT_MAX;

            budget = N0 * pow(m_factor, k);
            //printf("Budget %d and bound %f \n", budget, bound);
            done = dfs_heur( heuristic, state, state, bound, &theta_minus, &theta_plus, 0, 1 );

            nodes_expanded_for_startstate  += nodes_expanded_for_bound;
            nodes_generated_for_startstate += nodes_generated_for_bound;

            if ( done == INT_MAX ) //Timeout
            	return INT_MAX;
            if ( best_soln_sofar <= bound && done != -1) //A solution was found within the bound and it was proven to be optimal
            	return best_soln_sofar;
            if( done == -1) { //We have exhausted the search budget
            	upper = theta_minus;
            } else {
            	lower = bound;
            	up_min = theta_plus;
            }

            gettimeofday( &end_time, NULL );
            if(end_time.tv_sec - start.tv_sec > max_time_seconds)
            	return INT_MAX;
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
        m_factor = atof(argv[3]);
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

        d = zoomer( heuristic, &state, m_factor );

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


