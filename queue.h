/*
 * Queue container implementation
 */

#ifndef XMEGA_LIB_QUEUE
#define XMEGA_LIB_QUEUE

template <typename T, uint8_t size>
class queue_t
{
public:
    queue_t()
    {
        m_wr_itr = 0;
        m_rd_itr = 0;
    }

    bool push(T data)
    {
        uint8_t wr = m_wr_itr;

        uint8_t new_itr = incIfCan(m_wr_itr);

        if(new_itr == m_rd_itr)
            return false;

        m_data[wr] = data;
        m_wr_itr = new_itr;
        return true;
    }

    bool empty()
    {
        return m_rd_itr == m_wr_itr;
    }

    bool full()
    {
        return incIfCan(m_wr_itr) == m_rd_itr;
    }

    T top() const
    {
        return m_data[m_rd_itr];
    }

    void pop()
    {
        m_rd_itr = incIfCan(m_rd_itr);
    }

private:
    uint8_t incIfCan(uint8_t val)
    {
        ++val;
        return val == size ? 0 : val;
    }

    T m_data[size];

    volatile uint8_t m_wr_itr;
    volatile uint8_t m_rd_itr;
};

#endif
