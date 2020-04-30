/*
Copyright (C) 2011-2013 by the PSVN Research Group, University of Alberta
*/

#include "astar_data.hpp"

void AStarData::Clear()
{
    m_data.Clear();
    m_open.Clear();
}

bool AStarData::Relax(int64_t hash, const int g, const void* parent)
{
    StateData* ptr = m_data.GetPointer(hash);
    if (ptr == NULL)
        return false;
    if (g >= ptr->m_g)
        return true;
    //assert(ptr->m_expanded == false);
	if (ptr->m_expanded == false) {
	    RemoveFromOpen(ptr->m_g + ptr->m_h, ptr->m_g, ptr->m_index);
	    ptr->m_g = g;
	    ptr->m_came_from = parent;
	    ptr->m_index = AddToOpen(hash, ptr->m_g + ptr->m_h, ptr->m_g);
	} else {
		ptr->m_expanded = 0;
	    ptr->m_g = g;
	    ptr->m_came_from = parent;
	    ptr->m_index = AddToOpen(hash, ptr->m_g + ptr->m_h, ptr->m_g);
	}
    
    return true;
}

bool AStarData::OpenListEmpty()
{
    // pop any stale arrivals before determining if list is empty
    while (!m_open.Empty()) {
        int64_t hash = m_open.Top();
        if (hash != 0) {
        	break;
        } // not stale!
        m_open.Pop();
    }
    return m_open.Empty();
}

void AStarData::Pop(int64_t& hash, StateData& data)
{
    assert(!m_open.Empty());
	
	StateData* ptr;
	while (true) {
	    hash = m_open.Top();
	    m_open.Pop();
	    ptr = m_data.GetPointer(hash);
		if (ptr == NULL && hash != 0) {
			//fprintf(stderr, "Pop: %lu\n", hash);
		} else {
			break;
		}
	}
    assert(ptr != NULL);
    ptr->m_index = -1;   // mark that it is no longer in open list
    ptr->m_expanded = true;
    data = *ptr;
}

void AStarData::Top(int64_t& hash, StateData& data)
{
	if (m_open.Empty()) {
		return;
	} else {
	    assert(!m_open.Empty());
		
		StateData* ptr;
		while (true) {
		    hash = m_open.Top();
		    //m_open.Pop();
		    ptr = m_data.GetPointer(hash);
			if (ptr == NULL) {
				//fprintf(stderr, "Pop: %lu\n", hash);
				if (hash != 0) {
					//fprintf(stderr, "Pop: %lu\n", hash);
				}
				m_open.Pop();
			} else {
				break;
			}
		}
		/*
	    hash = m_open.Top();
	    StateData* ptr = m_data.GetPointer(hash);
		if (ptr == NULL) {
			fprintf(stderr, "Pop: %lu\n", hash);
		}
		*/
	    assert(ptr != NULL);
	    data = *ptr;
	}
}
