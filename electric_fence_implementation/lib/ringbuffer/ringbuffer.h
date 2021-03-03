using namespace std;

typedef unsigned int uint32;

template <class T>
class RingBuffer
{
private:
    uint32 m_Size;
    uint32 m_Position;
    T *m_Values;

public:
    RingBuffer(unsigned int size);
    ~RingBuffer();

    class iterator;

    void add(T value);
    uint32 size() const;
    T &get(uint32 position);
    iterator begin()
    {
        return iterator(0, *this);
    }
    iterator end()
    {
        return iterator(m_Size, *this);
    }
};