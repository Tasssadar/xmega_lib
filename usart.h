/*
 * USART lib for ATxmega chips
 * 
 * Options - use as "#define OPTION value":
 *     ENABLE_USART_C0        - enable usart interface. Interface is then available as
 *     ENABLE_USART_C1          usart_c0, usart_c1, ...
 *     ENABLE_USART_D0
 *     ENABLE_USART_D1
 *     ENABLE_USART_E0
 * 
 *     DEFAULT_BAUD           - default baud rate to set
 *
 *     DEFAULT_USART_RX_BUFF  - buffer size
 *     DEFAULT_USART_TX_BUFF
 *
 *     DISABLE_USART_FLOAT    - if set to 1, float calculations of baud
 *                              rate are disabled
 *
 *     USART_BSEL_HACK        - if set to 1, 1 is added to bsel register value.
 *                              To be used with DISABLE_USART_FLOAT
 */

#ifndef XMEGA_LIB_USART
#define XMEGA_LIB_USART

#include "queue.h"

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

#endif
