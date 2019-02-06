#define MYMAX(x,y) (((x)>(y))?(x):(y))
#define MYMIN(x,y) (((x)<(y))?(x):(y))

//---------------------------------------------------------------------------

#define MinCost 100 // This value should be greater than any start state's g* value.
int64_t pdb_lookup_count = 0;
abstraction_t* al_so_abst = NULL;
state_map_t* map = NULL;
void* work_stack[64];
void* work_child;
int lookahead_k = 0;
int solution_cost = INT_MAX;
int64_t expanded_nodes = 0; 
int64_t stored_nodes = 0;
int64_t trivial_nodes = 0;
int64_t lookahead_nodes = 0;
int64_t memory_used = 0;
bool solution_found = false;
int NextBestF = 0;
int64_t LHFrom = 0;

int Heuristic(const void* state, state_map_t* map, 
              const compiled_game_so_t* game)
{
    if (is_so_goal(state, game))
        return 0;
    int epsilon = cost_of_cheapest_so_fwd_rule(game);
    if (map != NULL) {
		pdb_lookup_count++;
		state_t abst_state;
	    abstract_state( al_so_abst, (state_t*)state, &abst_state );
		int* h = state_map_get( map, &abst_state );

		//return MYMAX(epsilon, *h);
		return *h;
    }
    return epsilon;
}

class ChildGenerator
{
public:
    ChildGenerator(const compiled_game_so_t* game, const void* state)
        : m_game(game), m_state(state)
    { 
        init_so_fwd_iter(m_iter, m_game);
    }
    
    bool Generate(void* child)
    {
        if ((m_rule_used = next_so_fwd_iter(m_iter, m_state)) >= 0) {
            apply_so_fwd_rule(m_rule_used, m_state, child, m_game);
            return true;
        }
        return false;
    }

    int Cost() const 
    { return so_fwd_rule_cost(m_rule_used, m_game); }

    const char* RuleName() const 
    { return m_game->fwd_rule_names[m_rule_used]; }

private:
    const compiled_game_so_t* m_game;
    const void* m_state;
    so_func_ptr m_iter;
    int m_rule_used;
};

class PruningChildGenerator
{
public:
    PruningChildGenerator(const compiled_game_so_t* game, 
                          const void* state,
                          int history)
        : m_game(game), m_state(state), m_history(history)
    { 
        init_so_fwd_iter(m_iter, m_game);
    }
    
    bool Generate(void* child, int* child_history)
    {
        while ((m_rule_used = next_so_fwd_iter(m_iter, m_state)) >= 0) {
            if (so_fwd_rule_valid_for_history(m_history, m_rule_used, m_game))
                break;
        }
        if (m_rule_used < 0)
            return false;
        apply_so_fwd_rule(m_rule_used, m_state, child, m_game);
        *child_history = next_so_fwd_history(m_history, m_rule_used, m_game);
        return true;
    }

    int Cost() const 
    { return so_fwd_rule_cost(m_rule_used, m_game); }

    const char* RuleName() const
    { return m_game->fwd_rule_names[m_rule_used]; }

private:
    const compiled_game_so_t* m_game;
    const void* m_state;
    so_func_ptr m_iter;
    int m_rule_used;
    int m_history;
};

