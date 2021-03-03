#include "ringbuffer.h"
#include <iostream>
using namespace std;

typedef unsigned int uint32;

template <class T>
class RingBuffer<T>::iterator
{

private:
    uint32 m_Position;
    RingBuffer &m_RingBuffer;

public:
    // Constructor
    iterator(uint32 position, RingBuffer ringBuffer) : m_Position(0), m_RingBuffer(ringBuffer)
    {
    }

    // Overload Operator Postfix ++
    iterator &operator++(int)
    {
        m_Position++;
        return *this;
    }

    // Overload operator prefix ++
    iterator &operator++()
    {
        m_Position++;
        return *this;
    }

    // Overload Operator de-reference operator
    T &operator*()
    {
        return m_RingBuffer.get(m_Position);
    }

    // Overload operator !=
    bool operator!=(const iterator &other) const
    {
        return (m_Position != other.m_Position);
    }
};

template <class T>
RingBuffer<T>::RingBuffer(unsigned int size) : m_Size(size), m_Position(0), m_Values(nullptr)
{
    // If memory is not sufficient, then
    // exception should be handled by caller
    m_Values = new T[m_Size + 1];
}

template <class T>
RingBuffer<T>::~RingBuffer()
{
    if (m_Values)
    {
        delete[] m_Values;
        m_Values = nullptr;
    }
}

template <class T>
void RingBuffer<T>::add(T value)
{
    m_Values[m_Position++] = value;
    if (m_Position == m_Size)
    {
        m_Position = 0;
    }
}

template <class T>
uint32 RingBuffer<T>::size() const
{
    return m_Size;
}

template <class T>
T &RingBuffer<T>::get(uint32 position)
{
    // There is no boundary check
    // Its caller responsibility
    return m_Values[position];
}