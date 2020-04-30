#ifndef _PSVN_GAME_SO_H
#define _PSVN_GAME_SO_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>

/* function pointer type used as iterator for compiled game shared object */
typedef int (*so_func_ptr)( const void *, void * );
typedef void (*so_actfunc_ptr)( const void *, void * );
typedef void (*so_dynactfunc_ptr)( const void *, void *, const void* );

typedef struct {
  const int num_vars;
  const int var_size;
  const int state_size;

  const int num_fwd_rules;
  const int num_bwd_rules;

  const char **fwd_rule_names;
  const char **bwd_rule_names;

  const int *fwd_rule_label_sets;
  const int *bwd_rule_label_sets;

  const int *fwd_rule_costs;
  const int cost_of_cheapest_fwd_rule;
  const int *bwd_rule_costs;
  const int cost_of_cheapest_bwd_rule;

  const int empty_history;

  const int max_children;
  const so_func_ptr fwdfn0;
  const int fwd_entry_id;
  const so_actfunc_ptr *fwd_rules;
  const so_dynactfunc_ptr *dyn_fwd_rules;
  const int fwd_history_len;
  const int *fwd_prune_table;

  const int bw_max_children;
  const so_func_ptr bwdfn0;
  const int bwd_entry_id;
  const so_actfunc_ptr *bwd_rules;
  const so_dynactfunc_ptr *dyn_bwd_rules;
  const int bwd_history_len;
  const int *bwd_prune_table;
  
  int (*is_goal)( const void * );
  void (*init_goal_state)( void *, int );
  int8_t (*next_goal_state)( void *, int * );
  void (*random_goal_state)( void * );

  int (*cost_of_cheapest_applicable_fwd_rule)( const void * );
  int (*cost_of_cheapest_applicable_bwd_rule)( const void * );

  ssize_t (*print_state)( FILE *, const void * );
  ssize_t (*sprint_state)( char *, const size_t, const void * );
  ssize_t (*read_state)( const char *, void * );

  uint64_t (*hash_state)( const void * );
  uint64_t (*hash_state_history) (const void *, const int );
  void (*hashlittle2)( const void *, size_t, uint32_t *, uint32_t * );

  void *(*new_state_map)();
  void (*destroy_state_map)( void * );
  void (*state_map_add)( void *, const void *, const int );
  int *(*state_map_get)( const void *, const void * );
  void (*write_state_map)( FILE *, const void * );
  void *(*read_state_map)( FILE * );

  void *(*allocate_abstraction)();
  void (*destroy_abstraction)( void * );
  void (*abstraction_compute_mapped_in)(void* );
  void *(*create_identity_abstraction)();
  void *(*read_abstraction_from_file)( const char * );
  void *(*read_abstraction_from_stream)( FILE * );
  void (*print_abstraction)( void * );
  void (*abstract_state)( const void *, const void *, void * );

  void (*init_dyn_abstractions)();
  int (*next_dyn_iter)(const void*, void*, const void*, const void*);
  int (*is_dyn_goal)(const void *, const void * );
  void (*init_dyn_goal_state)(void *, int *, const void *);
  int8_t (*next_dyn_goal_state)(void *, int *, const void *);
  void (*random_dyn_goal_state)(void *, const void * );

} compiled_game_so_t;


/* shared object states are void pointers */
#define new_so_state( compiled_game_so_ptr ) malloc( (compiled_game_so_ptr)->state_size )
#define destroy_so_state( state_ptr ) free(state_ptr)

#define init_so_history( compiled_game_so_ptr ) (compiled_game_so_ptr)->empty_history
#define init_so_fwd_iter( so_func_ptr_iter, compiled_game_so_ptr ) (so_func_ptr_iter=(compiled_game_so_ptr)->fwdfn0)
#define next_so_fwd_iter( so_func_ptr_iter, state_ptr ) ((so_func_ptr_iter)?(so_func_ptr_iter)(state_ptr,&so_func_ptr_iter):-1)
#define apply_so_fwd_rule( rule, state_ptr, result, compiled_game_so_ptr ) (compiled_game_so_ptr)->fwd_rules[(rule)](state_ptr,result)

#define init_so_dyn_fwd_iter(iter, compiled_game_so_ptr) ((iter).id = (compiled_game_so_ptr)->fwd_entry_id)
#define next_so_dyn_fwd_iter(iter, state, abst, so_ptr) (so_ptr)->next_dyn_iter(state, iter, abst, abst->fwd_rule_label_sets, (so_ptr)->fwd_var_test_table)
#define apply_so_dyn_fwd_rule(rule, state, result, abst, so_ptr ) (so_ptr)->dyn_fwd_rules[(rule)](state, result, abst)

#define so_fwd_rule_cost( rule, compiled_game_so_ptr ) (compiled_game_so_ptr)->fwd_rule_costs[(rule)]
#define so_fwd_rule_valid_for_history( history, rule_used, compiled_game_so_ptr ) ((compiled_game_so_ptr)->fwd_prune_table[(history)+(rule_used)]>=0)
#define next_so_fwd_history( history, rule_used, compiled_game_so_ptr ) (compiled_game_so_ptr)->fwd_prune_table[(history)+(rule_used)]
#define so_fwd_rule_pruned_by( history, rule_used, compiled_game_so_ptr ) (-(compiled_game_so_ptr)->fwd_prune_table[(history)+(rule_used)] - 1)

#define init_so_bwd_iter( so_func_ptr_iter, compiled_game_so_ptr ) (so_func_ptr_iter=(compiled_game_so_ptr)->bwdfn0)
#define next_so_bwd_iter( so_func_ptr_iter, state_ptr ) ((so_func_ptr_iter)?(so_func_ptr_iter)(state_ptr,&so_func_ptr_iter):-1)
#define apply_so_bwd_rule( rule, state_ptr, result, compiled_game_so_ptr ) (compiled_game_so_ptr)->bwd_rules[(rule)](state_ptr,result)

#define init_so_dyn_bwd_iter(iter, compiled_game_so_ptr) ((iter).id = (compiled_game_so_ptr)->bwd_entry_id)
#define next_so_dyn_bwd_iter(iter, state, abst, so_ptr) (so_ptr)->next_dyn_iter(state, iter, abst, abst->bwd_rule_label_sets, (so_ptr)->bwd_var_test_table)
#define apply_so_dyn_bwd_rule(rule, state, result, abst, so_ptr ) (so_ptr)->dyn_bwd_rules[(rule)](state, result, abst)

#define so_bwd_rule_cost( rule, compiled_game_so_ptr ) (compiled_game_so_ptr)->bwd_rule_costs[(rule)]
#define so_bwd_rule_valid_for_history( history, rule_used, compiled_game_so_ptr ) ((compiled_game_so_ptr)->bwd_prune_table[(history)+(rule_used)]>=0)
#define next_so_bwd_history( history, rule_used, compiled_game_so_ptr ) (compiled_game_so_ptr)->bwd_prune_table[(history)+(rule_used)]
#define so_bwd_rule_pruned_by( history, rule_used, compiled_game_so_ptr ) (-(compiled_game_so_ptr)->bwd_prune_table[(history)+(rule_used)] - 1)

#define is_so_goal( state_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->is_goal(state_ptr)
#define first_so_goal_state( state_ptr, int_ptr_goal_iter, compiled_game_so_ptr ) (compiled_game_so_ptr)->init_goal_state(state_ptr,*(int_ptr_goal_iter)=0)
#define next_so_goal_state( state_ptr, int_ptr_goal_iter, compiled_game_so_ptr ) (compiled_game_so_ptr)->next_goal_state(state_ptr,int_ptr_goal_iter)
#define random_so_goal_state( state_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->random_goal_state(state_ptr)

#define copy_so_state( dest_ptr, src_ptr, compiled_game_so_ptr ) memcpy(dest_ptr,src_ptr,(compiled_game_so_ptr)->state_size)

#define compare_so_states( a, b, compiled_game_so_ptr ) memcmp(a,b,(compiled_game_so_ptr)->state_size)

#define cost_of_cheapest_applicable_so_fwd_rule(state_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->cost_of_cheapest_fwd_rule(state_ptr)
#define cost_of_cheapest_applicable_so_bwd_rule(state_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->cost_of_cheapest_bwd_rule(state_ptr)

#define cost_of_cheapest_so_fwd_rule( compiled_game_so_ptr ) (compiled_game_so_ptr)->cost_of_cheapest_fwd_rule
#define cost_of_cheapest_so_bwd_rule( compiled_game_so_ptr ) (compiled_game_so_ptr)->cost_of_cheapest_bwd_rule

#define print_so_state( file_ptr, state_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->print_state(file_ptr,state_ptr)
#define sprint_so_state( string, max_len, state_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->sprint_state(string,max_len,state_ptr)
#define read_so_state( string, state_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->read_state(string,state_ptr)
#define dump_so_state( file, state_ptr, compiled_game_so_ptr ) fwrite(state_ptr,(compiled_game_so_ptr)->state_size,1,file)
#define load_so_state( file, state_ptr, compiled_game_so_ptr ) fread(state_ptr,(compiled_game_so_ptr)->state_size,1,file)

#define hash_so_state( state_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->hash_state( state_ptr )
#define hash_so_state_history( state_ptr, history, compiled_game_so_ptr ) (compiled_game_so_ptr)->hash_state_history( state_ptr, history )

#define new_so_state_map( compiled_game_so_ptr ) (compiled_game_so_ptr)->new_state_map()
#define destroy_so_state_map( state_map_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->destroy_state_map(state_map_ptr)
#define so_state_map_add( state_map_ptr, state_ptr, int_value, compiled_game_so_ptr ) (compiled_game_so_ptr)->state_map_add(state_map_ptr,state_ptr,int_value)
#define so_state_map_get( state_map_ptr, state_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->state_map_get(state_map_ptr,state_ptr)
#define write_so_state_map( file_ptr, state_map_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->write_state_map(file_ptr,state_map_ptr)
#define read_so_state_map( file_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->read_state_map(file_ptr)

#define allocate_so_abstraction( compiled_game_so_ptr ) (compiled_game_so_ptr)->allocate_abstract()
#define destroy_so_abstraction( abstraction_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->destroy_abstraction(abstraction_ptr)
#define abstraction_so_compute_mapped_in( abstraction_ptr, game_so_ptr ) (game_so_ptr)->abstraction_compute_mapped_in( abstraction_ptr )
#define create_so_identity_abstraction( game_ptr ) (game_ptr)->create_identity_abstraction()
#define read_so_abstraction_from_file( filename, compiled_game_so_ptr ) (compiled_game_so_ptr)->read_abstraction_from_file(filename)
#define read_so_abstraction_from_stream( stream, compiled_game_so_ptr ) (compiled_game_so_ptr)->read_abstraction_from_stream(stream)
#define print_so_abstraction(abstraction_ptr, game_ptr) (game_ptr)->print_abstraction(abstraction_ptr)
#define so_abstract_state( abstraction_ptr, state_ptr, out_ptr, compiled_game_so_ptr ) (compiled_game_so_ptr)->abstract_state(abstraction_ptr,state_ptr, out_ptr)

#endif
