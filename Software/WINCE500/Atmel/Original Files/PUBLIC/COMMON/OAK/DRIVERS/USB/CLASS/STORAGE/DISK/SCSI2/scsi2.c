//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

  scsi2.c

Abstract:

   Scsi2 Interface for USB Disk Class 

   bInterfaceSubClass = 0x02 : SFF8020i (ATAPI) CD-ROM
   bInterfaceSubClass = 0x04 : USB Floppy Interface (UFI)
   bInterfaceSubClass = 0x06 : SCSI Passthrough

   This code basically builds a SCSI2 Command Descriptor Block (CDB) for 
   transport via the USB Mass Storage Class.

--*/

#include <ntcompat.h>
#include <pkfuncs.h>

#include "usbmsc.h"
#include "scsi2.h"

//*****************************************************************************
//      P R I V A T E
//*****************************************************************************
static
DWORD
InspectSgReq(
    PSG_REQ pSgReq,
    UINT uiBytesPerSector
    );

//*****************************************************************************
//      S C S I - 2     I N T E R F A C E
//*****************************************************************************
DWORD
ScsiPassthrough(
    PSCSI_DEVICE       pDevice,
    PTRANSPORT_COMMAND pCommand,
    PTRANSPORT_DATA    pData
    )
{
    DWORD dwErr;
    
    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6>ScsiPassthrough\n")));

    do {

        if ( !pDevice || !pCommand || !pData ) {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL);
        if ( ERROR_SUCCESS != dwErr) {
            break;
        }

        __try {
            pCommand->dwLun=pDevice->Lun;
            dwErr = UsbsDataTransfer( pDevice->hUsbTransport,
                                      pCommand,
                                      pData );

        } __except ( EXCEPTION_EXECUTE_HANDLER ) {
            //
            // This could happen if the caller has embedded pointers in the 
            // struct. In these cases the caller should use IOCTL_DISK_Xxx
            // which knows about the contents of the command buffers, and 
            // correctly maps them in. Otherwise, a filter driver would be 
            // responsible for knowing about the command buffers and do the 
            // correct mapping.
            //
            dwErr = GetExceptionCode();
            DEBUGMSG(ZONE_ERR,(TEXT("USBDISK6::ScsiPassthrough:EXCEPTION:0x%x\n")
                , dwErr));
        }
        
        ReleaseRemoveLock(&pDevice->RemoveLock, NULL);

    } while (0);
    
    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6<ScsiPassthrough:%d\n"), dwErr));

    SetLastError(dwErr);

    return dwErr;
}


DWORD
ScsiUnitAttention(
    PSCSI_DEVICE    pDevice,
    UCHAR           Lun
    )
{
    DWORD dwRepeat, dwErr, dwStartErr, dwSenseErr;

    DEBUGMSG(ZONE_TRACE,(TEXT("Usbdisk6!ScsiUnitAttention++\r\n")));

    dwRepeat = pDevice->Timeouts.UnitAttnRepeat;

    do {
#if 0
        if (pDevice->DiskSubClass == USBMSC_SUBCLASS_UFI) {
            __asm int 3;
            // UFI floppies seem to require start unit first
            dwStartErr = ScsiStartStopUnit(pDevice, TRUE,  FALSE, Lun);
            if (ERROR_ACCESS_DENIED == dwStartErr || ERROR_INVALID_HANDLE == dwStartErr)
                return dwStartErr;
        }
#endif 0

        dwErr = ScsiTestUnitReady(pDevice, Lun);
        if (dwErr == ERROR_GEN_FAILURE) {
            // break;
            ;
        }

        if (ERROR_ACCESS_DENIED == dwErr || ERROR_INVALID_HANDLE == dwErr) {
            // break;
            ;
        }
        else if (ERROR_SUCCESS != dwErr) {

            DEBUGMSG(ZONE_ERR,(TEXT("Usbdisk6!ScsiUnitAttention> TEST UNIT READY failed(%d)"), dwErr));

            // some devices require a (re)start
            dwStartErr = ScsiStartStopUnit(pDevice, TRUE,  FALSE, Lun);
            if (ERROR_ACCESS_DENIED == dwStartErr || ERROR_INVALID_HANDLE == dwStartErr) {
                dwErr = dwStartErr;
                // break;
                ;
            }
            else if (ERROR_SUCCESS != dwStartErr) {
                // clear error
                dwSenseErr = ScsiGetSenseData(pDevice, Lun);
                if (ERROR_ACCESS_DENIED == dwSenseErr || ERROR_INVALID_HANDLE == dwSenseErr) {
                    dwErr = dwSenseErr;
                    // break;
                    ;
                }
            }

            Sleep(10);

        }

    } while (ERROR_SUCCESS != dwErr && --dwRepeat != 0);

    DEBUGMSG(ZONE_TRACE,(TEXT("Usbdisk6!ScsiUnitAttention-- Error(%d)\r\n"), dwErr));

    return dwErr;
}


typedef struct _USBMSC_DEVICE {
    ULONG Sig;
    CRITICAL_SECTION Lock;
} USBMSC_DEVICE;



DWORD
ScsiTestUnitReady(
    PSCSI_DEVICE    pDevice,
    UCHAR           Lun
    )
{
    USBMSC_DEVICE *pUsbDevice = (USBMSC_DEVICE *)pDevice->hUsbTransport;   
    TRANSPORT_COMMAND tCommand;
    UCHAR             bCDB[MAX_CDB];
    
    DWORD dwErr;
    
    DEBUGMSG(ZONE_TRACE,(TEXT("USBDISK6>ScsiTestUnitReady\n")));

    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL);
    if ( ERROR_SUCCESS != dwErr) {
        return dwErr;
    }

    tCommand.Flags   = DATA_OUT;
    tCommand.Timeout = pDevice->Timeouts.ScsiCommandTimeout;
    tCommand.Length  = USBMSC_SUBCLASS_SCSI == pDevice->DiskSubClass ? 
                       SCSI_CDB_6 : UFI_CDB;
    tCommand.CommandBlock = bCDB;
    tCommand.dwLun=Lun;

    memset(bCDB, 0, sizeof(bCDB));
    bCDB[0] = SCSI_TEST_UNIT_READY;

    ASSERT(Lun <= 0x7);
    bCDB[1] = ((Lun & 0x7) << 5);

    EnterCriticalSection(&pUsbDevice->Lock);
    dwErr = UsbsDataTransfer( pDevice->hUsbTransport,
                              &tCommand,
                              NULL );

    if ( dwErr != ERROR_SUCCESS ) {

        dwErr = ScsiGetSenseData( pDevice, Lun );

        DEBUGMSG(ZONE_ERR,(TEXT("ScsiTestUnitReady ERROR:%d\n"), dwErr));

        SetLastError(dwErr);

    }
    LeaveCriticalSection(&pUsbDevice->Lock);
    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
    
    DEBUGMSG(ZONE_TRACE,(TEXT("USBDISK6<ScsiTestUnitReady:%d\n"), dwErr));

    return dwErr;
}


DWORD
ScsiRequestSense(
    PSCSI_DEVICE    pDevice,
    PTRANSPORT_DATA pTData,
    UCHAR           Lun
    )
{
    TRANSPORT_COMMAND tCommand;
    UCHAR bCDB[MAX_CDB];
    DWORD dwErr;

    DEBUGMSG(ZONE_TRACE,(TEXT("USBDISK6>ScsiRequestSense\n")));

    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL) ;
    if ( ERROR_SUCCESS != dwErr) {
        return dwErr;
    }

    tCommand.Flags   = DATA_IN;
    tCommand.Timeout = pDevice->Timeouts.ScsiCommandTimeout;
    tCommand.Length  = USBMSC_SUBCLASS_SCSI == pDevice->DiskSubClass ? 
                       SCSI_CDB_6 : UFI_CDB;
    tCommand.CommandBlock = bCDB;
    tCommand.dwLun=Lun;

    memset(bCDB,0,sizeof(bCDB));
    bCDB[0] = SCSI_REQUEST_SENSE;
    ASSERT(Lun <= 0x7);
    bCDB[1] = ((Lun & 0x7) << 5);
    
    bCDB[4] = (UCHAR)pTData->RequestLength;

    dwErr = UsbsDataTransfer( pDevice->hUsbTransport,
                             &tCommand,
                             pTData );

    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);

    DEBUGMSG(ZONE_TRACE,(TEXT("USBDISK6<ScsiRequestSense:%d\n"), dwErr));

    return dwErr;
}


//
// Translates SCSI device error to Win32 error.
//
DWORD 
ScsiGetSenseData(
    PSCSI_DEVICE pDevice,
    UCHAR        Lun
    )
{
    TRANSPORT_DATA tData;
    UCHAR senseData[18];
    DWORD dwErr;
    UCHAR SenseKey, ASC, ASCQ;

    DEBUGMSG(ZONE_TRACE,(TEXT("USBDISK6>ScsiGetSenseData\n")));

    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL) ;
    if ( ERROR_SUCCESS != dwErr) {
        return dwErr;
    }

    tData.TransferLength = 0;
    tData.RequestLength = sizeof(senseData);
    tData.DataBlock = senseData;
    memset(senseData,0,sizeof(senseData));

    dwErr = ScsiRequestSense(pDevice, &tData, Lun);

    // did we receive a valid SenseKey
    if ( ERROR_SUCCESS == dwErr &&
         (0x70 & senseData[0]) || (0x71 & senseData[0]) && 
         tData.TransferLength >= 13 ) {
         
        // ASSERT( senseData[0] & 0x80 );  // Valid bit

        SenseKey = senseData[2] & 0x0f;
        ASC      = senseData[12];
        ASCQ     = senseData[13];
        
        DEBUGMSG(ZONE_WARN,(TEXT("ScsiGetSenseData - SenseKey:0x%x ASC:0x%x ASCQ:0x%x\n")
                            , SenseKey, ASC, ASCQ));

        EnterCriticalSection(&pDevice->Lock);

        switch (SenseKey) {

            case SENSE_NONE :
            case SENSE_RECOVERED_ERROR :
                dwErr = ERROR_SUCCESS;
                break;

            case SENSE_NOT_READY :
                switch( ASC ) {
                    case ASC_LUN:
                        DEBUGMSG(ZONE_WARN,(TEXT("SENSE_NOT_READY : ASC_LUN: %d\n"),
                             Lun));
                        if (0x02 == ASCQ) { // Initialization Required
                            TEST_TRAP();
                            ScsiStartStopUnit(pDevice, START, FALSE, pDevice->Lun);
                        }
                        dwErr = ERROR_NOT_READY;
                        break;

                    case ASC_MEDIUM_NOT_PRESENT :
                        DEBUGMSG(ZONE_WARN,(TEXT("SENSE_NOT_READY : ASC_MEDIUM_NOT_PRESENT\n")));
                        ASSERT(!ASCQ);

                        pDevice->Flags.MediumPresent = FALSE;
                        pDevice->Flags.MediumChanged = TRUE;
                        pDevice->MediumType = SCSI_MEDIUM_UNKNOWN;

                        memset(&pDevice->DiskInfo, 0, sizeof(DISK_INFO) );
                    
                        dwErr = DISK_REMOVED_ERROR;
                        break;

                    default:
                        DEBUGMSG(ZONE_WARN,(TEXT("SENSE_NOT_READY : Unhandled ASC:0x%x ASCQ:0x%x\n"),
                            ASC, ASCQ));
                        dwErr = ERROR_NOT_READY;
                        break;
                }
                break;

            case SENSE_MEDIUM_ERROR :
                DEBUGMSG(ZONE_WARN,(TEXT("SENSE_MEDIUM_ERROR\n")));
                dwErr = ERROR_UNRECOGNIZED_MEDIA;
                //TEST_TRAP();
                break;

            case SENSE_HARDWARE_ERROR :
                DEBUGMSG(ZONE_WARN,(TEXT("SENSE_HARDWARE_ERROR\n")));
                dwErr = ERROR_DISK_OPERATION_FAILED;
                TEST_TRAP();
                break;

            case SENSE_ILLEGAL_REQUEST :
                DEBUGMSG(ZONE_WARN,(TEXT("SENSE_ILLEGAL_REQUEST\n")));
                dwErr = ERROR_INVALID_PARAMETER;
                // TEST_TRAP();
                break;

            case SENSE_UNIT_ATTENTION :
                switch( ASC ) {
                    case ASC_MEDIA_CHANGED :
                        DEBUGMSG(ZONE_WARN,(TEXT("SENSE_UNIT_ATTENTION : ASC_MEDIA_CHANGED\n")));
                        ASSERT(!ASCQ);
                        
                        pDevice->Flags.MediumPresent = !pDevice->Flags.MediumPresent;
                        pDevice->Flags.MediumChanged = TRUE;
                        pDevice->MediumType = SCSI_MEDIUM_UNKNOWN;

                        memset(&pDevice->DiskInfo, 0, sizeof(DISK_INFO) );
                        break;

                    case ASC_RESET :
                        DEBUGMSG(ZONE_WARN,(TEXT("SENSE_UNIT_ATTENTION : ASC_RESET\n")));
                        break;

                    case ASC_COMMANDS_CLEARED :
                    default:
                        DEBUGMSG(ZONE_WARN,(TEXT("SENSE_UNIT_ATTENTION : Unhandled ASC:0x%x\n"), 
                            ASC));
                        break;
                }
                dwErr = DISK_REMOVED_ERROR;
                break;

            case SENSE_DATA_PROTECT :
                dwErr = ERROR_WRITE_PROTECT;
                break;

            default:
                dwErr = ERROR_FLOPPY_UNKNOWN_ERROR;
                break;
        }

        LeaveCriticalSection(&pDevice->Lock);
    
    } else {
        DEBUGMSG(ZONE_ERR,(TEXT("ScsiGetSenseData error:%d\n"), dwErr));
    }

    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);

    DEBUGMSG(ZONE_TRACE,(TEXT("USBDISK6<ScsiGetSenseData:%d\n"), dwErr));

    return dwErr;
}


// returns Win32 error
DWORD 
ScsiInquiry(
    PSCSI_DEVICE    pDevice,
    UCHAR           Lun
    )
{
    TRANSPORT_COMMAND tCommand;
    UCHAR             bCDB[MAX_CDB];

    TRANSPORT_DATA    tData = {0};
    UCHAR             bDataBlock[36]; // Standard Inquiry Data
#ifdef DEBUG
    DWORD          dwStatus;
    ANSI_STRING    asString;
    UNICODE_STRING usString = {0, 0, 0};
    WCHAR          wcBuff[128]; // excessively large for the unexcected
#endif    
    DWORD dwErr = ERROR_SUCCESS;;
    
    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6>ScsiInquiry:Lun:%d\n"), Lun));

    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL) ;
    if ( ERROR_SUCCESS != dwErr) {
        return dwErr;
    }

    tCommand.Flags   = DATA_IN;
    tCommand.Timeout = pDevice->Timeouts.ScsiCommandTimeout;
    tCommand.Length  = USBMSC_SUBCLASS_SCSI == pDevice->DiskSubClass ? 
                        SCSI_CDB_6 : UFI_CDB;
    tCommand.CommandBlock = bCDB;
    tCommand.dwLun=Lun;

    memset( bCDB, 0, sizeof(bCDB));
    bCDB[0] = SCSI_INQUIRY;
    
    ASSERT(Lun <= 0x7);
    bCDB[1] = ((Lun & 0x7) << 5);

    // EVPB is tbd
    // PageCode = 0
    bCDB[4] = sizeof(bDataBlock); // Allocation Length

    memset( bDataBlock, 0, sizeof(bDataBlock));
    tData.TransferLength = 0;
    tData.RequestLength = sizeof(bDataBlock);
    tData.DataBlock = bDataBlock;
    
    dwErr = UsbsDataTransfer(pDevice->hUsbTransport,
                             &tCommand,
                             &tData );

    if (dwErr != ERROR_SUCCESS || (tData.TransferLength != tData.RequestLength))
    {
        dwErr = ScsiGetSenseData( pDevice, Lun );

        DEBUGMSG(ZONE_ERR,(TEXT("ScsiInquiry ERROR:%d\n"), dwErr));

        SetLastError(dwErr);

    } else {

        CHAR  asBuff[17] = {0};
        UCHAR perq; // peripherial qualifier
        #define PERQ_LUN_CONNECTED      0x0
        #define PERQ_LUN_DISCONNECTED   0x1
        #define PERQ_LUN_INVALID        0x3

        DEBUGMSG(ZONE_SCSI,(TEXT("InquiryData@Lun:%d = 0x%x\n"), 
            Lun, bDataBlock[0]));

        if (bDataBlock[0] == 0x7F) {
            
            dwErr = ERROR_INVALID_PARAMETER; 
            TEST_TRAP();
        
        } else {

            EnterCriticalSection(&pDevice->Lock);

            pDevice->DeviceType = bDataBlock[0] & SCSI_DEVICE_UNKNOWN;
        
            perq = (bDataBlock[0] & 0xE0) >> 5;
            ASSERT(PERQ_LUN_INVALID != perq);

            // currently developing Direct Access & CD-ROM devices only
            ASSERT(SCSI_DEVICE_DIRECT_ACCESS == pDevice->DeviceType ||
                   SCSI_DEVICE_CDROM == pDevice->DeviceType);

            pDevice->Flags.RMB = (bDataBlock[1] & 0x80) >> 7;

    //        ASSERT(bDataBlock[3] & 0x01);   // Response Data Format

#ifdef DEBUG
            usString.Buffer = wcBuff;
            usString.MaximumLength = sizeof(wcBuff);

            //
            // VID
            //
            memcpy(asBuff, &bDataBlock[8], 8); asBuff[8] = 0;
            RtlInitAnsiString( &asString,  asBuff);
            dwStatus = RtlAnsiStringToUnicodeString(&usString, &asString, FALSE);
            if (STATUS_SUCCESS != dwStatus) { // NT status code
                DEBUGMSG(ZONE_ERR, (TEXT("RtlAnsiStringToUnicodeString error:0x%x\n"),
                    dwStatus));
                TEST_TRAP();
            } else {
                DEBUGMSG(ZONE_SCSI, (TEXT("VID: %s\n"), usString.Buffer ));
            }

            //
            // PID
            //
            memcpy(asBuff, &bDataBlock[16], 16); asBuff[16] = 0;
            RtlInitAnsiString( &asString, asBuff );
            dwStatus = RtlAnsiStringToUnicodeString( &usString, &asString, FALSE );
            if (STATUS_SUCCESS != dwStatus) { // NT status code
                DEBUGMSG(ZONE_ERR, (TEXT("RtlAnsiStringToUnicodeString error:0x%x\n"),
                dwStatus));
                TEST_TRAP();
            } else {
                DEBUGMSG(ZONE_SCSI, (TEXT("PID: %s\n"), usString.Buffer ));
            }

            //
            // PRL
            //
            memcpy(asBuff, &bDataBlock[32], 4); asBuff[4] = 0;
            RtlInitAnsiString( &asString, asBuff );
            dwStatus = RtlAnsiStringToUnicodeString(&usString, &asString, FALSE);
            if (STATUS_SUCCESS != dwStatus) { // NT status code
                DEBUGMSG(ZONE_ERR, (TEXT("RtlAnsiStringToUnicodeString error:0x%x\n"),
                    dwStatus));
                TEST_TRAP();
            } else {
                DEBUGMSG(ZONE_SCSI, (TEXT("PRL: %s\n"), usString.Buffer ));
            }
#endif DEBUG
    
            LeaveCriticalSection(&pDevice->Lock);
        }
    }

    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
    
    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6<ScsiInquiry:%d\n"), dwErr));
    
    return dwErr;
}


DWORD
ScsiSendDiagnostic(
    PSCSI_DEVICE    pDevice,
    UCHAR           Lun
    )
{
    TRANSPORT_COMMAND tCommand;
    UCHAR             bCDB[MAX_CDB];
    
    DWORD dwErr;
    
    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6>ScsiSendDiagnostic\n")));

    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL) ;
    if ( ERROR_SUCCESS != dwErr) {
        return dwErr;
    }

    tCommand.Flags   = DATA_IN;
    tCommand.Timeout = pDevice->Timeouts.ScsiCommandTimeout;
    tCommand.Length  = USBMSC_SUBCLASS_SCSI == pDevice->DiskSubClass ?
                       SCSI_CDB_6 : UFI_CDB;
    tCommand.CommandBlock = bCDB;
    tCommand.dwLun=Lun;
    
    memset( bCDB, 0, sizeof(bCDB));
    bCDB[0] = SCSI_SEND_DIAGNOSTIC;

    ASSERT(Lun <= 0x7);
    bCDB[1] = ((Lun & 0x7) << 5);

    bCDB[1] |= 0x4; // SelfTest

    dwErr = UsbsDataTransfer(pDevice->hUsbTransport,
                             &tCommand,
                             NULL );

    if ( dwErr != ERROR_SUCCESS ) {

        dwErr = ScsiGetSenseData( pDevice, Lun );

        DEBUGMSG(ZONE_ERR,(TEXT("ScsiSendDiagnostic ERROR:%d\n"), dwErr));

        SetLastError(dwErr);

    }

    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
    
    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6<ScsiSendDiagnostic:%d\n"), dwErr));

    return dwErr;
}


// returns Win32 error
DWORD 
ScsiReadCapacity(
    PSCSI_DEVICE pDevice,
    PDISK_INFO   pDiskInfo,
    UCHAR        Lun
    )
{
    TRANSPORT_COMMAND tCommand;
    UCHAR             bCDB[MAX_CDB];

    TRANSPORT_DATA tData;
    UCHAR          bDataBlock[8];
    DWORD dwSizeDB = sizeof(bDataBlock);

    DWORD dwSizeDI = sizeof(DISK_INFO);
    DWORD dwErr    = ERROR_SUCCESS;
    
    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6>ScsiReadCapacity\n")));

    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL) ;
    if ( ERROR_SUCCESS != dwErr) {
        return dwErr;
    }

    memset(pDiskInfo, 0, dwSizeDI);

    tCommand.Flags   = DATA_IN;
    tCommand.Timeout = pDevice->Timeouts.ScsiCommandTimeout;
    tCommand.Length  = USBMSC_SUBCLASS_SCSI == pDevice->DiskSubClass ?
                       SCSI_CDB_10 : UFI_CDB;
    tCommand.CommandBlock = bCDB;
    tCommand.dwLun=Lun;

    memset( bCDB, 0, sizeof(bCDB));
    bCDB[0] = SCSI_READ_CAPACITY;

    ASSERT(Lun <= 0x7);
    bCDB[1] = ((Lun & 0x7) << 5);

    // PMI & LBA support is TBD
    // It's useful in determining where longer access times occur

    memset( bDataBlock, 0, dwSizeDB);
    tData.TransferLength = 0;
    tData.RequestLength = dwSizeDB;
    tData.DataBlock = bDataBlock;

    dwErr = UsbsDataTransfer(pDevice->hUsbTransport,
                             &tCommand,
                             &tData );

    if ( dwErr != ERROR_SUCCESS ||
        (tData.TransferLength != tData.RequestLength) ) {

        dwErr = ScsiGetSenseData( pDevice, Lun );

        DEBUGMSG(ZONE_ERR,(TEXT("ScsiReadCapacity ERROR:%d\n"), dwErr));

        SetLastError(dwErr);

    } else {

        EnterCriticalSection(&pDevice->Lock);
        // Get total sectors from Last Logical Block Address
        pDiskInfo->di_total_sectors  = GetDWORD(&bDataBlock[0])+1; 
        // Block Length in bytes
        pDiskInfo->di_bytes_per_sect = GetDWORD(&bDataBlock[4]);
        pDiskInfo->di_flags         |= DISK_INFO_FLAG_PAGEABLE | DISK_INFO_FLAG_CHS_UNCERTAIN;

        if ( pDiskInfo->di_bytes_per_sect && pDiskInfo->di_total_sectors) {
            //
            // update our media info & flags
            //
            pDevice->Flags.MediumPresent = TRUE;

            if ( 0 != memcmp(&pDevice->DiskInfo, pDiskInfo, dwSizeDI) ) {

                memcpy(&pDevice->DiskInfo, pDiskInfo, dwSizeDI);

            }
        }

        LeaveCriticalSection(&pDevice->Lock);

        DUMP_DISK_INFO(pDiskInfo);
    }
    
    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);

    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6<ScsiReadCapacity:%d\n"), dwErr));
    
    return dwErr;
}


//
// Mode sense is dependant on Device Type.
// returns Win32 error. 
//
DWORD 
ScsiModeSense10(
    PSCSI_DEVICE    pDevice,
    UCHAR           Lun
    )
{
    TRANSPORT_DATA    tData;
    TRANSPORT_COMMAND tCommand;
    UCHAR             bCDB[MAX_CDB];
    #define MAX_LIST_LENGTH     512             // Note: USB Mass Storage Spec says this is max 72 bytes 
    UCHAR   bDataBlock[MAX_LIST_LENGTH] = {0};  // Standard Header + mode pages
    USHORT  usPageLength = 8;                   // Standard Header size

    DWORD dwErr = ERROR_SUCCESS;

    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6>ScsiModeSense\n")));

    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL) ;
    if ( ERROR_SUCCESS != dwErr) {
        return dwErr;
    }

    ASSERT( SCSI_DEVICE_DIRECT_ACCESS == pDevice->DeviceType || 
            SCSI_DEVICE_CDROM == pDevice->DeviceType );

    memset( bCDB, 0, sizeof(bCDB));

    tCommand.Flags   = DATA_IN;
    tCommand.Timeout = pDevice->Timeouts.ScsiCommandTimeout;
    tCommand.Length  = USBMSC_SUBCLASS_SCSI == pDevice->DiskSubClass ?
						SCSI_CDB_10 : UFI_CDB;
    tCommand.CommandBlock = bCDB;
    tCommand.dwLun=Lun;

    bCDB[0] = SCSI_MODE_SENSE10;

    ASSERT(Lun <= 0x7);
    bCDB[1] = ((Lun & 0x7) << 5);
    // bCDB[1] |= 0x8;      // DBD  Note: USB Mass Storage Spec says this is 0

    // PC = current
    // Page Code = ALL
    bCDB[2] = 0x3f;			// this was original set to 0 but 0 is not defined in the USB Mass Storage Spec 
							//		some devices return ILLEGAL REQUEST. So use some defines value as default

    switch (pDevice->DeviceType) 
    {
        case SCSI_DEVICE_DIRECT_ACCESS:
            if (pDevice->DiskSubClass == USBMSC_SUBCLASS_UFI) {
                bCDB[2] = MODE_PAGE_FLEXIBLE_DISK;
                usPageLength += 32;
            } else {
                usPageLength = sizeof(bDataBlock);
            }
            break;

        case SCSI_DEVICE_CDROM:
            bCDB[2] = MODE_PAGE_CDROM;
            usPageLength += 8;
            break;

        default:
            DEBUGMSG(ZONE_ERR,(TEXT("ScsiModeSense10: Unknown DeviceType:0x%x\n"),
                    pDevice->DeviceType));
            TEST_TRAP();
            usPageLength = sizeof(bDataBlock);
            break;
    }

    bCDB[7] = (sizeof(bDataBlock) & 0xFF00) >> 8; // MSB
    bCDB[8] =  sizeof(bDataBlock) & 0x00FF;       // LSB

    memset( bDataBlock, 0, sizeof(bDataBlock));
    ASSERT( usPageLength <= sizeof(bDataBlock));

    tData.TransferLength = 0;
    // Note: some devices fail with only 8 byte header request length, 
    // but all should recover with extra buffer space.
    tData.RequestLength = usPageLength;
    tData.DataBlock = bDataBlock;

    dwErr = UsbsDataTransfer( pDevice->hUsbTransport,
                             &tCommand,
                             &tData );

    if ( dwErr != ERROR_SUCCESS || tData.TransferLength < 8 ) { // want at least the header

        dwErr = ScsiGetSenseData( pDevice, Lun );
        
        SetLastError(dwErr);

        DEBUGMSG(ZONE_ERR,(TEXT("ScsiModeSense ERROR:%d\n"), dwErr));

    } else {
        
        EnterCriticalSection(&pDevice->Lock);

        // look at the Header
        DEBUGMSG(ZONE_SCSI,(TEXT("Medium Type:0x%x\n"), bDataBlock[2]));

        pDevice->MediumType = bDataBlock[2];

        // bit7 is WP bit
        DEBUGMSG(ZONE_SCSI,(TEXT("Device Specific:0x%x\n"), bDataBlock[3] ));
        pDevice->Flags.WriteProtect = bDataBlock[3] & 0x80;
        
        LeaveCriticalSection(&pDevice->Lock);
        
        //
        // TBD: look at the the requested page ...
        //
    }

    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
    
    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6<ScsiModeSense:%d\n"), dwErr));
    
    return dwErr;
}

DWORD
ScsiModeSense6(
    PSCSI_DEVICE pDevice,
    UCHAR        Lun
    )
{
    TRANSPORT_DATA    tData;
    TRANSPORT_COMMAND tCommand;
    UCHAR             bCDB[SCSI_CDB_6];
    UCHAR             bDataBlock[MAX_LIST_LENGTH] = {0}; // standard header + mode pages
    USHORT            usPageLength = 8;                  // standard header size
    DWORD             dwErr = ERROR_SUCCESS;

    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL);
    if (ERROR_SUCCESS != dwErr) {
        return dwErr;
    }

    ASSERT(SCSI_DEVICE_DIRECT_ACCESS == pDevice->DeviceType || SCSI_DEVICE_CDROM == pDevice->DeviceType);

    memset(bCDB, 0, sizeof(bCDB));
    tCommand.Flags = DATA_IN;
    tCommand.Timeout = pDevice->Timeouts.ScsiCommandTimeout;
    tCommand.Length = USBMSC_SUBCLASS_SCSI == pDevice->DiskSubClass ? SCSI_CDB_6 : UFI_CDB;
    tCommand.CommandBlock = bCDB;
    tCommand.dwLun = Lun;
    DEBUGCHK(Lun <= 0x7);

    bCDB[0] = SCSI_MODE_SENSE6;
    bCDB[1] = ((Lun & 0x7) << 5);
    bCDB[2] = 0x3f;

    switch (pDevice->DeviceType) {
        case SCSI_DEVICE_DIRECT_ACCESS:
            if (pDevice->DiskSubClass == USBMSC_SUBCLASS_UFI) {
                bCDB[2] = MODE_PAGE_FLEXIBLE_DISK;
                usPageLength += 32;
            }
            else {
                usPageLength = sizeof(bDataBlock);
            }
            break;
        case SCSI_DEVICE_CDROM:
            bCDB[2] = MODE_PAGE_CDROM;
            usPageLength += 8;
            break;
        default:
            usPageLength = sizeof(bDataBlock);
            break;
    }
    usPageLength = 8;
    DEBUGCHK(usPageLength <= sizeof(bDataBlock));

    memset(bDataBlock, 0, sizeof(bDataBlock));

    // a device may fail this command if the header request length is only 8 bytes;
    // if this is the case, then the device should recover with extra buffer space

    tData.TransferLength = 0;
    tData.RequestLength = usPageLength;
    tData.DataBlock = bDataBlock;

    dwErr = UsbsDataTransfer(pDevice->hUsbTransport, &tCommand, &tData);

    if (dwErr != ERROR_SUCCESS || tData.TransferLength < 8 ) { // we want at least the header
        dwErr = ScsiGetSenseData(pDevice, Lun);
        SetLastError(dwErr);
        DEBUGMSG(ZONE_ERR,(TEXT("ScsiModeSense6: device failed command\r\n"), dwErr));
    }
    else {
        EnterCriticalSection(&pDevice->Lock);
        DEBUGMSG(ZONE_SCSI,(TEXT("Scsi2ModeSense6: medium type=0x%x\n"), bDataBlock[2]));
        pDevice->MediumType = bDataBlock[2];
        pDevice->Flags.WriteProtect = bDataBlock[3] & 0x80; // inspect WP bit
        LeaveCriticalSection(&pDevice->Lock);
    }

    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
    return dwErr;
}

DWORD
ScsiStartStopUnit(
    PSCSI_DEVICE    pDevice,
    BOOL            Start,  // TRUE = START, else STOP
    BOOL            LoEj,
    UCHAR           Lun
    )
{
    TRANSPORT_COMMAND tCommand;
    UCHAR             bCDB[MAX_CDB];
    
    DWORD dwErr;
    
    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6>ScsiStartStopUnit\n")));

    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL);
    if ( ERROR_SUCCESS != dwErr) {
        return dwErr;
    }

    tCommand.Flags   = DATA_OUT;
    tCommand.Timeout = pDevice->Timeouts.ScsiCommandTimeout;
    tCommand.Length  = USBMSC_SUBCLASS_SCSI == pDevice->DiskSubClass ?
                       SCSI_CDB_6 : UFI_CDB;
    tCommand.CommandBlock = bCDB;
    tCommand.dwLun=Lun;

    memset( bCDB, 0, sizeof(bCDB));
    bCDB[0] = SCSI_START_STOP;

    ASSERT(Lun <= 0x7);
    bCDB[1] = ((Lun & 0x7) << 5);

    bCDB[4] = (LoEj & 0x1) << 1;
    bCDB[4] |= Start & 0x1;

    dwErr = UsbsDataTransfer(pDevice->hUsbTransport,
                             &tCommand,
                             NULL );

    if ( dwErr != ERROR_SUCCESS ) {

        dwErr = ScsiGetSenseData( pDevice, Lun );

        DEBUGMSG(ZONE_ERR,(TEXT("ScsiStartStopUnit ERROR:%d\n"), dwErr));

        SetLastError(dwErr);

    }

    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
    
    DEBUGMSG(ZONE_SCSI,(TEXT("USBDISK6<ScsiStartStopUnit:%d\n"), dwErr));

    return dwErr;
}


//
// Checks the users SG to see if we can use it.
// Since we basically do a prescan then return flag saying
// if we need to use double buffering.
//
DWORD
CheckSegments(
    IN PSCSI_DEVICE    pDevice,
    IN PSG_REQ         pSgReq,
    IN OUT PDWORD      pTransferLength,
    IN OUT PDWORD      pFlags
    )
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD sg;

    DEBUGMSG(ZONE_TRACE,(TEXT("USBMSC>CheckSegments\n")));


    if ( !pDevice || 0 == pDevice->DiskInfo.di_bytes_per_sect || 
         0 == pDevice->DiskInfo.di_total_sectors ) 
    {
        dwErr = ERROR_FLOPPY_UNKNOWN_ERROR;
        DEBUGMSG(ZONE_ERR,(TEXT("CheckSegments ERROR:1: di_bytes_per_sect:%d di_total_sectors:%d\n"),
            pDevice?pDevice->DiskInfo.di_bytes_per_sect:-1, pDevice?pDevice->DiskInfo.di_total_sectors:-1));
        
        TEST_TRAP();

    } else if ( !pSgReq || !pTransferLength || !pFlags) {

        dwErr=ERROR_INVALID_PARAMETER;
        DEBUGMSG(ZONE_ERR,(TEXT("CheckSegments ERROR:2: %d\n"), dwErr ));
    
    } else if (pSgReq->sr_num_sg > MAX_SG_BUF ||
               pSgReq->sr_start + pSgReq->sr_num_sec > pDevice->DiskInfo.di_total_sectors)
    {
            dwErr=ERROR_INVALID_PARAMETER;
            DEBUGMSG(ZONE_ERR,(TEXT("CheckSegments ERROR:3: sg:%d sr_start:%d, sr_num_sec:%d, di_total_sectors:%d di_bytes_per_sect:%d\n"),
                pSgReq->sr_num_sg, pSgReq->sr_start, pSgReq->sr_num_sec, pDevice->DiskInfo.di_total_sectors, pDevice->DiskInfo.di_bytes_per_sect));
    
    } else {
        //
        // ensure all the buffers in the SG list are OK
        //
        ASSERT(pFlags);
        ASSERT(pTransferLength);
	    EnterCriticalSection(&pDevice->Lock);

        for (sg = 0, *pTransferLength = 0, *pFlags = 0; sg < pSgReq->sr_num_sg; sg++) 
        {
            if ( !pSgReq->sr_sglist[sg].sb_buf )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                DEBUGMSG(ZONE_ERR,(TEXT("CheckSegments ERROR:4:%d\n"), dwErr));
                break;
            }

            // do we need double buffering?
            if ((pSgReq->sr_sglist[sg].sb_len % pDevice->DiskInfo.di_bytes_per_sect) != 0)
            {
                *pFlags |= 0x1;
                DEBUGMSG(ZONE_READ,(TEXT("CheckSegments: Double Buffered\n")));
            }

            // sum all the buffer lengths to make sure they match 
            // the requested number of sectors
            *pTransferLength += pSgReq->sr_sglist[sg].sb_len;
        }

        if (dwErr == ERROR_SUCCESS && 
            *pTransferLength > (pSgReq->sr_num_sec * pDevice->DiskInfo.di_bytes_per_sect)) 
        {
            dwErr = ERROR_INVALID_PARAMETER;
            DEBUGMSG(ZONE_ERR,(TEXT("CheckSegments ERROR:5: invalid user SG buffers (%u > %u)\n"),
            *pTransferLength, (pSgReq->sr_num_sg * pDevice->DiskInfo.di_bytes_per_sect)));
        }
               
	    LeaveCriticalSection(&pDevice->Lock);
    }

    DEBUGMSG(ZONE_TRACE,(TEXT("USBMSC<CheckSegments:%d\n"), dwErr));

    return dwErr;
}

/*++

    Validate a Scatter/Gather request object.

    Returns: If fail, Win32 error.
             If pass, ERROR_SUCCESS.
--*/
static DWORD
InspectSgReq(
    PSG_REQ pSgReq,
    UINT uiBytesPerSector
    )
{
    UINT uiSgIndex;         // Index into Scatter/Gather buffer array.
	UINT uiBytesToTransfer; // Running count of the number of bytes requested to transfer.
	
	DEBUGMSG(ZONE_TRACE, (TEXT("USBMSC>InspectSgReq\n")));

	if (pSgReq == NULL) {
		DEBUGMSG(ZONE_ERR, (TEXT("USBMSC>InspectSgReq: Error: pSqReq: NULL\n")));
		return ERROR_BAD_ARGUMENTS;
    }    
    if (pSgReq->sr_num_sec == 0) {
		DEBUGMSG(ZONE_ERR, (TEXT("USBMSC>InspectSgReq: Error: ->sr_num_sec: 0\n")));
		return ERROR_BAD_ARGUMENTS;
	}
	if (pSgReq->sr_num_sg == 0) {
		DEBUGMSG(ZONE_ERR, (TEXT("USBMSC>InspectSgReq: Error: ->sr_num_sg: 0\n")));
		return ERROR_BAD_ARGUMENTS;
	}
	DEBUGMSG(ZONE_TRACE, (TEXT("USBMSC>InspectSgReq: ->sr_num_sec: %d, ->sr_num_sg: %d, bytes/sec: %d\n"), pSgReq->sr_num_sec, pSgReq->sr_num_sg, uiBytesPerSector));
	for (uiSgIndex = 0, uiBytesToTransfer = 0; uiSgIndex < pSgReq->sr_num_sg; uiSgIndex += 1) {
		if (pSgReq->sr_sglist[uiSgIndex].sb_buf == NULL) {
			DEBUGMSG(ZONE_ERR, (TEXT("USBMSC>InspectSgReq: Error: ->sr_sglist[%d].sb_buf: NULL\n"), uiSgIndex));
			return ERROR_BAD_ARGUMENTS;
		}
		if (pSgReq->sr_sglist[uiSgIndex].sb_len == 0) {
			DEBUGMSG(ZONE_ERR, (TEXT("USBMSC>InspectSgReq: Error: ->sr_sglist[%d].sb_len: 0\n"), uiSgIndex));
			return ERROR_BAD_ARGUMENTS;
		}
		DEBUGMSG(ZONE_TRACE, (TEXT("USBMSC>InspectSgReq: ->sr_sglist[%d].sb_len: %d\n"), uiSgIndex, pSgReq->sr_sglist[uiSgIndex].sb_len));
		uiBytesToTransfer += pSgReq->sr_sglist[uiSgIndex].sb_len;
    }
	if (uiBytesPerSector==0 || ((uiBytesToTransfer / uiBytesPerSector) != pSgReq->sr_num_sec)) {
		DEBUGMSG(ZONE_ERR, (TEXT("USBMSC>InspectSgReq: Error: [(tot bytes = %d)*(bytes/sec = %d)] != (->sr_num_sec = %d)\n"), uiBytesToTransfer, uiBytesPerSector, pSgReq->sr_num_sec));
		return ERROR_BAD_ARGUMENTS;
	}
	DEBUGMSG(ZONE_TRACE,(TEXT("USBMSC<InspectSgReq\n")));	
	return ERROR_SUCCESS;
}

/*++

    Complete a Scatter/Gather request on a SCSI device.

    Returns: If fail, Win32 error.
             If pass, total number of bytes transferred.
                 Completion status returned in pSqReq->sr_status.    
--*/
DWORD
ScsiRWSG(
    PSCSI_DEVICE pDevice,
    PSG_REQ pSgReq,
    UCHAR Lun,
    BOOL bRead
    )
{
    UINT uiExpectedBytesXferred = 0; // Expected total size of transfer (bytes).
    UINT uiActualBytesXferred = 0;   // Actual total size of transfer (bytes).
    DWORD dwFlags = 0;               // Flags indicating need for double buffering.
    DWORD dwErr;                     // Error code.

    DEBUGMSG(ZONE_TRACE,(TEXT("USBMSC>ScsiRWSG\n")));
	
    //
    // Don't trust pSqReq.
    //
	dwErr = InspectSgReq(pSgReq, pDevice->DiskInfo.di_bytes_per_sect);
	if (dwErr != ERROR_SUCCESS) {		
	    DEBUGMSG(ZONE_ERR, (TEXT("USBMSC>ScsiRWSG: InspectSGReq failed. Error: %d\n"), dwErr));
		return dwErr;
    }
	
    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL);
    if (ERROR_SUCCESS != dwErr) {
		DEBUGMSG(ZONE_ERR, (TEXT("USBMSC>ScsiRWSG: AcquireRemoveLock failed. Error: %d\n"), dwErr));
        return dwErr;
    }
	
    EnterCriticalSection(&pDevice->Lock);	
	
	//
	// Determine whether the Scatter/Gather buffers are sector aligned.
	// If the buffers are not sector aligned, then double buffering is
	//     required.
	// If the buffers are sector aligned, then dwFlags == 0.
	// Otherwise, dwFlags != 0.
	//
    dwErr = CheckSegments(pDevice, pSgReq, (PDWORD) &uiExpectedBytesXferred, &dwFlags);	

	if (ERROR_SUCCESS == dwErr) {
		
		HANDLE hProc = GetCallerProcess();                // Calling process.
		LONGLONG llSectorsRemaining = pSgReq->sr_num_sec; // Total number of sectors to transfer.
		UINT uiStartingSector = pSgReq->sr_start;         // Starting sector.
		UINT uiSectorsToXfer = 0;                         // Number of sectors to transfer in current sub-transfer.
		UINT uiBytesXferred = 0;                          // Number of bytes to transfer in current sub-transfer.
		UINT sg;                                          // Index of current Scatter/Gather buffer.
		
        if (dwFlags == 0) {
			
			//
			// Scatter/Gather buffers ARE sector aligned.
			// Double buffering is NOT required.
			// TRANSFER EACH SCATTER/GATHER BUFFER SEPARATELY.
            //            
            for (sg = 0; (sg < pSgReq->sr_num_sg) && (llSectorsRemaining > 0) && (pSgReq->sr_sglist[sg].sb_buf != NULL) && (pSgReq->sr_sglist[sg].sb_len != 0); sg++) {
				
                //
				// Map the caller's buffer into our context, if necessary.
				//
                PVOID pBuffer = MapCallerPtr((LPVOID)pSgReq->sr_sglist[sg].sb_buf, pSgReq->sr_sglist[sg].sb_len);
                if (pSgReq->sr_sglist[sg].sb_buf != NULL && pBuffer == NULL) {
                    DEBUGMSG(ZONE_ERR,(TEXT("USBMSC>ScsiRWSG: Security violation\r\n")));
                    return ERROR_ACCESS_DENIED;
                }
				ASSERT(pBuffer != NULL);
                ASSERT((pSgReq->sr_sglist[sg].sb_len % pDevice->DiskInfo.di_bytes_per_sect) == 0);
				
                //
                // Calculate the number of contiguous sectors to transfer
                //    (from the current Scatter/Gather buffer).
                // This is not redundant, as pSgReq->sr_num_sec gives the total
                //    number of sectors transferred through the Scatter/Gather 
                //    request, and this calculation determines the number of
                //    contiguous sectors in the current Scatter/Gather buffer.
                //
                uiSectorsToXfer = min((pSgReq->sr_sglist[sg].sb_len / pDevice->DiskInfo.di_bytes_per_sect), (DWORD) llSectorsRemaining);
                uiBytesXferred = pSgReq->sr_sglist[sg].sb_len;

				ASSERT(uiSectorsToXfer != 0);
                ASSERT(uiSectorsToXfer <= llSectorsRemaining);

				DEBUGMSG(ZONE_READ, (TEXT("Scsi%sSG[%d] uiStartingSector: %d, uiSectorsToXfer: %d\n"), bRead ? TEXT("Read") : TEXT("Write"), sg, uiStartingSector, uiSectorsToXfer));
				
                //
                // Transfer the contiguous sectors in the current
                //     Scatter/Gather buffer.
                // (Recall) Double buffer is not required, as all buffers are 
                //     sector aligned.
                //
				dwErr = ScsiReadWrite(pDevice, uiStartingSector, uiSectorsToXfer, pBuffer, &uiBytesXferred, Lun,  bRead);				
                if (ERROR_SUCCESS == dwErr) {
					llSectorsRemaining  -= uiSectorsToXfer;
					ASSERT(llSectorsRemaining >= 0);
                    uiStartingSector += uiSectorsToXfer;                    
                    pBuffer = (PVOID) ((PUCHAR) pBuffer + uiBytesXferred);
                    uiActualBytesXferred += uiBytesXferred;
                }
                else {
                    LeaveCriticalSection(&pDevice->Lock);
                    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
                    DEBUGMSG(ZONE_ERR,(TEXT("USBMSC>ScsiRWSG: ScsiReadWrite failed. Error: %d\n"), dwErr));
                    return dwErr;					
                }                
            }        
        }
		else {
			
			//
			// Scatter/Gather buffers are NOT sector aligned.
			// Double buffering IS required.
            //			
            UINT uiDbBuffOffset = 0;       // Current position in double buffer.
			UINT uiBytesInSgBuff = 0;      // Number of bytes in current Scatter/Gather buffer.
			PUCHAR pucSgBuffOffset = NULL; // Pointer to current position in double buffer.
			
            memset(&pDevice->SgBuff[0], 0, SG_BUFF_SIZE);

			DEBUGMSG(ZONE_TRACE, (TEXT("USBMSC>ScsiRWSG: SgReq (%s) buffers not sector aligned, double buffer\n"), bRead ? (TEXT("Read")) : (TEXT("Write"))));

            //
			// If reading, read from storage device to double buffer, and fill
			//     each Scatter/Gather buffer in order, re-filling the double
			//     buffer as necessary.
			// If writing, fill double buffer from each Scatter/Gather buffer
			//     in order, flushing the double buffer to storage device as
			//     necessary.
            //
            for (sg = 0; sg < pSgReq->sr_num_sg; sg += 1) {
				
				//
			    // Map the caller's buffer into our context, if necessary.
			    //
                pucSgBuffOffset = MapCallerPtr((LPVOID)pSgReq->sr_sglist[sg].sb_buf, pSgReq->sr_sglist[sg].sb_len);
                if (pSgReq->sr_sglist[sg].sb_buf != NULL && pucSgBuffOffset == NULL) {
                    DEBUGMSG(ZONE_ERR,(TEXT("USBMSC>ScsiRWSG: Security violation\r\n")));
                    return ERROR_ACCESS_DENIED;
                }
				uiBytesInSgBuff = (bRead ? 0 : (UINT) pSgReq->sr_sglist[sg].sb_len);
				
				while (bRead ? (uiBytesInSgBuff < pSgReq->sr_sglist[sg].sb_len) : (uiBytesInSgBuff > 0)) {

                    if (bRead) {                       
						if (uiDbBuffOffset == uiBytesXferred) {														
                            uiSectorsToXfer = ((llSectorsRemaining > (SG_BUFF_SIZE / pDevice->DiskInfo.di_bytes_per_sect)) ? (SG_BUFF_SIZE / pDevice->DiskInfo.di_bytes_per_sect) : (UINT) llSectorsRemaining);
					        uiBytesXferred = uiSectorsToXfer * pDevice->DiskInfo.di_bytes_per_sect;
						    goto __DbXfer;
__DbRead:
	                        ;
						}
                    }

                    //
                    // If reading, copy a byte from the double buffer to the
                    //     current Scatter/Gather buffer.
                    // If writing, copy a byte from the current Scatter/Gather
                    //     buffer to the double buffer.
                    //
					bRead ? (*pucSgBuffOffset = pDevice->SgBuff[uiDbBuffOffset]) : (pDevice->SgBuff[uiDbBuffOffset] = *pucSgBuffOffset);
					bRead ? (uiBytesInSgBuff += 1) : (uiBytesInSgBuff -= 1);
					uiDbBuffOffset += 1;
					pucSgBuffOffset++;
					uiActualBytesXferred += 1;

                    if ( ! bRead ) {
                        uiSectorsToXfer = ((llSectorsRemaining > (SG_BUFF_SIZE / pDevice->DiskInfo.di_bytes_per_sect)) ? (SG_BUFF_SIZE / pDevice->DiskInfo.di_bytes_per_sect) : (UINT) llSectorsRemaining);
					    uiBytesXferred = uiSectorsToXfer * pDevice->DiskInfo.di_bytes_per_sect;
						if (uiDbBuffOffset == uiBytesXferred) {
							goto __DbXfer;
__DbWrite:							
							;
						}						
                    }
					goto __DbEnd;
__DbXfer:
                    //
					// Fill/flush the double buffer from/to the storage device.
					//							
					DEBUGMSG(ZONE_TRACE, (TEXT("USBMSC>ScsiRWSG: StartSec: %d, SecsToXfer: %d, BytesXfer'd: %d\n"), uiStartingSector, uiSectorsToXfer, uiBytesXferred));
					dwErr = ScsiReadWrite(pDevice, uiStartingSector, uiSectorsToXfer, &pDevice->SgBuff[0], &uiBytesXferred, Lun, bRead);
					if (dwErr != ERROR_SUCCESS) {
						DEBUGMSG(ZONE_ERR, (TEXT("USBMSC>ScsiRWSG: ScsiReadWrite failed. Error: %d\n"), dwErr));
						LeaveCriticalSection(&pDevice->Lock);
					    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
						return dwErr;
					}
					uiDbBuffOffset = 0;
                    llSectorsRemaining -= uiSectorsToXfer;
                    uiStartingSector += uiSectorsToXfer;
			        if ((uiBytesXferred != (uiSectorsToXfer * pDevice->DiskInfo.di_bytes_per_sect))) {
						LeaveCriticalSection(&pDevice->Lock);
					    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
						return ERROR_DISK_OPERATION_FAILED;
			        }
					if (bRead) goto __DbRead;
					goto __DbWrite;
__DbEnd:
                    ;
				}
		    }

            if ((llSectorsRemaining != 0) || (uiStartingSector != (pSgReq->sr_start + pSgReq->sr_num_sec))) {
				LeaveCriticalSection(&pDevice->Lock);
			    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
				return ERROR_DISK_OPERATION_FAILED;
            }
            if (( ! bRead ) && uiBytesInSgBuff != 0) {
				LeaveCriticalSection(&pDevice->Lock);
				ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
				return ERROR_DISK_OPERATION_FAILED;
            }
		}
    } else {
        DEBUGMSG(ZONE_ERR,(TEXT("USBMSC>ScsiRWSG: CheckSegments failed. Error: %d.\n"),dwErr));
    }
	
	if (uiExpectedBytesXferred != uiActualBytesXferred) {
		LeaveCriticalSection(&pDevice->Lock);
		ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
		return ERROR_DISK_OPERATION_FAILED;
	}
	
	pSgReq->sr_status = dwErr;

	LeaveCriticalSection(&pDevice->Lock);
    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);
	
    DEBUGMSG(ZONE_TRACE,(TEXT("USBMSC<ScsiRWSG: ->sr_status: %d\n"), dwErr));
	
    return uiActualBytesXferred;
}

/* ++

ScsiReadWrite: 
    
    Transfers full sectors.

-- */
DWORD
ScsiReadWrite(
    IN PSCSI_DEVICE pDevice,
    IN DWORD        dwStartSector,
    IN DWORD        dwNumSectors,
    IN OUT PVOID    pvBuffer,        // pointer to transfer buffer
    IN OUT PDWORD   pdwTransferSize, // IN: sizeof input buffer, OUT: number of bytes transferred
    IN UCHAR        Lun,
    IN BOOL         bRead
    )
{
    TRANSPORT_COMMAND tCommand;
    UCHAR             bCDB[MAX_CDB];
    TRANSPORT_DATA    tData;
    DWORD             dwErr;
    DWORD             dwSectorBytes;
    UCHAR             retries = 0;

    DEBUGMSG(ZONE_TRACE,(TEXT("USBMSC>ScsiReadWrite\n")));

    if (!pDevice || !pDevice->DiskInfo.di_bytes_per_sect || 
        !pvBuffer || !pdwTransferSize) {
        return ERROR_INVALID_PARAMETER;
    }

    // buffer size limitations.
    if (0 == *pdwTransferSize || !dwNumSectors) {
        DEBUGMSG(ZONE_ERR,(TEXT("ScsiReadWrite: invalid transfer size:%u, %u\n"),
            *pdwTransferSize, dwNumSectors));
        TEST_TRAP();
        return ERROR_INVALID_PARAMETER;
    }

    dwSectorBytes = dwNumSectors * pDevice->DiskInfo.di_bytes_per_sect;

    // Transfer buffers must be large enough to fit the requested # sectors
    if (*pdwTransferSize < dwSectorBytes)
    {
        DEBUGMSG(ZONE_ERR,(TEXT("ScsiReadWrite: buffer too small (%u < %u)\n"), *pdwTransferSize, dwSectorBytes));
        return ERROR_INSUFFICIENT_BUFFER;
    }

    dwErr = AcquireRemoveLock(&pDevice->RemoveLock, NULL);
    if ( ERROR_SUCCESS != dwErr) {
        return dwErr;
    }

    EnterCriticalSection(&pDevice->Lock);

    tCommand.Flags = bRead ? DATA_IN : DATA_OUT;
    tCommand.Timeout = dwNumSectors * (bRead ? pDevice->Timeouts.ReadSector : pDevice->Timeouts.WriteSector);
    tCommand.Length = USBMSC_SUBCLASS_SCSI == pDevice->DiskSubClass ? SCSI_CDB_10 : UFI_CDB;
    tCommand.CommandBlock = bCDB;
    tCommand.dwLun=Lun;

    memset(bCDB, 0, sizeof(bCDB) );
    bCDB[0] = bRead ? SCSI_READ10 : SCSI_WRITE10;

    ASSERT(Lun <= 0x7);
    bCDB[1] = ((Lun & 0x7) << 5);

    // Logical Block Address
    SetDWORD( &bCDB[2], dwStartSector);

    // TransferLength (in sectors)
    SetWORD( &bCDB[7], (WORD)dwNumSectors);

    tData.TransferLength = 0;
    tData.RequestLength  = dwSectorBytes;
    tData.DataBlock = pvBuffer;

    DEBUGMSG(ZONE_READ,(TEXT("Scsi%s - LBA:%d TL:%d TimeOut:%d\n"), 
        bRead ? TEXT("Read") : TEXT("Write"), dwStartSector, dwNumSectors, tCommand.Timeout));

    do {
        // since we do retries we also have to (re)poke the device
        dwErr = ScsiUnitAttention(pDevice, pDevice->Lun);
        if (ERROR_SUCCESS != dwErr) 
            break;

        dwErr = UsbsDataTransfer( pDevice->hUsbTransport,
                                  &tCommand,
                                  &tData );

        if ( ERROR_SUCCESS == dwErr ) {

            *pdwTransferSize = tData.TransferLength;

        } else {
        
            dwErr = ScsiGetSenseData(pDevice, Lun);

            if (ERROR_SUCCESS == dwErr)
                dwErr = ERROR_GEN_FAILURE;

            DEBUGMSG(ZONE_ERR,(TEXT("ScsiReadWrite ERROR:%d\n"), dwErr));

            *pdwTransferSize = 0;
        }

    } while ((ERROR_SUCCESS != dwErr) && (++retries < 3));

    LeaveCriticalSection(&pDevice->Lock);

    ReleaseRemoveLock(&pDevice->RemoveLock, NULL);

    DEBUGMSG(ZONE_TRACE,(TEXT("USBMSC<ScsiReadWrite:%d\n"), dwErr));

    return dwErr;
}

// EOF
