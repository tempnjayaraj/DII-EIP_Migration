
#ifndef _FIFO_H
#define _FIFO_H


#include "SnTypes.h"

// Generic CFifo class for bytes
class CFifo
{
public:
    inline CFifo(void){}
    inline virtual ~CFifo(){}

    inline void Init(void)
    {
        m_pbHead = m_pbTail = m_pbBuf;
    }

    inline void DeInit(void)
    {
        m_pbHead = m_pbTail = m_pbBuf;
    }

    // Empty: H == T
    inline SnBool IsEmpty(void)
    {
        return m_pbHead == m_pbTail;
    }

    // Full: H == (T - 1)
    inline SnBool IsFull(void)
    {
        return
            ((m_pbHead == &m_pbBuf[iNUM_BYTES - 1]) && (m_pbTail == m_pbBuf))
            ||
            (m_pbHead == (m_pbTail - 1))
            ;
    }

    // Call only if room is available
    inline void Push(SnByte b)
    {
        *m_pbHead = b;
        if(m_pbHead == &m_pbBuf[iNUM_BYTES - 1]) {
            m_pbHead = m_pbBuf;
        } else {
            m_pbHead++;
        }
    }

    // Call only if data is available
    inline SnByte Pop(void)
    {
        SnByte b;

        b = *m_pbTail;
        if(m_pbTail == &m_pbBuf[iNUM_BYTES - 1]) {
            m_pbTail = m_pbBuf;
        } else {
            m_pbTail++;
        }
        return b;
    }

    // Call only if data is available
    inline SnByte PeekTail(void)
    {
        return *m_pbTail;
    }

    // Call only if data is available
    inline SnByte PeekHead(void)
    {
        if(m_pbHead == m_pbBuf) {
            return m_pbBuf[iNUM_BYTES - 1];
        } else {
            return *(m_pbHead - 1);
        }
    }

private:
    enum {iNUM_BYTES = 64};
    inline CFifo(const CFifo &oOld);    //prevent copying

    SnByte* m_pbHead;   // Points to next empty location
    SnByte* m_pbTail;   // Points to next full location
    SnByte m_pbBuf[iNUM_BYTES];
};


#endif // #ifndef _FIFO_H
