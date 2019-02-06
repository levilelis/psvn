/*
  Copyright (C) 2011-2013 by the PSVN Research Group, University of Alberta
*/

#ifndef _HASHMAP_HPP
#define _HASHMAP_HPP

#include <inttypes.h>
#include <cassert>

template <typename T>
class HashMap
{
public:
    HashMap();
    explicit HashMap(const HashMap& other);
    ~HashMap();

    /* Loads entries from other in table. */
    void LoadFrom(const HashMap& other);

    /* Returns pointer to newly added data. */
    T* Add(uint64_t hash, const T& data);
    
    /* Returns pointer to data in map, NULL if data not found.
       WARNING: pointer will be invalid if map is resized, so be sure
       to never use it after a call to Add(). */
    T* GetPointer(uint64_t hash);

    bool Get(uint64_t hash, T& data) const;
    bool Exists(uint64_t hash) const;
    size_t NumStored() const;
    size_t Memory() const;
    void Clear();

private:
    static const int START_SIZE = 1024;

    size_t m_size;
    size_t m_mask;
    size_t m_available;
    
    typedef struct
    {
        uint64_t hash;
        T data;
    } Data;
    Data* m_entry;

    void IncreaseSize();
    bool FindHash(uint64_t hash, size_t& index) const;
};

template<typename T>
HashMap<T>::HashMap()
    : m_size(START_SIZE),
      m_mask(START_SIZE - 1),
      m_available(START_SIZE * 3 / 4),
      m_entry(new Data [START_SIZE])
{
    Clear();
}

template<typename T>
HashMap<T>::HashMap(const HashMap& other)
    : m_size(other.m_size),
      m_mask(other.m_mask),
      m_available(other.m_available),
      m_entry(new Data [m_size])
{
    for (size_t i = 0; i < m_size; ++i) {
        m_entry[i] = other.m_entry[i];
    }
}

template<typename T>
HashMap<T>::~HashMap()
{
    delete[] m_entry;
}

template<typename T>
void HashMap<T>::IncreaseSize()
{
    const size_t old_size = m_size;
    m_size *= 2;
    m_mask = m_size - 1;
    m_available = m_size * 3 / 4;
    Data* old_entry = m_entry;
    m_entry = new Data [m_size];
    Clear();
    for (size_t i = 0; i < old_size; ++i) {
        if (old_entry[i].data.IsValid()) 
            Add(old_entry[i].hash, old_entry[i].data);
    }
    assert(m_available >= m_size*3/4 - (m_size/2*3/4));
    delete[] old_entry;
}

template<typename T>
void HashMap<T>::LoadFrom(const HashMap& other)
{
    // NOTE: NEED to do this if other is much larger
    // because we do linear probing... Can take a very
    // long time for FindHash() to finish if every 
    // slot is being filled sequentially. 
    while (m_size < other.NumStored())
        IncreaseSize();
    for (size_t i = 0; i < other.m_size; ++i) {
        if (other.m_entry[i].data.IsValid()) {
            Add(other.m_entry[i].hash,
                other.m_entry[i].data);
        }
    }
}

template<typename T>
T* HashMap<T>::Add(uint64_t hash, const T& data)
{
    if (m_available == 0) {
    	IncreaseSize();
    }   
    size_t index = 0;
    FindHash(hash, index);
    if (!m_entry[index].data.IsValid()) {
        m_available--;
        m_entry[index].hash = hash;
        m_entry[index].data = data;
        return &m_entry[index].data;
    } else if (m_entry[index].hash == hash) {
        m_entry[index].data = data;
        return &m_entry[index].data;
    }
    return NULL;
}

template<typename T>
bool HashMap<T>::FindHash(uint64_t hash, size_t& index) const
{
    for (index = hash; ;) {
        index &= m_mask;
        if (!m_entry[index].data.IsValid()) {
            return false;
        } else if (m_entry[index].hash == hash) {
            return true;
        }
        index++;
    }
    // should never get here
    assert(false);
    return false;
}

template<typename T>
bool HashMap<T>::Get(uint64_t hash, T& data) const
{
    size_t index;
    if (FindHash(hash, index)) {
        data = m_entry[index].data;
        return true;
    }
    return false;
}

template<typename T>
T* HashMap<T>::GetPointer(uint64_t hash)
{
    size_t index;
    if (FindHash(hash, index))
        return &m_entry[index].data;
    return NULL;
}


template<typename T>
bool HashMap<T>::Exists(uint64_t hash) const
{
    size_t index;
    return FindHash(hash, index);
}

template<typename T>
size_t HashMap<T>::NumStored() const
{
    return m_size * 3 / 4 - m_available;
}

template<typename T>
size_t HashMap<T>::Memory() const
{
	return sizeof(Data) * ( m_size * 3 / 4 - m_available );
    //return sizeof(Data) * m_size;
}

template<typename T>
void HashMap<T>::Clear()
{
    m_available = m_size * 3 / 4;    
    for (size_t i = 0; i < m_size; ++i)
        m_entry[i].data.Invalidate();
}

#endif // _HASHMAP_HPP
