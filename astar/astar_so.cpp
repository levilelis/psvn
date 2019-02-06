#include <vector>
#include <iostream>
#include <inttypes.h>
#include <sys/time.h>
#include "astar_data.hpp"
#include "reservoir.hpp"
#include "psvn_game_so.h"
#include "timer.hpp"
#include "al_common.h"
extern "C" {
#include "misc.h"
#include "so_util.h"
}


using namespace std;


int AStar(const compiled_game_so_t* game, const void* start_state,
          int64_t *nodes_expanded, int64_t *nodes_stored, int64_t *memory_used)
{
	Timer instance_timer;

    if (is_so_goal(start_state, game))
        return 0;

    Reservoir res(16);
    void* child = res.Get(game->state_size);
    AStarData data;
    data.AddData(hash_so_state(start_state, game), StateData(start_state), 0, 0);

    int current_f_layer = Heuristic(start_state, map, game);
    long nodes_expanded_f_layer = 0;

    *nodes_expanded = 0;
    while (!data.OpenListEmpty()) {
        int64_t current_hash;
        StateData current_data;
        data.Pop(current_hash, current_data);

        if( current_data.G() + current_data.H() != current_f_layer && current_data.G() != 0 )
        {
        	//cout << "g: \t" << current_data.G() << " h: \t" << current_data.H() << endl;
        	cout << current_f_layer << " " << nodes_expanded_f_layer << " " << instance_timer.Elapsed() << endl;
        	current_f_layer = current_data.G() + current_data.H();
        	//cout << "Memory Used: " << data.Memory() << " Nodes Stored: " << data.NumStored() << endl;
        }

        if( data.Memory() > 3000000000 )
		{
			//cout << "Reached Memory Limit!" << endl;
			//cout << "Memory Used: " << data.Memory() << " Nodes Stored: " << data.NumStored() << endl;
			return -1;
		}

        if (is_so_goal(current_data.State(), game)) {
            *nodes_stored = data.NumStored();
            *memory_used = data.Memory();
            return current_data.G();
        }

		//char ss[100];
		//sprint_state(ss, 100, (state_t*)current_data.State());
		//cout << ss << "g: " << current_data.G() << " h: " <<  current_data.H() << endl;


        (*nodes_expanded)++;
        nodes_expanded_f_layer++;
		StateData* current_ptr = data.GetPointer(current_hash);
		current_ptr->m_expanded = true;
        ChildGenerator generator(game, current_data.State());
        while (generator.Generate(child)) {

      /*  	char ss[100];
        	sprint_state(ss, 100, (state_t*)current_data.State());
        	cout << ss << " g: " << current_data.G() + generator.Cost()
        			<<	"h: " << Heuristic(child, map, game) << endl;
*/
            const int64_t child_hash = hash_so_state(child, game);

            const int child_g = current_data.G() + generator.Cost();
            // relax previously seen child
            if (data.Relax(child_hash, child_g, current_data.State()))
                continue;
            // add new child
            void* new_child = res.Get(game->state_size);
            copy_so_state(new_child, child, game);
            const int child_h = Heuristic(child, map, game);
            data.AddData(child_hash, StateData(new_child, current_data.State(),
                                               child_g, child_h),
                         child_g + child_h, child_g);
        }
    }
    return INT_MAX;
}

int main( int argc, char **argv )
{
	void *state;
    //compiled_game_so_t *game;
    int trials, d;
    int64_t expanded, total_expanded, stored, total_stored, memory_used;
    char line[4096];

    //game = load_psvn_so_object( argv[1] );

    /* read the pdb */
    if( argc > 1 ) {
		char filename[1024];
	    strcpy(filename, argv[1]);
	    al_so_abst = read_abstraction_from_file( filename );
		strcpy(strstr(filename, ".abst"), ".state_map");
		FILE* map_file = fopen( filename, "r" );
		map = read_state_map( map_file );
		fclose(map_file);
        if (map == NULL)
            return EXIT_FAILURE;
    } else {
		map = NULL;
    }
    
    state = new_so_state( game );
    total_expanded = 0;
    total_stored = 0;
    //Timer total_timer;
	struct timeval fetch_start1, fetch_end1, fetch_total1;
	fetch_total1.tv_sec = 0;
	fetch_total1.tv_usec = 0;
	int total_cost = 0;
	// use a vector to record all test instances costs
	vector<int> all_ds;
    for( trials = 0;
         fgets( line, 4096, stdin ) != NULL
             && read_so_state( line, state, game ) > 0;
         ++trials ) 
    {
        printf( "problem %d: ", trials + 1 );
        print_so_state( stdout, state, game );
        printf( "\n" );
        
        Timer instance_timer;
		gettimeofday( &fetch_start1, NULL );
        d = AStar( game, state, &expanded, &stored, &memory_used);
		gettimeofday( &fetch_end1, NULL );
	    fetch_end1.tv_sec -= fetch_start1.tv_sec;
	    fetch_end1.tv_usec -= fetch_start1.tv_usec;
	    if( fetch_end1.tv_usec < 0 ) {
	        fetch_end1.tv_usec += 1000000;
	        --fetch_end1.tv_sec;
	    }
		if (fetch_end1.tv_usec + fetch_total1.tv_usec >= 1000000) {
			fetch_total1.tv_usec = fetch_end1.tv_usec + fetch_total1.tv_usec - 1000000;
			fetch_total1.tv_sec = fetch_total1.tv_sec + 1;		
		} else {
			fetch_total1.tv_usec = fetch_end1.tv_usec + fetch_total1.tv_usec;
		}
		fetch_total1.tv_sec = fetch_total1.tv_sec + fetch_end1.tv_sec;
        assert(d != INT_MAX);

    //    printf("elapsed %0.3lf cost %d expanded %" PRId64 " stored %" PRId64 " memory %s\n",
     //          instance_timer.Elapsed(), d, expanded, stored,
     //          print_bytes(memory_used));
        total_cost += d;
		total_expanded += expanded;
        total_stored += stored;
		all_ds.push_back(d);
    }
    destroy_so_state(state);
    if( map != NULL ) {
	    destroy_abstraction(al_so_abst);
	    destroy_state_map(map);
    }
    /*
    //double elapsed = total_timer.Elapsed();
	double elapsed = fetch_total1.tv_sec + fetch_total1.tv_usec / 1000000.0f;
    printf("==================================\n");
    printf("expanded %" PRId64 " (%.3gm) %.2fm/s\n", total_expanded,
           (total_expanded / 1000000.0),
           (total_expanded / 1000000.0) / elapsed);
    printf("stored   %" PRId64 " (%.3gm) %.2fm/s\n", total_stored,
           (total_stored / 1000000.0),
           (total_stored / 1000000.0) / elapsed);
	printf("pdb lookup  %" PRId64 " (%.3gm) %.2fm/s\n", pdb_lookup_count,
	   	   (pdb_lookup_count / 1000000.0),
	   	   (pdb_lookup_count / 1000000.0) / elapsed);
	printf("elapsed  %.03lfs\n", elapsed);
	printf("total cost: %d\n", total_cost);
	*/

	/*
	char output_temp[1024];
    strcpy(output_temp, argv[1]);
	strcpy(strstr(output_temp, ".abst"), ".txt");
	string output_filename = "all_ds_";
	output_filename += output_temp;
	FILE* file = fopen(output_filename.c_str(), "w");
	for (int i = 0; i < all_ds.size(); i++) {
		fprintf(file, "%d\n", all_ds[i]);
	}
	fclose(file);

	output_filename = "all_results_to_excel_";
	output_filename += output_temp;
	file = fopen(output_filename.c_str(), "w");
	fprintf(file, "A*\n");
	fprintf(file, "%" PRId64 "\t%" PRId64 "\t%" PRId64 "\t%.03lf\n",
		total_expanded, total_stored, pdb_lookup_count, elapsed);
	fprintf(file, "----------------------------------------\n\n");
	fclose(file);
	*/
    return 0;
}
