// MsgList.cpp: implementation of the MsgList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "shaver.h"
#include "MsgList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*
 * Define to print out all the details
 */
//#define MSG_LST_INFO   1

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MsgList::MsgList()
{
    // Initialize Free List of Msgs
    m_ptFreeHead = 0;
    m_ptWaitHead = m_ptWaitTail = 0;

    for (int iCnt = 0; iCnt < iMAX_WAIT_REQ; iCnt++) {
        FreeMsgNode(&m_ptMsgBlock[iCnt]);
    }
}

MsgList::~MsgList()
{
}

MsgWaitList *MsgList::AllocateMsgNode()
{
    MsgWaitList *ptMsg = m_ptFreeHead;
    
    if (ptMsg != NULL) {
        m_ptFreeHead = ptMsg->ptNext;
        if (m_ptFreeHead != NULL) {
            m_ptFreeHead->ptPrev = 0;
        }
        ptMsg->ptPrev = ptMsg->ptNext = ptMsg->ptHold = 0;
    }

    return ptMsg;
}

void MsgList::FreeMsgNode(MsgWaitList *ptMsg)
{
#ifdef MSG_LST_INFO
    DEBUGMSG(TRUE, (TEXT("FreeMsgNode(0x%x)\n"), ptMsg));
#endif
    ptMsg->ptPrev = 0;
    ptMsg->ptNext = m_ptFreeHead;
    if (m_ptFreeHead) {
        m_ptFreeHead->ptPrev = ptMsg;
    }
    m_ptFreeHead = ptMsg;
}

MSG_LST_RET MsgList::AddToMsgList(XmtMsg *ptXmtMsg, DWORD dwTime, XmtMsg *ptHoldMsg)
{
    MsgWaitList *ptMsg = AllocateMsgNode();

    if (ptMsg == NULL) {
        if (ptHoldMsg == NULL) {
            DEBUGMSG(TRUE, (TEXT("AddToMsgList(0x%02x, %d): No more free nodes\n"),
                ptXmtMsg->bReqNum, dwTime));
        } else {
            DEBUGMSG(TRUE, (TEXT("AddToMsgList(0x%02x, %d, 0x%02x): No more free nodes\n"),
                ptXmtMsg->bReqNum, dwTime, ptHoldMsg->bReqNum));
        }
        return MSG_LST_FAIL;
    }
    ptMsg->tXmtMsg = *ptXmtMsg;
    ptMsg->dwTimeSent = dwTime;
    ptMsg->ptPrev = m_ptWaitTail;
    ptMsg->ptNext = 0;

    if (m_ptWaitTail == NULL) {
#ifdef MSG_LST_INFO
        if (ptHoldMsg == NULL) {
            DEBUGMSG(TRUE, (TEXT("AddToMsgList(0x%02x, %d): { 0x%x } is only Node\n"),
                ptXmtMsg->bReqNum, dwTime, ptMsg));
        } else {
            DEBUGMSG(TRUE, (TEXT("AddToMsgList(0x%02x, %d, 0x%02x): { 0x%x } is only Node\n"),
                ptXmtMsg->bReqNum, dwTime, ptHoldMsg->bReqNum, ptMsg));
        }
#endif
        m_ptWaitHead = m_ptWaitTail = ptMsg;
    } else {
#ifdef MSG_LST_INFO
        if (ptHoldMsg == NULL) {
            DEBUGMSG(TRUE, (TEXT("AddToMsgList(0x%02x, %d): Add { 0x%x } to Tail\n"),
                ptXmtMsg->bReqNum, dwTime, ptMsg));
        } else {
            DEBUGMSG(TRUE, (TEXT("AddToMsgList(0x%02x, %d, 0x%02x): Add { 0x%x } to Tail\n"),
                ptXmtMsg->bReqNum, dwTime, ptHoldMsg->bReqNum, ptMsg));
        }
#endif
        m_ptWaitTail->ptNext = ptMsg;
        m_ptWaitTail = ptMsg;
    }

    return MSG_LST_ADDED;
}

MSG_LST_RET MsgList::ScheduleForAfterExistingRequest(XmtMsg *ptXmtMsg)
{
    MsgWaitList *ptMsg = m_ptWaitHead;
    SnByte bCmd = ptXmtMsg->pbMsg[1];

    while (ptMsg) {
        if (bCmd == ptMsg->tXmtMsg.pbMsg[1]) {
            if (ptMsg->ptHold == NULL) {
                if ((ptMsg->ptHold = AllocateMsgNode()) == NULL) {
                    DEBUGMSG(TRUE, (TEXT("ScheduleForAfterExistingRequest(0x%02x): No free nodes\n"), ptXmtMsg->bReqNum));
                    return MSG_LST_FAIL;
                }
                ptMsg->ptHold->tXmtMsg = *ptXmtMsg;
#ifdef MSG_LST_INFO
                DEBUGMSG(TRUE, (TEXT("ScheduleForAfterExistingRequest(0x%02x): Added to Hold Msg 0x%02x\n"),
                    ptXmtMsg->bReqNum, ptMsg->tXmtMsg.bReqNum));
#endif
            }
            return MSG_LST_ADDED;
        }
        ptMsg = ptMsg->ptNext;
    }

    return MSG_LST_NOT_FOUND;
}

DWORD MsgList::TimeInMsSince(DWORD dwTime)
{
    DWORD dw = GetTickCount();
    return (dw >= dwTime) ? dw - dwTime : ~(DWORD)0 - dwTime + dw + 1;
}

void MsgList::ExtractMsgNode(MsgWaitList *ptMsg, XmtMsg *ptXmtMsg, XmtMsg *ptHoldMsg)
{
#ifdef MSG_LST_INFO
    if (ptMsg == NULL) {
        DEBUGMSG(TRUE, (TEXT("ExtractMsgNode(0x%x, 0x%x, 0x%x): Null Msg\n"), ptMsg, ptXmtMsg, ptHoldMsg));
        return;
    } else {
        DEBUGMSG(TRUE, (TEXT("ExtractMsgNode(0x%x, 0x%x, 0x%x)\n"), ptMsg, ptXmtMsg, ptHoldMsg));
    }
#endif
    
    // Remove from wait list
    if (m_ptWaitHead == ptMsg) {
        m_ptWaitHead = ptMsg->ptNext;
    }
    if (m_ptWaitTail == ptMsg) {
        m_ptWaitTail = ptMsg->ptPrev;
    }
    if (ptMsg->ptPrev) {
        ptMsg->ptPrev->ptNext = ptMsg->ptNext;
    }
    if (ptMsg->ptNext) {
        ptMsg->ptNext->ptPrev = ptMsg->ptPrev;
    }

    // Copy data
    if (ptXmtMsg != NULL) {
        *ptXmtMsg = ptMsg->tXmtMsg;
    }
    if (ptHoldMsg != NULL && ptMsg->ptHold != NULL) {
        *ptHoldMsg = ptMsg->ptHold->tXmtMsg;
    }

    // Free node(s)
    if (ptMsg->ptHold) {
        FreeMsgNode(ptMsg->ptHold);
        ptMsg->ptHold = 0;
    }
    FreeMsgNode(ptMsg);
}

MSG_LST_RET MsgList::RemoveExpiredMsg(SnWord wMaxWait, XmtMsg *ptXmtMsg, XmtMsg *ptHoldMsg)
{
    MsgWaitList *ptMsg = m_ptWaitHead;

#ifdef MSG_LST_INFO
    DEBUGMSG(TRUE, (TEXT("RemoveExpiredMsg(0x%02x)\n"), wMaxWait));
#endif
    while (ptMsg) {
        if (TimeInMsSince(ptMsg->dwTimeSent) >= wMaxWait) {
            ExtractMsgNode(ptMsg, ptXmtMsg, ptHoldMsg);
            return MSG_LST_REMOVED;
        }
        ptMsg = ptMsg->ptNext;
    }

    return MSG_LST_NOT_FOUND;
}

MSG_LST_RET MsgList::RemoveReqMsg(SnByte bReqNum, XmtMsg *ptXmtMsg, XmtMsg *ptHoldMsg)
{
    MsgWaitList *ptMsg = m_ptWaitHead;

#ifdef MSG_LST_INFO
    DEBUGMSG(TRUE, (TEXT("RemoveReqMsg(0x%02x)\n"), bReqNum));
#endif
    while (ptMsg) {
        if (bReqNum == ptMsg->tXmtMsg.bReqNum) {
            ExtractMsgNode(ptMsg, ptXmtMsg, ptHoldMsg);
            return MSG_LST_REMOVED;
        }
        ptMsg = ptMsg->ptNext;
    }

    return MSG_LST_NOT_FOUND;
}

void MsgList::EmptyMsgList()
{
    MsgWaitList *ptMsg = m_ptWaitHead;

#ifdef MSG_LST_INFO
    DEBUGMSG(TRUE, (TEXT("EmptyMsgList()\n")));
#endif
    while (ptMsg) {
        MsgWaitList *ptMsgNext = ptMsg->ptNext;
        ExtractMsgNode(ptMsg, NULL, NULL);
        ptMsg = ptMsgNext;
    }
}

int MsgList::GetLongestTimeRequestHasBeenWaiting()
{
    MsgWaitList *ptMsg = m_ptWaitHead;
    int iMaxWaitTime = 0;

    while (ptMsg) {
        int iWaitTime = TimeInMsSince(ptMsg->dwTimeSent);
        if (iWaitTime >= iMaxWaitTime) {
            iMaxWaitTime = iWaitTime;
        }
        ptMsg = ptMsg->ptNext;
    }

    return iMaxWaitTime;
}
