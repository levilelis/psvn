/*
Copyright (C) 2011-2013 by the PSVN Research Group, University of Alberta
*/

#ifndef _ASTAR_DATA_HPP
#define _ASTAR_DATA_HPP

#include <vector>
#include <string>
#include "psvn_game_so.h"
#include "hash_map.hpp"
#include "priority_queue.hpp"

class StateData
{
public:
    const void* m_state;
    const void* m_came_from;
    int16_t m_expanded;
    int16_t m_g;
    int16_t m_h;
    int16_t m_static_h; // used in al*(k)
    int m_index;        // position in bucket priority queue, -1 if not in open
	bool m_lookahead;

    StateData(): m_g(-1) { }

    explicit StateData(const void* state) 
        : m_state(state), m_came_from(0), m_expanded(0), m_g(0), m_h(0),
          m_index(-1), m_lookahead(false)
    { }

    StateData(const void* state, const void* came_from, int g, int h)
        : m_state(state), m_came_from(came_from), m_expanded(0),
          m_g(g), m_h(h), m_static_h(h), m_index(-1), m_lookahead(false)
    { }

    const void* State() const { return m_state; }
    const void* From() const { return m_came_from; }
    int G() const { return m_g; }
    int H() const { return m_h; }
    int StaticH() const { return m_static_h; }
    bool Expanded() const { return m_expanded; }

    bool IsValid() { return m_g != -1; }
    void Invalidate() { m_g = -1; }
};

class AStarData
{
public:
    void Clear();
    bool GetData(int64_t hash, StateData& data) const;
    StateData* AddData(int64_t hash, const StateData& data, 
                       const int f, const int g);
    StateData* AddData(int64_t hash, const StateData& data);

    /* Returns true if child has already been seen, false otherwise.
       If seen already seen, relaxes g and updates optimal path. */
    bool Relax(int64_t hash, int g, const void* from);

    /* Removes top state from open list, marks state as expanded, and
       stores the hash and data for the state. */
    void Pop(int64_t& hash, StateData& data);
	void Top(int64_t& hash, StateData& data);
    /* Returns true if the list is empty, false otherwise. */
    bool OpenListEmpty();

    size_t NumStored() const;

    size_t Memory() const;

    int AddToOpen(int64_t hash, int f, int g);
    void RemoveFromOpen(int f, int g, int index);

    StateData* GetPointer(int64_t hash) { return m_data.GetPointer(hash); }

private:
    HashMap<StateData> m_data;
    PriorityQueue<int64_t> m_open;
};

inline bool AStarData::GetData(int64_t hash, StateData& data) const
{
    return m_data.Get(hash, data);
}

inline StateData* AStarData::AddData(int64_t hash, const StateData& data)
{   
    return m_data.Add(hash, data);
}

inline StateData* AStarData::AddData(int64_t hash, const StateData& data, 
                                     const int f, const int g)
{
    StateData* ptr = m_data.Add(hash, data);
    ptr->m_index = AddToOpen(hash, f, g);
    return ptr;
}

inline int AStarData::AddToOpen(int64_t hash, int f, int g)
{
    return m_open.Add(f, g, hash);
}

inline void AStarData::RemoveFromOpen(int f, int g, int index)
{
    m_open.Modify(f, g, index, 0);
}

inline size_t AStarData::NumStored() const
{
    return m_data.NumStored();
}

inline size_t AStarData::Memory() const
{
    return m_data.Memory();
}

#endif // _ASTAR_DATA_HPP
