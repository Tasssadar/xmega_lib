#include <avr/io.h>

#ifndef F_PER
    #error "F_PER not defined"
#endif

#ifndef DEFAULT_BAUD
    #define DEFAULT_BAUD 38400
#endif

#ifndef DEFAULT_USART_RX_BUFF
    #define DEFAULT_USART_RX_BUFF 32
#endif

#ifndef DEFAULT_USART_TX_BUFF
    #define DEFAULT_USART_TX_BUFF 96
#endif

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

enum usart_part
{
    USART_RECEIVER    = 0,
    USART_TRANSMITER  = 1
};

template <uint8_t rxbuf, uint8_t txbuf>
class usart_t
{
public:
    usart_t(USART_t& usart)
    {
        m_usart = &usart;
    }

    void init()
    {
        setPartStatus(USART_RECEIVER, true);
        setPartStatus(USART_TRANSMITER, true);

        m_usart->CTRLA = (USART_RXCINTLVL_LO_gc | USART_DREINTLVL_LO_gc);

        m_usart->CTRLB |= USART_CLK2X_bm;
        m_usart->CTRLC |= USART_CHSIZE_8BIT_gc;

        setBaud(DEFAULT_BAUD);
    }

    void setBaud(uint32_t baud, uint8_t bscale = 0)
    {
#if DISABLE_USART_FLOAT
        uint16_t bsel = (F_PER/( (1 << bscale) * 8*baud) - 1);
#else
        double bsel_d = (float(F_PER)/( (1 << bscale) * 8*baud) - 1.0);
        uint16_t bsel = uint16_t(bsel_d);

        if(bsel_d - bsel >= 0.5)
            ++bsel;
#endif
#if USART_BSEL_HACK
        ++bsel;
#endif
        m_usart->BAUDCTRLB = (bscale << 4) | (bsel >> 8);
        m_usart->BAUDCTRLA = (bsel & 0xFF);
    }

    void setPartStatus(usart_part part, bool enable)
    {
        static const uint8_t part_mask[] = { USART_RXEN_bm, USART_TXEN_bm };
        if(enable)
            m_usart->CTRLB |= part_mask[part];
        else
            m_usart->CTRLB &= ~(part_mask[part]);
    }

    void readData()
    {
        m_rxbuf.push(m_usart->DATA);
    }

    void writeData()
    {
        if(m_txbuf.empty())
            return;

        m_usart->DATA = m_txbuf.top();
        m_txbuf.pop();
    }

    bool peek(uint8_t &ch)
    {
        if(m_rxbuf.empty())
            return false;

        ch = m_rxbuf.top();
        m_rxbuf.pop();
        return true;
    }

    uint8_t get()
    {
        uint8_t res = 0;
        while(!peek(res))
        {
            asm("nop");
        }
        return res;
    }

    void send(const char *text)
    {
        for(; *text; ++text)
            m_txbuf.push(*text);
    }

private:

    USART_t *m_usart;
    bool m_ready_send;

    queue_t<uint8_t, rxbuf> m_rxbuf;
    queue_t<uint8_t, txbuf> m_txbuf;
};


#if ENABLE_USART_C0
    usart_t<DEFAULT_USART_RX_BUFF, DEFAULT_USART_TX_BUFF> usart_c0(USARTC0);

    ISR(USARTC0_RXC_vect)
    {
        usart_c0.readData();
    }

    ISR(USARTC0_DRE_vect)
    {
        usart_c0.writeData();
    }
#endif

#if ENABLE_USART_C1
    usart_t<DEFAULT_USART_RX_BUFF, DEFAULT_USART_TX_BUFF> usart_c1(USARTC1);

    ISR(USARTC1_RXC_vect)
    {
        usart_c1.readData();
    }

    ISR(USARTC1_DRE_vect)
    {
        usart_c1.writeData();
    }
#endif

#if ENABLE_USART_D0
    usart_t<DEFAULT_USART_RX_BUFF, DEFAULT_USART_TX_BUFF> usart_d0(USARTD0);

    ISR(USARTD0_RXC_vect)
    {
        usart_d0.readData();
    }

    ISR(USARTD0_DRE_vect)
    {
        usart_d0.writeData();
    }
#endif

#if ENABLE_USART_D1
    usart_t<DEFAULT_USART_RX_BUFF, DEFAULT_USART_TX_BUFF> usart_d1(USARTD1);

    ISR(USARTD1_RXC_vect)
    {
        usart_d1.readData();
    }

    ISR(USARTD1_DRE_vect)
    {
        usart_d1.writeData();
    }
#endif

#if ENABLE_USART_E0
    usart_t<DEFAULT_USART_RX_BUFF, DEFAULT_USART_TX_BUFF> usart_e0(USARTE0);

    ISR(USARTE0_RXC_vect)
    {
        usart_e0.readData();
    }

    ISR(USARTE0_DRE_vect)
    {
        usart_e0.writeData();
    }
#endif

void init_usart()
{
#if ENABLE_USART_C0
    PORTC_OUT |= (1<<3);
    PORTC_DIR |= (1<<3);
    usart_c0.init();
#endif

#if ENABLE_USART_C1
    PORTC_OUT |= (1<<7);
    PORTC_DIR |= (1<<7);
    usart_c1.init();
#endif

#if ENABLE_USART_D0
    PORTD_OUT |= (1<<3);
    PORTD_DIR |= (1<<3);
    usart_d0.init();
#endif

#if ENABLE_USART_D1
    PORTD_OUT |= (1<<7);
    PORTD_DIR |= (1<<7);
    usart_d1.init();
#endif

#if ENABLE_USART_E0
    PORTE_OUT |= (1<<3);
    PORTE_DIR |= (1<<3);
    usart_e0.init();
#endif
}


