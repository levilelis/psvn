/*
Copyright (C) 2011-2013 by the PSVN Research Group, University of Alberta
*/

#ifndef _RESERVOIR_HPP
#define _RESERVOIR_HPP

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <vector>

/* Dynamically increasing pool of memory. */
class Reservoir
{
public:
    Reservoir();
    Reservoir(size_t page_size_in_mb);
    ~Reservoir();

    void* Get(size_t size);
    void Reset();

private:
    size_t m_page_size;
    size_t m_index;
    size_t m_page;
    std::vector<void*> m_pages;

    void Create(size_t page_size_in_mb);

    /* Non-copyable */
    Reservoir(const Reservoir& other);
    void operator=(const Reservoir& other);
};

inline Reservoir::Reservoir()
{
    Create(16);
}

inline Reservoir::Reservoir(size_t page_size_in_mb)
{
    Create(page_size_in_mb);
}

inline void Reservoir::Create(size_t page_size_in_mb)
{
    m_page_size = page_size_in_mb * 1048576;
    m_index = 0;
    m_page = 0;
    m_pages.push_back(malloc(m_page_size));
}

inline Reservoir::~Reservoir()
{    for (size_t i = 0; i < m_pages.size(); ++i)
        free(m_pages[i]);
}

inline void* Reservoir::Get(size_t size)
{
    assert(size < m_page_size);
    if (m_index + size >= m_page_size) {
        m_page++;
        m_index = 0;
    }
    if (m_page == m_pages.size()) {
        m_pages.push_back(malloc(m_page_size));
    }
    void* ret = (char*)m_pages[m_page] + m_index;
    m_index += size;
    return ret;
}

inline void Reservoir::Reset()
{
    m_page = 0;
    m_index = 0;
}

#endif // _RESERVOIR_HPP
