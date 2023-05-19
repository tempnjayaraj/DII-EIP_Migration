#include "Controller.h"

// Some bits in the control register
#define FLEXCAN_FCMCR_FREEZ_ACK	((SnWord)1 << 8)
#define FLEXCAN_FCMCR_SOFT_RST	((SnWord)1 << 9)
#define FLEXCAN_FCMCR_HALT 		((SnWord)1 << 12)
#define FLEXCAN_FCMCR_FRZ1 		((SnWord)1 << 14)

// Setup rcv buf to respond to msg id of 1
void FlexCanPrepRcvBuf(int iBufIndex,SnBool yExtendedFrame)
{
	// Write the Control/Status word to Hold the Receive MB Inactive (CODE = 0000 binary)
	ptFlexCan->ptMB[iBufIndex].wFCMB_CTL = 0 << 4;
	
	// Write the ID_HIGH and ID_LOW words
	if(yExtendedFrame) {
		ptFlexCan->ptMB[iBufIndex].wFCMB_ID_H = 0
		| (1 << 3)		// IDE = 1
		;

		ptFlexCan->ptMB[iBufIndex].wFCMB_ID_L = 0
		| (1 << 1)
		;
	} else {
		ptFlexCan->ptMB[iBufIndex].wFCMB_ID_H = 0
		;

		ptFlexCan->ptMB[iBufIndex].wFCMB_ID_L = 0
		|
		(1 << 5)
		;
	}
	
	// Write the Control/Status word to mark the Receive MB as Active and Empty (CODE = 0100 binary)
	ptFlexCan->ptMB[iBufIndex].wFCMB_CTL = 4 << 4;
}

// Init for CAN, adapted from processor expert output
// Setup CAN for 0.5MB/s operation, with 8 quanta per bit (4MHz quanta)
// MB0 is setup for XMT, MB1 for RCV
void FlexCanInit(void)
{
    int iCnt;
    volatile SnWord w;
    
 	ptFlexCan->wFCMCR |= FLEXCAN_FCMCR_SOFT_RST;    /* Soft-reset of the FlexCAN module */
 	w = *(volatile SnWord*)0x002000;                /* Data flash read to kill time */

	/* Wait for entering the debug mode */
	while(!(ptFlexCan->wFCMCR & FLEXCAN_FCMCR_FREEZ_ACK)) {
		;
	}

	/* FCMAXMB: ??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,??=0,MAXMB=15 */
	ptFlexCan->wFCMAXMB = 1;                /* Set the Message buffer register */ 

	/* Initialization of all message buffers */
	ptFlexCan->ptMB[0].wFCMB_CTL =  0x0000;     /* Initialize of the message buffer 0 as a transmit buffer */
	ptFlexCan->ptMB[0].wFCMB_ID_H = 0x0010;     /* Set ID_HIGH register of the message buffer 0 */
	ptFlexCan->ptMB[0].wFCMB_ID_L = 0x0004;
	ptFlexCan->ptMB[1].wFCMB_CTL =  0x0040;     /* Initialize of the message buffer 1 as a receive buffer */
	ptFlexCan->ptMB[1].wFCMB_ID_H = 0x0008;     /* Set ID_HIGH register of the message buffer 1 */
	ptFlexCan->ptMB[1].wFCMB_ID_L = 0x0020;
	for(iCnt = 2; iCnt <= 15; iCnt++) {
	    ptFlexCan->ptMB[iCnt].wFCMB_CTL = 0;    // Not active	    
	}

	/* FCRXGMASK_L: MID14=1,MID13=1,MID12=1,MID11=1,MID10=1,MID9=1,MID8=1,MID7=1,MID6=1,MID5=1,MID4=1,MID3=1,MID2=1,MID1=1,MID0S=1,??=0 */
	ptFlexCan->wFCRnGMASK_L = 0xfffe;       /* Set the acceptance mask register for the buffers 0-13 */ 
	/* FCRXGMASK_H: MID28=1,MID27=1,MID26=1,MID25=1,MID24=1,MID23=1,MID22=1,MID21=1,MID20=1,MID19=1,MID18=1,??=0,??=1,MID17=1,MID16=1,MID15=1 */
	ptFlexCan->wFCRnGMASK_H = 0xffef;       /* Set the acceptance mask register for the buffers 0-13 */ 
	/* FCRX14MASK_L: MID14=1,MID13=1,MID12=1,MID11=1,MID10=1,MID9=1,MID8=1,MID7=1,MID6=1,MID5=1,MID4=1,MID3=1,MID2=1,MID1=1,MID0S=1,??=0 */
	ptFlexCan->wFCRn14MASK_L = 0xfffe;      /* Set the acceptance mask register for the buffer 14 */ 
	/* FCRX14MASK_H: MID28=1,MID27=1,MID26=1,MID25=1,MID24=1,MID23=1,MID22=1,MID21=1,MID20=1,MID19=1,MID18=1,??=0,??=1,MID17=1,MID16=1,MID15=1 */
	ptFlexCan->wFCRn14MASK_H = 0xffef;      /* Set the acceptance mask register for the buffer 14 */ 
	/* FCRX15MASK_L: MID14=1,MID13=1,MID12=1,MID11=1,MID10=1,MID9=1,MID8=1,MID7=1,MID6=1,MID5=1,MID4=1,MID3=1,MID2=1,MID1=1,MID0S=1,??=0 */
	ptFlexCan->wFCRn15MASK_L = 0xfffe;      /* Set the acceptance mask register for the buffer 15 */ 
	/* FCRX15MASK_H: MID28=1,MID27=1,MID26=1,MID25=1,MID24=1,MID23=1,MID22=1,MID21=1,MID20=1,MID19=1,MID18=1,??=0,??=1,MID17=1,MID16=1,MID15=1 */
	ptFlexCan->wFCRn15MASK_H = 0xffef;      /* Set the acceptance mask register for the buffer 15 */ 
	/* FCIMASK1: BUF15M=0,BUF14M=0,BUF13M=0,BUF12M=0,BUF11M=0,BUF10M=0,BUF9M=0,BUF8M=0,BUF7M=0,BUF6M=0,BUF5M=0,BUF4M=0,BUF3M=0,BUF2M=0,BUF1M=0,BUF0M=0 */
	ptFlexCan->wFCIMASK1 = 0;               /* Enable/Disable corresponding FlexCAN message buffer interrupt */ 
	/* FCCTL1: PRES_DIV=14,RJW=0,PSEG1=0,PSEG2=0 */
	/* UH: Changed PSEG1=3, PSEG2=1 */
	//ptFlexCan->wFCCTL1 = 3584;              /* Set the device control/timing register */ 
	ptFlexCan->wFCCTL1 = 0
		| (4 << 8)			// PRES_DIV = 4 => SCLK = 60 / (4 + 1) = 12MHz
		| (2 << 6)			// RJW = 2 => actual = 3
		| (2 << 3)			// PSEG1 = 2 => actual = 3
		| (2 << 0)			// PSEG2 = 2 => actual = 3
		;
	/* FCCTL0: BUSOFF_MASK=0,ERROR_MASK=0,??=0,??=0,??=0,??=0,??=0,??=0,SAMP=0,LOOPB=0,TSYNC=0,LBUF=0,LOM=0,PROPSEG=4 */
	ptFlexCan->wFCCTL0 = 4;                 /* Set the Control 0 register */ 	
	/* FCMCR: STOP=0,FRZ1=1,??=0,HALT=0,NOT_RDY=0,WAKE_MASK=0,SOFT_RST=0,FREEZ_ACK=0,??=0,SELF_WAKE=0,AUTO_POWER_SAVE=0,STOP_ACK=0,??=0,??=0,??=0,??=0 */
	ptFlexCan->wFCMCR &= ~(FLEXCAN_FCMCR_HALT | FLEXCAN_FCMCR_FRZ1); /* Enable FlexCAN device in debug mode */
	
	FlexCanPrepRcvBuf(1,TRUE);
}

void FlexCanReadMsgData(int iBufIndex, int iNumBytes, SnByte* pbBuf)
{ 
	SnWord *pwDataBytes = (SnWord *)&ptFlexCan->ptMB[iBufIndex].pbFCMB_DATA[0];
	int iDataByte = 0;

  	while (iDataByte < iNumBytes) {
  	    SnWord wPair = *pwDataBytes++;
  	    
  	    pbBuf[iDataByte++] = (wPair >> 8);
  	    if (iDataByte < iNumBytes) {
  	        pbBuf[iDataByte++] = (wPair & 0xff);
  	    }
  	}
}

void FlexCanWriteMsgData(int iBufIndex, int iNumBytes, const SnByte* pbBuf)
{ 
	SnWord *pwDataBytes = (SnWord *)&ptFlexCan->ptMB[iBufIndex].pbFCMB_DATA[0];
	int iDataByte = 0;

  	while (iDataByte < iNumBytes) {
  	    SnWord wPair = pbBuf[iDataByte++] << 8;
  	    if (iDataByte < iNumBytes) {
  	        wPair |= pbBuf[iDataByte++];
  	    }
  	    *pwDataBytes++ = wPair;
  	}
}

// Transmit a CAN message, return TRUE on success
void FlexCanXmtMsg(int iBufIndex,SnBool yExtendedFrame,SnQByte qMsgId,int iNumBytes,const SnByte* pbBuf)
{
  	// Write Control/Status word to hold the transmit MB inactive (CODE = 1000 binary)
  	ptFlexCan->ptMB[iBufIndex].wFCMB_CTL = 0x8 << 4;
  	
  	// Write the ID_HIGH and ID_LOW words
  	if(yExtendedFrame) {
	  	ptFlexCan->ptMB[iBufIndex].wFCMB_ID_H =	0
	  		| ((((SnWord)(qMsgId >> 18)) & 0x7ff) << 5)	// Bits 28 - 18 of ID
	  		| (1 << 4)									// Set SRR bit for Extended frames
	  		| (1 << 3)									// Or in IDE bit (for extended messages)
	  		| (((SnWord)(qMsgId >> 15)) & 0x7)			// Bits 17 - 15 of ID
	  		;
	  	ptFlexCan->ptMB[iBufIndex].wFCMB_ID_L = 0
	  		| ((SnWord)qMsgId << 1)						// Bits 14 - 0 of ID
	  		| (0 << 0)              					// RTR = 0 for data frame, 1 for remote frame
	  		;
  	} else {
	  	ptFlexCan->ptMB[iBufIndex].wFCMB_ID_H =	0
	  		| ((((SnWord)(qMsgId >> 18)) & 0x7ff) << 5)
	  		| (0 << 4)					                // RTR = 0 for data frame, 1 for remote frame
	  		// Lower 4 bits are 0
	  		;
	  	ptFlexCan->ptMB[iBufIndex].wFCMB_ID_L = 0;
  	}
  	
  	// Write the data bytes
  	FlexCanWriteMsgData(iBufIndex, iNumBytes, pbBuf);

  	// Write the Control/ Status word (active CODE, LENGTH)
  	ptFlexCan->ptMB[iBufIndex].wFCMB_CTL = 0
  		| (0xC << 4)									// CODE = 1100 binary (active)
  		| (iNumBytes & 0xf)								// LENGTH
  		;
  		
	// At the end of transmission:
  		// The value of the Free-Running Timer is written into the TIME_STAMP field in the MB
  		// The CODE field in the Control/Status word of the MB is updated
  		// A Status Flag is set in the Interrupt Flag Register (FCIFLAG1, Section 7.8.12)
  	
  	// Poll for xmt to finish
  	while(0 == (ptFlexCan->wFCIFLAG1 & (1 << iBufIndex))) {
  		;
  	}

  	// Clear interrupt flag by writing 1 to it
	ptFlexCan->wFCIFLAG1 |= (1 << iBufIndex);
}

void DeferredFlexCanActions(void)
{
    static const SnByte pbRCV_MSG[8] = {
    	1 << 0,
    	1 << 1,
    	1 << 2,
    	1 << 3,
    	1 << 4,
    	1 << 5,
    	1 << 6,
    	1 << 7
    };
	static const SnByte pbXMT_MSG[8] = {
    	1 << 7,
    	1 << 6,
    	1 << 5,
    	1 << 4,
    	1 << 3,
    	1 << 2,
    	1 << 1,
    	1 << 0,
	};

	if(ptFlexCan->wFCIFLAG1 & (1 << 1)) {
        SnByte pbMsgData[8];
        SnBool yValidMsg = FALSE;
	
		// Receive message is pending
		
		// Clear status
		ptFlexCan->wFCIFLAG1 = 1 << 1;
		
		// Check message contents
		FlexCanReadMsgData(1, 8, pbMsgData);
		yValidMsg = !memcmp(pbRCV_MSG, pbMsgData, 8);

        // Prep new Receive
	    FlexCanPrepRcvBuf(1,TRUE);
	    
	    // Send back Ack Message if all is good
	    if (yValidMsg) {
    		FlexCanXmtMsg(
    			0,						//int iBufIndex
    			TRUE,					//SnBool yExtendedFrame
    			2,						//SnQByte qMsgId
    			8,						//int iNumBytes
    			pbXMT_MSG			    //const SnByte* pbBuf
    		);
	    }
	}
}