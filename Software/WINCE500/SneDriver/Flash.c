//
// Includes common to all the MDD files.
//
#define WINCEOEM 1

#include <windows.h>

#include "SnTypes.h"
#include "SneDriver.h"

#define DQ6_MASK (0x40)

#define WRITE_TO_FLASH(aAddr, wValue) \
	*(volatile SnWord *)(aAddr) = (SnWord)(wValue)
#define WRITE_TO_FLASH_OFFS(aAddr, aOffs, wValue) \
	*((volatile SnWord *)(aAddr) + (aOffs)) = (SnWord)(wValue)

SnQByte g_qFlashOffset = FLASH_DEFAULT_OFFSET;
SnAddr g_aFlashBase;
SnAddr g_aFlashEnd;

SnQByte g_qStoreMaxSize;
SnAddr g_aStoreFlashBase;
SnAddr g_aStoreFlashTop;
SnAddr g_aStoreFlashAddr;
SnByte *g_pbStoreBuf;

ULONG FlashInterruptThread(PVOID pContext)
{

	NKDbgPrintfW(TEXT("FlashInterruptThread(0x%x)\n\r"), pContext);

	// yShutdown is only set in deinit (by StopThreads)
    while (!g_yShutdown) {
		// Block on interrupt event
        WaitForSingleObject(g_hFlashInterrupt, INFINITE);

        // If the Flash is idle, (Busy is not low), then wake up any Flash Wait Routine
        if (g_ptPioA->qPIO_PDSR & 0x400000)
            SetEvent(g_hFlashWaitEvent);

        // Notify the system the interrupt is finished
		InterruptDone(g_dwFlashSysIntr);
    }

	NKDbgPrintfW(TEXT("FlashInterruptThread(0x%x) - Shutdown!\n\r"), pContext);

    // Should never get here unless the driver is unloading
    return 0;
}

SnBool WaitForEraseComplete(SnAddr aSectorAddr)
{
    volatile SnWord wRead1, wRead2;
    SnQByte qStart, qEnd;
    DWORD dwWaitRet;

    qStart = GetTickCount();
    dwWaitRet = WaitForSingleObject(g_hFlashWaitEvent, FLASH_ERASE_TIMEOUT_IN_MS);
    qEnd = GetTickCount();

    if (dwWaitRet != WAIT_OBJECT_0) {
        NKDbgPrintfW(TEXT("WaitForEraseComplete(0x%08x): Error/Timeout {%dms}\n"), aSectorAddr, qEnd-qStart);
        return FALSE;
    }

	wRead1 = *(volatile SnWord *)aSectorAddr;
	wRead2 = *(volatile SnWord *)aSectorAddr;

	/* Checking for DQ6 Toggling */
	if ((wRead1 ^ wRead2) & DQ6_MASK) {
        NKDbgPrintfW(TEXT("WaitForEraseComplete(0x%08x): Toggle {%dms}\n"), aSectorAddr, qEnd-qStart);
        return FALSE;
    }

    NKDbgPrintfW(TEXT("WaitForEraseComplete(0x%08x): {%dms}\n"), aSectorAddr, qEnd-qStart);

    return TRUE;
}

SnBool EraseFlashPages(SnQByte qPages)
{
    SnAddr aSectorAddr = g_aFlashBase + g_qFlashOffset;

    if (qPages == 0 || !IS_FLASH_PAGE_ALIGNED(g_qFlashOffset) ||
        (g_qFlashOffset + (FLASH_PAGE_SIZE * qPages)) > FLASH_UPPER_LIMIT) {
        return FALSE;
    }

    /* Erase all Sectors Selected */

    for (;qPages-- > 0; aSectorAddr += FLASH_PAGE_SIZE) {
	    WRITE_TO_FLASH_OFFS(g_aFlashBase, 0x555, 0x00AA);	// Unlock Cycle 1
	    WRITE_TO_FLASH_OFFS(g_aFlashBase, 0x2AA, 0x0055);	// Unlock Cycle 2
	    WRITE_TO_FLASH_OFFS(g_aFlashBase, 0x555, 0x0080);	// Setup Command
	    WRITE_TO_FLASH_OFFS(g_aFlashBase, 0x555, 0x00AA);	// Extra Unlock Cycle 1
	    WRITE_TO_FLASH_OFFS(g_aFlashBase, 0x2AA, 0x0055);	// Extra Unlock Cycle 2

		WRITE_TO_FLASH(aSectorAddr, 0x0030);		// Erase Sector

        if (WaitForEraseComplete(aSectorAddr) == FALSE)
            return FALSE;
    }

	return TRUE;
}

SnBool WaitForWriteComplete(SnAddr aSectorAddr)
{
    volatile SnWord wRead1, wRead2;
    SnQByte qStart, qEnd;
    DWORD dwWaitRet;

    qStart = GetTickCount();
    dwWaitRet = WaitForSingleObject(g_hFlashWaitEvent, FLASH_WRITE_TIMEOUT_IN_MS);
    qEnd = GetTickCount();

    if (dwWaitRet != WAIT_OBJECT_0) {
        NKDbgPrintfW(TEXT("WaitForWriteComplete(0x%08x): Error/Timeout {%dms}\n"), aSectorAddr, qEnd-qStart);
        return FALSE;
    }

	wRead1 = *(volatile SnWord *)aSectorAddr;
	wRead2 = *(volatile SnWord *)aSectorAddr;

	/* Checking for DQ6 Toggling */
	if ((wRead1 ^ wRead2) & DQ6_MASK) {
        NKDbgPrintfW(TEXT("WaitForWriteComplete(0x%08x): Toggle {%dms}\n"), aSectorAddr, qEnd-qStart);
        return FALSE;
    }

    if (qEnd - qStart > 1)
        NKDbgPrintfW(TEXT("WaitForWriteComplete(0x%08x): {%dms}\n"), aSectorAddr, qEnd-qStart);

    return TRUE;
}

SnBool WriteFlashData(volatile SnQByte *pqData, SnQByte qBytes)
{
    SnAddr aSectorAddr = g_aFlashBase + FLASH_ALIGNED_OFFSET(g_qFlashOffset);
    volatile SnWord *pwFlash = (SnWord *)(g_aFlashBase + g_qFlashOffset);
	volatile SnWord *pwData = (volatile SnWord *)pqData;
    SnQByte qWords = qBytes >> 1;
    SnQByte qCnt;

    if (qBytes == 0 || (qBytes & 3) || (g_qFlashOffset + qBytes) > FLASH_UPPER_LIMIT) {
        return FALSE;
    }

    /*
     * The block write must start on a 64 byte boundary, so do single word writes
     * until we reach the next 64 byte boundary.
     */ 
    while (((SnAddr)pwFlash & 0x3f) != 0 && qWords > 0) {
		WRITE_TO_FLASH_OFFS(g_aFlashBase, 0x555, 0x00AA);	// Unlock Cycle 1
		WRITE_TO_FLASH_OFFS(g_aFlashBase, 0x2AA, 0x0055);	// Unlock Cycle 2
		WRITE_TO_FLASH_OFFS(g_aFlashBase, 0x555, 0x00A0);	// Write Word

        *pwFlash++ = *pwData++;

		if (WaitForWriteComplete(aSectorAddr) == FALSE)
			return FALSE;

        qWords--;
    }

    /*
     * The block write can FLASH up to 32 SnWord's at a time. So break up the data
     * into 32 SnWord chunks. 
     */ 
    while (qWords > 0) {
        if (IS_FLASH_PAGE_ALIGNED((SnAddr)pwFlash)) {
            aSectorAddr = (SnAddr)pwFlash;
        }

		WRITE_TO_FLASH_OFFS(g_aFlashBase, 0x555, 0x00AA);	// Unlock Cycle 1
		WRITE_TO_FLASH_OFFS(g_aFlashBase, 0x2AA, 0x0055);	// Unlock Cycle 2
		WRITE_TO_FLASH(aSectorAddr, 0x0025);			// Write Bufer Load Command

        qCnt = (qWords >= 0x20) ? 0x20 : qWords;
        qWords -= qCnt;

        /* The block write uses N-1 for its count */
        qCnt--;

		WRITE_TO_FLASH(aSectorAddr, qCnt);		// Write # of Words
        do {
			*pwFlash++ = *pwData++;				// Write Data
        } while (qCnt-- > 0);

		WRITE_TO_FLASH(aSectorAddr, 0x0029);	// Write Buffer to Flash Command

		if (WaitForWriteComplete(aSectorAddr) == FALSE)
			return FALSE;
    }

    return TRUE;
}

SnBool ReadFlashData(volatile SnQByte *pqData, SnQByte qBytes)
{
    volatile SnQByte *pqFlash = (SnQByte *)(g_aFlashBase + g_qFlashOffset);
    SnQByte qWords = qBytes >> 2;

    if (qBytes == 0 || (qBytes & 3) || (g_qFlashOffset + qBytes) > FLASH_UPPER_LIMIT) {
        return FALSE;
    }

    do {
        *pqData++ = *pqFlash++;
    } while (--qWords > 0);

    return TRUE;
}

SnBool VerifyFlashData(volatile SnQByte *pqData, SnQByte qBytes)
{
    volatile SnQByte *pqFlash = (SnQByte *)(g_aFlashBase + g_qFlashOffset);
    SnQByte qWords = qBytes >> 2;

    if (qBytes == 0 || (qBytes & 3) || (g_qFlashOffset + qBytes) > FLASH_UPPER_LIMIT) {
        return FALSE;
    }

    do {
        if (*pqData++ != *pqFlash++) {
            return FALSE;
        }
    } while (--qWords > 0);

    return TRUE;
}

SnBool CrcBufData(SnByte *pbData, SnQByte qBytes, SnByte *pbCrc)
{
    SnByte bCrc = (pbCrc) ? *pbCrc : 0;

    while (qBytes-- > 0) {
        bCrc = g_pbCrcTable[bCrc ^ *pbData++];
    }

    if (pbCrc) {
        *pbCrc = bCrc;
    }

    return TRUE;
}

SnBool SetFlashOffset(SnQByte qOffset)
{
    if ((qOffset & 3) || qOffset >= FLASH_UPPER_LIMIT) {
        return FALSE;
    }

    g_qFlashOffset = qOffset;

    return TRUE;
}

//
// Store Data Layout
//
// qMaxStoreSize
// qCurStoreSize
// pbStoreData[qCurStoreSize] 
// bCRC
//
// qCurStoreSize <= (qMaxStoreSize - (sizeof(SnQByte) * 2) - sizeof(SnByte))
//

typedef struct {
    SnQByte qMaxStoreSize;
    SnQByte qCurStoreSize;
} SnStoreHeader;

#define STORE_SIZE_EXTRA    (sizeof(SnStoreHeader) + sizeof(SnByte))

_inline SnBool FlashStoreIsErased(SnAddr aFlashStoreAddr, SnQByte qMaxStoreSize)
{
    volatile SnQByte *pqFlash = (SnQByte *)(g_aFlashBase + aFlashStoreAddr);
    SnQByte qWords = qMaxStoreSize >> 2;

    do {
        if (*pqFlash++ != 0xffffffff) {
            return FALSE;
        }
    } while (--qWords > 0);

    return TRUE;
}

SnBool FlashStoreIsValid(SnAddr aFlashStoreAddr, SnStoreHeader *ptCurStore)
{
    volatile SnQByte *pqFlash = (SnQByte *)(g_aFlashBase + aFlashStoreAddr);
    SnQByte qWords;
    SnQByte qData;
    SnByte bCrc = 0;

    ptCurStore->qMaxStoreSize = *pqFlash;
    ptCurStore->qCurStoreSize = *(pqFlash + 1);

    // Validate the Store Header
    if (ptCurStore->qMaxStoreSize > FLASH_PAGE_SIZE ||
        ptCurStore->qCurStoreSize > FLASH_PAGE_SIZE)
        return FALSE;

    qWords = ptCurStore->qMaxStoreSize >> 2;

    do {
        qData = *pqFlash++;
        bCrc = g_pbCrcTable[bCrc ^ (qData & 0xFF)];
        bCrc = g_pbCrcTable[bCrc ^ ((qData >> 8) & 0xFF)];
        bCrc = g_pbCrcTable[bCrc ^ ((qData >> 16) & 0xFF)];
        bCrc = g_pbCrcTable[bCrc ^ (qData >> 24)];
    } while (--qWords);

    // Validate the Crc of the entire flash store
    if (bCrc != 0)
        return FALSE;

   return TRUE;
}

SnBool ReadFlashStore(SnFlashStore *ptFlashStore,
                               SnByte *pbLastStore, SnQByte *pqLastStoreSize)
{
    SnStoreHeader tCurStore;
    SnAddr aLastFlashAddr;
    SnQByte qCurSize, qMaxStoreSize;

    NKDbgPrintfW(TEXT("ReadFlashStore()\n"));

    // Adjust MaxStoreSize to include extra space for sizes and CRC.
    // Also round it up to the nearest 64 byte size.
    qMaxStoreSize = (ptFlashStore->qMaxStoreBufSize + STORE_SIZE_EXTRA + 0x3f) & ~0x3f;

    // Reject any invalid paramters
    if (ptFlashStore->aFlashStoreTop <= ptFlashStore->aFlashStoreBase ||
        ptFlashStore->aFlashStoreTop > FLASH_UPPER_LIMIT ||
       !IS_FLASH_PAGE_ALIGNED(ptFlashStore->aFlashStoreBase) ||
       !IS_FLASH_PAGE_ALIGNED(ptFlashStore->aFlashStoreTop) ||
       qMaxStoreSize > FLASH_PAGE_SIZE) {
        return FALSE;
    }

    qCurSize = 0;
    g_qStoreMaxSize = qMaxStoreSize;
    g_aStoreFlashBase = ptFlashStore->aFlashStoreBase;
    g_aStoreFlashTop = ptFlashStore->aFlashStoreTop;
    g_aStoreFlashAddr = aLastFlashAddr = ptFlashStore->aFlashStoreBase;
    if (g_pbStoreBuf) {
        free(g_pbStoreBuf);
    }
    g_pbStoreBuf = (SnByte *)malloc(qMaxStoreSize);
    if (g_pbStoreBuf == NULL) {
        return FALSE;
    }
    memset(g_pbStoreBuf, 0xff, qMaxStoreSize);

    // Scan through the Stores to ptFlashStore->aFlashStoreTop the last entry
    while (g_aStoreFlashAddr < ptFlashStore->aFlashStoreTop) {

        // Check for erased flash Store
        if (FlashStoreIsErased(g_aStoreFlashAddr, qMaxStoreSize)) {
            break;
        }

        // If Store entry is invalid, erase the whole flash.
        if (FlashStoreIsValid(g_aStoreFlashAddr, &tCurStore) == FALSE) {
            EraseFlashStore();
            qCurSize = 0;
            break;
        }

        qCurSize = tCurStore.qCurStoreSize;
        aLastFlashAddr = g_aStoreFlashAddr;
        g_aStoreFlashAddr += tCurStore.qMaxStoreSize;
    }

    if (pqLastStoreSize) {
        *pqLastStoreSize = qCurSize;
    }

    if (pbLastStore && qCurSize > 0 && qCurSize <= ptFlashStore->qMaxStoreBufSize) {
        g_qFlashOffset = aLastFlashAddr + sizeof(SnStoreHeader);
        return ReadFlashData((SnQByte *)pbLastStore, qCurSize);
    }

    return TRUE;
}

SnBool WriteFlashStore(SnByte *pbStoreData, SnQByte qStoreSize)
{
    SnStoreHeader *ptCurStore = (SnStoreHeader *)g_pbStoreBuf;
    SnQByte qAdjustedStoreSize;
    SnBool yStatus;

    // Adjust StoreSize to include extra space for sizes and CRC.
    // Also round it up to the nearest 64 byte size.
    qAdjustedStoreSize = (qStoreSize + STORE_SIZE_EXTRA + 0x3f) & ~0x3f;

    if (g_aStoreFlashAddr == 0 || qAdjustedStoreSize > g_qStoreMaxSize) {
        return FALSE;
    }

    if ((g_aStoreFlashAddr + g_qStoreMaxSize) >= g_aStoreFlashTop) {
        SnQByte qFlashPages = (g_aStoreFlashTop - g_aStoreFlashBase) / FLASH_PAGE_SIZE;

        g_qFlashOffset = g_aStoreFlashBase;
        if (EraseFlashPages(qFlashPages) == FALSE) {
            return FALSE;
        }
        g_aStoreFlashAddr = g_aStoreFlashBase;
    }

    
    ptCurStore->qMaxStoreSize = g_qStoreMaxSize;
    ptCurStore->qCurStoreSize = qStoreSize;
    memcpy(g_pbStoreBuf + (sizeof(SnQByte) * 2), pbStoreData, qStoreSize);

    // Caluculate Crc of Store
    g_pbStoreBuf[qAdjustedStoreSize - 1] = 0;
    CrcBufData(g_pbStoreBuf, qAdjustedStoreSize - 1, &g_pbStoreBuf[qAdjustedStoreSize - 1]);

    g_qFlashOffset = g_aStoreFlashAddr;
    yStatus = WriteFlashData((SnQByte *)g_pbStoreBuf, qAdjustedStoreSize);
    g_aStoreFlashAddr += g_qStoreMaxSize;

    return yStatus;
}

SnBool EraseFlashStore(void)
{
    SnQByte qFlashPages;

    NKDbgPrintfW(TEXT("EraseFlashStore()\n"));

    if (g_aStoreFlashBase == 0) {
        return FALSE;
    }
    
    qFlashPages = (g_aStoreFlashTop - g_aStoreFlashBase) / FLASH_PAGE_SIZE;
    g_qFlashOffset = g_aStoreFlashAddr = g_aStoreFlashBase;

    if (EraseFlashPages(qFlashPages) == FALSE) {
        return FALSE;
    }

    return TRUE;
}

SnBool InitFlash(void)
{
    g_aFlashBase = (SnAddr)g_pxFlashMap;
    g_aFlashEnd = g_aFlashBase + FLASH_UPPER_LIMIT;

    g_qStoreMaxSize = 0;
    g_aStoreFlashBase = 0;
    g_aStoreFlashTop = 0;
    g_aStoreFlashAddr = 0;
    g_pbStoreBuf = 0;

    return TRUE;
}
