/*
 * AbstractionHeuristic.h
 *
 *  Created on: Oct 2, 2013
 *      Author: levilelis
 */

#ifndef ABSTRACTIONHEURISTIC_H_
#define ABSTRACTIONHEURISTIC_H_

#include <iostream>
using namespace std;

#define MYMAX(x,y) (((x)>(y))?(x):(y))
#define MYMIN(x,y) (((x)<(y))?(x):(y))

class AbstractionHeuristic {

private:
    abstraction_t* abst;
    state_map_t* map;

public:
    AbstractionHeuristic();
    ~AbstractionHeuristic();

    void read_abstraction_data(char* prefix);
    void destroy_abstraction_data();
    int abstraction_data_lookup(const state_t* state) const;
    state_map_t* getStateMap();
    void setHValue(const state_t* state, int h);
    void get_abstracted_state(state_t* state, state_t* abst_state);

};

AbstractionHeuristic::AbstractionHeuristic()
{
	abst = NULL;
	map = NULL;
}

AbstractionHeuristic::~AbstractionHeuristic()
{
	destroy_abstraction_data();
}

state_map_t* AbstractionHeuristic::getStateMap()
{
	return this->map;
}

void AbstractionHeuristic::read_abstraction_data(char* prefix)
{
    /* get the abstraction filename by adding the extension ".abst" to the prefix */
    char filename[1024];
    strcpy(filename, prefix);
    strcat(filename, ".abst");

    abst = read_abstraction_from_file( filename );
    if( abst == NULL ) {
        fprintf( stderr, "could not read the abstraction file %s\n", filename );
        exit(-1);
    }
    /* get the pattern database filename by adding the extension ".state_map" to the prefix */
    char map_filename[1024];
    strcpy(map_filename, prefix);
    strcat(map_filename, ".state_map");

    /* read the pattern database (state_map) */
    FILE* file = fopen( map_filename, "r" );
    if( file == NULL ) {
        fprintf( stderr, "could not open the pattern database file %s\n", map_filename);
        destroy_abstraction( abst );
        exit(-1);
    }

    map = read_state_map( file );

    fclose( file );
    if( map == NULL ) {
        fprintf( stderr, "could not read the pattern database (state_map) %s\n", map_filename);
        destroy_abstraction( abst );
        exit(-1);
    }
}

void AbstractionHeuristic::setHValue(const state_t* state, int h)
{
	if( is_goal( state ) )
	{
		h = 0;
	}

    state_t abst_state;
    abstract_state( abst, state, &abst_state );

    uint64_t idx = state_map_hash_state( map, &abst_state );
    if( map->entries[ idx ].state.vars[ 0 ] < 0 ) {
      return;
    }

    map->entries[ idx ].value = h;
}

void AbstractionHeuristic::destroy_abstraction_data()
{
    destroy_abstraction(abst);
    destroy_state_map(map);
}

void AbstractionHeuristic::get_abstracted_state(state_t* state, state_t* abst_state)
{
	abstract_state( abst, state, abst_state );
}

int AbstractionHeuristic::abstraction_data_lookup(const state_t* state) const
{
	if( is_goal( state ) )
	{
		return 0;
	}

//	int epsilon = cost_of_cheapest_applicable_fwd_rule( state );

    state_t abst_state;
    abstract_state( abst, state, &abst_state );
    int *h;
    h = state_map_get( map, &abst_state );
    if (h == NULL) {
        return INT_MAX;
    }

 //   return MYMAX(*h, epsilon);
    return *h;
}

#endif /* ABSTRACTIONHEURISTIC_H_ */
