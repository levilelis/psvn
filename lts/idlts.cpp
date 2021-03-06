#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/time.h>
#include <vector>
#include <math.h>
#include <iostream>
#include "../AbstractionHeuristic.h"

using namespace std;

#define myMAX(x,y) (((x)>(y))?(x):(y))
#define myMIN(x,y) (((x)<(y))?(x):(y))

/* GLOBAL VARIABLES */
int64_t nodes_expanded_for_bound ;    // number of nodes expanded for a given cost bound
int64_t nodes_generated_for_bound ;   // number of nodes generated for a given cost bound
int64_t nodes_expanded_for_startstate ;   // number of nodes expanded until solution found for a given start state
int64_t nodes_generated_for_startstate ;  // number of nodes generated until solution found for a given start state
int solution_cost = INT_MAX;
float temperature;

void set_temperature_to_max_operator_cost()
{
	int max = 0;
	for (int i = 0; i < NUM_FWD_RULES; i++)
	{
		if(fwd_rule_costs[i] > max)
			max = fwd_rule_costs[i];
	}

	temperature = max;
	//cout << "Setting temperature to: " << temperature << endl;
}

void compute_f_values(const AbstractionHeuristic * heuristic, const state_t* parent, const state_t *state, int* f_values, const int current_g,
		state_t *children, int *move_costs, int & num_children)
{
    int rule_used;
    func_ptr iter;

    init_fwd_iter( iter );
    while( ( rule_used = next_fwd_iter( iter, state ) ) >= 0 ) {
    	apply_fwd_rule( rule_used, state, &children[num_children] );

        if( compare_states( &children[num_children], parent ) == 0 )   // parent pruning
            continue;

        const int move_cost = fwd_rule_costs[ rule_used ];
        int child_f = heuristic->abstraction_data_lookup( &children[num_children] ) + current_g + move_cost;
        f_values[num_children] = child_f;

        move_costs[num_children] = move_cost;
        num_children++;
    }
}

void compute_probability_distribution_from_values(int* f_values, float* probs, const int f_value_parent, const int num_children)
{
	float den = 0.0;

	for(int i = 0; i < num_children; i++)
	{
		den += exp((f_value_parent - f_values[i])/temperature);
	}

	for(int i = 0; i < num_children; i++)
	{
		probs[i] = (exp((f_value_parent - f_values[i])/temperature)/den);
	}
}

int dfs_heur( const AbstractionHeuristic * heuristic,
              const state_t *state,
			  const int f_state, // for computing policy
              const state_t *parent_state, // for parent pruning
              const int bound, int current_g,
			  int depth, float p)
{
    int rule_used;
    func_ptr iter;
    state_t child;
    state_t children[NUM_FWD_RULES];
    int move_costs[NUM_FWD_RULES];
    int f_values_children[NUM_FWD_RULES];
    float probability_distribution[NUM_FWD_RULES];
    int num_children = 0;

    nodes_expanded_for_bound++;

    //Computing the number of children and their f_values
    compute_f_values(heuristic, parent_state, state, f_values_children, current_g, children, move_costs, num_children);
    compute_probability_distribution_from_values(f_values_children, probability_distribution, f_state, num_children);

    for(int child_count = 0; child_count < num_children; child_count++) {
        nodes_generated_for_bound++;
        const int move_cost = move_costs[child_count];

        if(is_goal(&children[child_count])) {
        	solution_cost = current_g + move_cost;
        	return 1;
        } else {
        	int f_child = f_values_children[child_count];

            float v = log2(depth) - p;

            if(v <= bound)
            {
               if( dfs_heur( heuristic, &children[child_count],
            		   	   	 f_child,
                             state,      // parent pruning
                             bound, current_g + move_cost,
							 depth + 1, p + log2(probability_distribution[child_count])) )
               {
                   return 1;
               }
            }
        }
    }

    return 0;
}

int idastar( const AbstractionHeuristic * heuristic, const state_t *state )
{
    int bound, done;

    nodes_expanded_for_startstate  = 0;
    nodes_generated_for_startstate = 0;

    set_temperature_to_max_operator_cost();

    if (is_goal(state)) { return 0; }

    solution_cost = INT_MAX;
    int h_value_start_state = heuristic->abstraction_data_lookup( state ); // initial bound = h(start)
    bound = 1;
    while (1) {
        nodes_expanded_for_bound  = 0;
        nodes_generated_for_bound = 0;

        done = dfs_heur( heuristic, state,
        					 h_value_start_state, // f_value of the start state
                             state,         // parent pruning
                             bound, 0,
							 1, 0);
        //printf( "bound: %d, expanded: %" PRId64 ", generated: %" PRId64 "\n", bound, nodes_expanded_for_bound, nodes_generated_for_bound );
        nodes_expanded_for_startstate  += nodes_expanded_for_bound;
        nodes_generated_for_startstate += nodes_generated_for_bound;
        if( done ) {
            break;
        }

        bound += 1;
    }

    return solution_cost;
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
 //       print_state( stdout, &state );
  //      printf( "\n" );
        gettimeofday( &start, NULL );

        d = idastar( heuristic, &state );

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



