// MsgList.h: interface for the MsgList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGLIST_H__BF0772A6_38D6_459A_BFC7_30804B9FD315__INCLUDED_)
#define AFX_MSGLIST_H__BF0772A6_38D6_459A_BFC7_30804B9FD315__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define iMAX_MSG_SIZE 64            // Max # of bytes for a Msg
#define iMAX_WAIT_REQ 16            // Max # of Requests that are waiting for a Reply

typedef struct XmtMsg {
    SnByte bTask;					// Task for Xmt thread
    SnByte bReqNum;                 // Request Number for XMT_TASK_RCV_ACK and XMT_TASK_RCV_NAK
    SnByte bRetries;                // Retry Count
    SnByte bNumBytes;			    // # valid bytes in pbMsg, can be 0
    SnByte pbMsg[iMAX_MSG_SIZE];	// Bytes to write
} XmtMsg;

typedef struct {
    SnByte bCmd;
    SnByte bReqNum;                 // Request Number of received Msg
    SnByte bCmdDataLen;
    SnByte pbCmdData[iMAX_MSG_SIZE];
} RcvMsg;

typedef struct MsgWaitList {
    XmtMsg tXmtMsg;
    DWORD dwTimeSent;               // Set to System Tick Count when Msg was sent
    struct MsgWaitList *ptPrev;
    struct MsgWaitList *ptNext;
    struct MsgWaitList *ptHold;
} MsgWaitList;

typedef enum {
	MSG_LST_ADDED,
	MSG_LST_REMOVED,
    MSG_LST_NOT_FOUND,
	MSG_LST_FAIL
} MSG_LST_RET;

class MsgList  
{
public:
	MsgList();
	virtual ~MsgList();

    MSG_LST_RET AddToMsgList(XmtMsg *ptXmtMsg, DWORD dwTime, XmtMsg *ptHoldMsg);
    MSG_LST_RET RemoveExpiredMsg(SnWord wMaxWait, XmtMsg *ptXmtMsg, XmtMsg *ptHoldMsg);
    MSG_LST_RET RemoveReqMsg(SnByte bReqNum, XmtMsg *ptXmtMsg, XmtMsg *ptHoldMsg);
    MSG_LST_RET ScheduleForAfterExistingRequest(XmtMsg *ptXmtMsg);
    void EmptyMsgList();
    int GetLongestTimeRequestHasBeenWaiting();

    DWORD TimeInMsSince(DWORD dwTime);

private:
    MsgWaitList m_ptMsgBlock[iMAX_WAIT_REQ];                    // Maximum Requests pending before error
    MsgWaitList *m_ptFreeHead, *m_ptFreeTail;
    MsgWaitList *m_ptWaitHead, *m_ptWaitTail;

    void FreeMsgNode(MsgWaitList *ptMsg);
    MsgWaitList *AllocateMsgNode();
    void ExtractMsgNode(MsgWaitList *ptMsg, XmtMsg *ptXmtMsg, XmtMsg *ptHoldMsg);
};

#endif // !defined(AFX_MSGLIST_H__BF0772A6_38D6_459A_BFC7_30804B9FD315__INCLUDED_)
