#include <queue>
#include <deque>

template <typename T, int len, typename Container = std::deque<T>>
class FixedQueue : public std::queue<T, Container>
{
public:
    void push(const T &value)
    {
        if (this->size() == len)
        {
            this->c.pop_front();
        }
        std::queue<T, Container>::push(value);
    }
};