#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SnTypes.h"
#include "SoftwareUpgrade.h"

/* 
 * Because the input block may be incompressible, we must provide a little
 * more output space in case that compression is not possible.
 */
#define MAX_COMPRESSED_SIZE(n)	((n) + sizeof(SnQByte))

typedef struct			// Image header (one per BIN image)
{
    SnQByte qImageAddr;
    SnQByte qImageLen;
} NkHdr;

typedef struct			// Record header (one per section in image)
{
    SnQByte qRecordAddr;
    SnQByte qRecordLen;
    SnQByte qRecordChksum;
} NkRec;

typedef struct {
//  SnWord wID;				Read this seperately so the rest of the struct aligns.		
	SnQByte qFileSize;
	SnWord wReserved1;
	SnWord wReserved2;
	SnQByte qDataOffs;
	SnQByte qBitmapHeaderSize;
	SnQByte qWidth;
	SnQByte qHeight;
	SnWord wPlanes;
	SnWord wBitPerPixel;
	SnQByte qCompression;
    SnQByte qSizeOfImageData;
	SnQByte qHorzPixelPerMeter;
	SnQByte qVertPixelPerMeter;
	SnQByte qColors;
	SnQByte qImportantColors;
} BmpHdr;

NkHdr tImg;
NkRec tRec;
SnWinCeHdr tUpgrade = { 0 };
void *pxNkRaw = 0;
void *pxSplashRaw = 0;
void *pxNkCompressed = 0;
void *pxSplashCompressed = 0;

char *pcNkFile = 0;
char *pcSRecFile = 0;
char *pcFootFile = 0;
char *pcSjMduFile = 0;
char *pcRlMduFile = 0;
char *pcSplashFile = 0;
char *pcBootErrorFile = 0;
char *pcOutFile = "NextGen.upg";
char *pcNumberFile = "Numbers.bmp";
SnQByte qSplashScreenRamStart = 0xC1C00000;

#define PROGRAM_FLASH_HALF_SIZE  0x10000
SnWord pwProgramFlash[PROGRAM_FLASH_HALF_SIZE];
#define DATA_FLASH_SIZE 0x1000
SnWord pwDataFlash[DATA_FLASH_SIZE];
#define FALSE 0
#define TRUE  1
SnMotorHdr tMotorHdr;
SnQByte qProgramFlashLength;
SnQByte qDataFlashLength;

SnByte *pbFootProgram = 0;
SnQByte qMaxFootAppSize = 0;
SnRs485Hdr tFootHdr;

#define MDU_SJ_APP_START			0x0800
#define MDU_SJ_APP_MAX_SIZE_BYTES	0x1A80
#define MDU_SJ_BOOTINFO_OFFSET		0x1A70

#define MDU_RL_APP_START			0x1000
#define MDU_RL_APP_MAX_SIZE_BYTES	0x3000
#define MDU_RL_BOOTINFO_OFFSET		0x2FE0

//SnByte *pbMduProgram = 0;
SnByte pbMduProgram[MDU_RL_APP_MAX_SIZE_BYTES];
SnRs485Hdr tMduHdr;

const SnByte pbCrcTable[] = 
{
/*0*/   0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
/*1*/ 157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
/*2*/  35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
/*3*/ 190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
/*4*/  70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
/*5*/ 219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
/*6*/ 101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
/*7*/ 248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
/*8*/ 140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
/*9*/  17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
/*a*/ 175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
/*b*/  50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
/*c*/ 202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
/*d*/  87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
/*e*/ 233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
/*f*/ 116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
//     0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
};

SnByte CrcMem(SnByte *pbSrc, SnQByte qLen)
{
	SnByte bCrc = 0;

    do {
        bCrc = pbCrcTable[bCrc ^ *pbSrc++];
    } while (--qLen > 0);

	return bCrc;
}

int ReadBootWindowsCE(void)
{
    SnByte pbSyncBytes[7];
	SnByte *pbNkRaw;
 	FILE *ptIn;

	ptIn = fopen(pcNkFile, "rb");
	if (ptIn == NULL) {
		fprintf(stderr, "Can't open file '%s' for reading.\n", pcNkFile);
		return -1;
	}
	if (fread(pbSyncBytes, sizeof(pbSyncBytes), 1, ptIn) != 1) {
		fprintf(stderr, "Error reading image header.\n");
		return -1;
	}
	if (fread(&tImg, sizeof(NkHdr), 1, ptIn) != 1) {
		fprintf(stderr, "Error reading image header.\n");
		return -1;
	}

	tUpgrade.qNkRamStart = tImg.qImageAddr;
	tUpgrade.qNkRamLen = tImg.qImageLen;

	pbNkRaw = pxNkRaw = malloc(tImg.qImageLen);
	if (pxNkRaw == NULL) {
		fprintf(stderr, "Error allocating %d bytes of memory.\n", tImg.qImageLen);
		return -1;
	}

	memset(pxNkRaw, 0, tImg.qImageLen);
	for(;;) {
		if (fread(&tRec, sizeof(NkRec), 1, ptIn) != 1) {
			fprintf(stderr, "Error reading record header.\n");
			return -1;
		}
		if (tRec.qRecordAddr == 0) {
			tUpgrade.qNkRamBoot = tRec.qRecordLen;
			break;
		}
		if (fread(pbNkRaw + (tRec.qRecordAddr - tImg.qImageAddr), tRec.qRecordLen, 1, ptIn) != 1) {
			fprintf(stderr, "Error reading record data.\n");
			return -1;
		}
	}

	fclose(ptIn);

	return 0;
}
int ReadBmp(char *pcBmpFile, BmpHdr *ptBmp, SnQByte *pqLen, void **ppxRaw)
{
	SnQByte qLen;
    SnWord *pwRaw;
	void *pxRaw;
	SnWord wID;
	int iX, iY;
 	FILE *ptIn;

    ptIn = fopen(pcBmpFile, "rb");
    if (ptIn == NULL) {
        fprintf(stderr, "Can't open file '%s' for reading.\n", pcBmpFile);
        return -1;
    }
	if (fread(&wID, 1, sizeof(SnWord), ptIn) != sizeof(SnWord) || wID != 0x4D42) {
        fprintf(stderr, "Error reading file ID '%s'.\n", pcBmpFile);
        return -1;
	}
	if (fread(ptBmp, 1, sizeof(BmpHdr), ptIn) != sizeof(BmpHdr)) {
        fprintf(stderr, "Error reading file header '%s'.\n", pcBmpFile);
        return -1;
	}

	qLen = (ptBmp->qWidth * ptBmp->qHeight * 2);
    pxRaw = malloc(qLen);
	if (pxRaw == NULL) {
        fprintf(stderr,
            "Error allocating %d bytes of memory.\n", qLen);
        return -1;
    }

	pwRaw = pxRaw;
	for (iY = ptBmp->qHeight - 1; iY >= 0; iY--) {
		if (fseek(ptIn, 54 + (iY * (ptBmp->qWidth * 3)), SEEK_SET) < 0) {
			fprintf(stderr, "Error seeking in file '%s'.\n", pcBmpFile);
			return -1;
		}
		for (iX = 0; iX < ptBmp->qWidth; iX++) {
			SnByte pbBGR[3];
			int iColor;

			if(fread(&pbBGR[0], 1, 3, ptIn) != 3) {
				fprintf(stderr, "Error reading BGR pixel in file '%s'.\n", pcBmpFile);
				break;
			}
			for(iColor = 0; iColor < 3; iColor++) {
														// b4 b3 b2 b1 b0 X X X
				pbBGR[iColor] >>= 1;					// 0 b4 b3 b2 b1 b0 X X
				pbBGR[iColor] += 0x02;					// C B4 B3 B2 B1 B0 X X
				if(pbBGR[iColor] & 0x80) {
					pbBGR[iColor] = 0x1f;				// Saturate
				} else {
					pbBGR[iColor] >>= 2;				// 0 0 0 B4 B3 B2 B1 B0
				}
			}
			*pwRaw++ = (pbBGR[0] << 10) | (pbBGR[1] << 5) | pbBGR[2];
		}
	}

    fclose(ptIn);

	*ppxRaw = pxRaw;
	*pqLen = qLen;
	
	return 0;
}

int WriteBmp(char *pcBmpFile, BmpHdr *ptBmp, void *pxRaw)
{
    SnWord *pwRaw = pxRaw;
	SnWord wID = 0x4D42;
	int iX, iY;
 	FILE *ptOut;

    ptOut = fopen(pcBmpFile, "wb");
    if (ptOut == NULL) {
        fprintf(stderr, "Can't open file '%s' for writing.\n", pcBmpFile);
        return -1;
    }
	if (fwrite(&wID, 1, sizeof(SnWord), ptOut) != sizeof(SnWord)) {
        fprintf(stderr, "Error writing file ID '%s'.\n", pcBmpFile);
        return -1;
	}
	if (fwrite(ptBmp, 1, sizeof(BmpHdr), ptOut) != sizeof(BmpHdr)) {
        fprintf(stderr, "Error writing file header '%s'.\n", pcBmpFile);
        return -1;
	}

	pwRaw = pxRaw;

	for (iY = ptBmp->qHeight - 1; iY >= 0; iY--) {
		SnWord *pwData = pwRaw + (iY * ptBmp->qWidth);
		for (iX = 0; iX < ptBmp->qWidth; iX++) {
			SnByte pbBGR[3];

			pbBGR[0] = ((*pwData >> 10) & 0x1f) << 3;
			pbBGR[1] = ((*pwData >> 5) & 0x1f) << 3;
			pbBGR[2] = (*pwData & 0x1f) << 3;
			if(fwrite(&pbBGR[0], 1, 3, ptOut) != 3) {
				fprintf(stderr, "Error writing BGR pixel in file '%s'.\n", pcBmpFile);
				break;
			}
			pwData++;
		}
	}

    fclose(ptOut);

	return 0;
}

void CopyBmp(SnWord *pwSrc, SnWord *pwDst, SnQByte qW, SnQByte qH, SnQByte qSrcSpan, SnQByte qDstSpan)
{
	SnQByte qCnt;

	while (qH-- > 0) {
		for (qCnt = 0; qCnt < qW; qCnt++) {
			*pwDst++ = *pwSrc++;
		}
		pwSrc += (qSrcSpan - qW);
		pwDst += (qDstSpan - qW);
	}
}

void CopyVersion(void *pxSrc, void *pxDst, SnQByte qX, SnQByte qY)
{
	SnWord *pwSrc, *pwDst;
	SnQByte qDigit;

	pwDst = (SnWord *)pxDst + (qY * 800) + qX;
	
	qDigit = tUpgrade.bMajorVers % 10;
	pwSrc = (SnWord *)pxSrc + (10 * qDigit);
	CopyBmp(pwSrc, pwDst, 10, 16, 104, 800);
	pwDst += 11;

	pwSrc = (SnWord *)pxSrc + (10 * 10);
	CopyBmp(pwSrc, pwDst, 3, 16, 104, 800);
	pwDst += 4;

	qDigit = (tUpgrade.bMinorVers / 10) % 10;
	pwSrc = (SnWord *)pxSrc + (10 * qDigit);
	CopyBmp(pwSrc, pwDst, 10, 16, 104, 800);
	pwDst += 11;

	qDigit = tUpgrade.bMinorVers % 10;
	pwSrc = (SnWord *)pxSrc + (10 * qDigit);
	CopyBmp(pwSrc, pwDst, 10, 16, 104, 800);
}

int ReadSplash(void)
{
    SnQByte qSplashLen, qNumberLen;
	void *pxNumberRaw;
	BmpHdr tSplashBmp, tNumberBmp;

	if (ReadBmp(pcSplashFile, &tSplashBmp, &qSplashLen, &pxSplashRaw) < 0) {
		return -1;
	}

	if (ReadBmp(pcNumberFile, &tNumberBmp, &qNumberLen, &pxNumberRaw) < 0) {
		return -1;
	}

	if (tSplashBmp.qWidth == 800 &&  tSplashBmp.qHeight == 480)
		CopyVersion(pxNumberRaw, pxSplashRaw, 485, 395);

	WriteBmp("SplashWithVersion.bmp", &tSplashBmp, pxSplashRaw);

    tUpgrade.qSplashRamStart = qSplashScreenRamStart;
	tUpgrade.qSplashRamLen = qSplashLen;

	return 0;
}

// Read a line from file into output buffer pbData
// Return value is number of characters read
// Routine works regardless of whether CR, CR-LF, or LF are used as line separators
static int ReadLine(char *pcFile,int iLine,FILE *ptIn,char *pcData)
{
    int iCharIndex;
    int iChar;
    SnBool yCR;
    SnBool yDone;

    iCharIndex = 0;
    yCR = 0;
    yDone = 0;
    while(!yDone) {
        iChar = fgetc(ptIn);
        switch(iChar) {
        case EOF:
            yDone = 1;
            break;
        case '\r':
            yCR = 1;
            break;
        case '\n':
            yDone = 1;
            break;
        default:
            if(yCR) {
                yCR = 0;
                ungetc(iChar,ptIn);
                yDone = 1;
            } else if(iCharIndex >= 514) {
                fprintf(stderr,"Error: File %s, line %d: Line too long\n",pcFile,iLine);
                return 0;
            } else {
                pcData[iCharIndex++] = (char)iChar;
            }
            break;
        }
    }

    pcData[iCharIndex] = 0;
    return iCharIndex;
}

int GetHexVal(char c)
{
    switch(c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return c - '0';

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
        return c - 'a' + 10;

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
        return c - 'A' + 10;

    default:
        return -1;
    }
}

// Returns 2 if at end of file, 1 if read of valid intel hex line occurred, 0 otherwise
// For data records, the buffer pbBuf is filled in using the address in
// the record i.e. pbBuf is the same size as the memory corresponding to
// the hex file
int ReadIntelHexLine(FILE* ptFile, SnQByte qOffset, SnByte* pbBuf)
{
    int iIndex;
    int iChar;
    char c;
    int iVal;
    int iNumHeaderBytes;
    int iHeaderAddress;
    int iHeaderRecordType;
    int iNumDataBytes;
    SnByte bDataByte;
    SnByte bCalculatedCheckSum;
    SnByte bFileCheckSum;
    int iByteOffset;

    iNumDataBytes = -1;
    bCalculatedCheckSum = 0;
    bDataByte = 0;  // initialize for calculated checksum
    iByteOffset = 0;
    for(iIndex = 0; ; iIndex++) {
        iChar = fgetc(ptFile);
		putchar(iChar);
        if(iChar == EOF) {
            return 2;
        }
        c = (char)iChar;
        switch(iIndex) {
        case 0:
            if((c == '\n') || (c == '\r')) {
                return 1;
            } else if(c != ':') {
                return 0;
            }
            break;

        case 1:
            if((iVal = GetHexVal(c)) < 0) {
                return 0;
            }             
            iNumHeaderBytes = iVal << 4;
            break;

        case 2:
            if((iVal = GetHexVal(c)) < 0) {
                return 0;
            }             
            iNumHeaderBytes |= iVal;
            bCalculatedCheckSum += iNumHeaderBytes & 0xff;
            break;

        case 3:
            if((iVal = GetHexVal(c)) < 0) {
                return 0;
            }
            iHeaderAddress = iVal << 12;
            break;

        case 4:
            if((iVal = GetHexVal(c)) < 0) {
                return 0;
            }
            iHeaderAddress |= iVal << 8;
            bCalculatedCheckSum += (iHeaderAddress >> 8) & 0xff;
            break;

        case 5:
            if((iVal = GetHexVal(c)) < 0) {
                return 0;
            }
            iHeaderAddress |= iVal << 4;
            break;

        case 6:
            if((iVal = GetHexVal(c)) < 0) {
                return 0;
            }
            iHeaderAddress |= iVal;
            bCalculatedCheckSum += iHeaderAddress & 0xff;
            break;

        case 7:
            if((iVal = GetHexVal(c)) < 0) {
                return 0;
            }
            iHeaderRecordType = iVal << 4;
            break;

        case 8:
            if((iVal = GetHexVal(c)) < 0) {
                return 0;
            }
            iHeaderRecordType |= iVal;
            bCalculatedCheckSum += iHeaderRecordType & 0xff;
            break;

        default:
            if((c == '\n') || (c == '\r')) {
                if(iIndex & 1) {
                    bFileCheckSum = bDataByte;
                    if(iNumDataBytes != iNumHeaderBytes) {
                        return 0;
                    } else if(0 != (SnByte)(bFileCheckSum + bCalculatedCheckSum)) {
                        return 0;
                    }
                    return 1;
                } else {
                    return 0;
                }
            } else {
                if(iIndex & 1) {    // ms nibble
                    iNumDataBytes++;
                    bCalculatedCheckSum += bDataByte;   // add old byte first
                    if((iIndex > 9) && (iHeaderRecordType == 0)) {    // data record
						if (iHeaderAddress < qOffset)
							return 0;
                        pbBuf[iHeaderAddress - qOffset + iByteOffset] = bDataByte;
                        iByteOffset++;
                    }

                    if((iVal = GetHexVal(c)) < 0) {
                        return 0;
                    }
                    bDataByte = iVal << 4;
                } else {            // ls nibble
                    if((iVal = GetHexVal(c)) < 0) {
                        return 0;
                    }
                    bDataByte |= iVal;
                }
            }
            break;
        }
    }
}

// Returns 0 if file was good, -1 otherwise
int CrcFoot(void)
{
	SnByte *pbData = pbFootProgram + 2;
	SnQByte qCnt = qMaxFootAppSize - 2;
	SnWord wOldCrc = *(SnWord *)pbFootProgram;
	SnWord wNewCrc = 0;

	while (qCnt--)
		wNewCrc += *pbData++;

	return (wOldCrc == (SnWord)(~wNewCrc)) ? 0 : -1;
}

// Returns 0 if file was good, -1 otherwise
int ReadFootFile(void)
{
    FILE* ptFile;
    
	if (strcmp(strrchr(pcFootFile,  '.'), ".bin") == 0) {
		qMaxFootAppSize = 0x2000;
		ptFile = fopen(pcFootFile,"rb");
		if(!ptFile) {
			fprintf(stderr, "ERROR: Could not open file %s\n", pcFootFile);
			return -1;
		}

		pbFootProgram = malloc(qMaxFootAppSize);
		if (pbFootProgram == 0) {
			fclose(ptFile);
			return -1;
		}
		memset(pbFootProgram, 0xff, qMaxFootAppSize);

		if (fread(pbFootProgram, qMaxFootAppSize, 1, ptFile) != 1 || CrcFoot() != 0) {
			fclose(ptFile);
			free(pbFootProgram);
			pbFootProgram = 0;
			return -1;
		}

		tFootHdr.qType = HDR_RS485_FOOT_1;
		tFootHdr.wVersion = *(SnWord *)&pbFootProgram[4];
		tFootHdr.bProgramFlashCrc = CrcMem(pbFootProgram, qMaxFootAppSize);
		tFootHdr.bHdrCrc = CrcMem((SnByte*)&tFootHdr, sizeof(SnRs485Hdr)-1);
	} else {
		int iStatus;
		const SnQByte qBootInfoOffest = 0x1BF0;

		qMaxFootAppSize = (14*1024/2);
		ptFile = fopen(pcFootFile,"r");
		if(!ptFile) {
			fprintf(stderr, "ERROR: Could not open file %s\n", pcFootFile);
			return -1;
		}

		pbFootProgram = malloc(qMaxFootAppSize);
		if (pbFootProgram == 0) {
			fclose(ptFile);
			return -1;
		}
		memset(pbFootProgram, 0xff, qMaxFootAppSize);

		do {
			iStatus = ReadIntelHexLine(ptFile, 0, pbFootProgram);
		} while (iStatus == 1);
		if (iStatus != 2) {
			fclose(ptFile);
			free(pbFootProgram);
			pbFootProgram = 0;
			return -1;
		}
		pbFootProgram[qBootInfoOffest + sizeof(SnWord)] = CrcMem(pbFootProgram, qBootInfoOffest + sizeof(SnWord));

		tFootHdr.qType = HDR_RS485_FOOT;
		tFootHdr.wVersion = *(SnWord *)&pbFootProgram[qBootInfoOffest];
		tFootHdr.bProgramFlashCrc = CrcMem(pbFootProgram, qMaxFootAppSize);
		tFootHdr.bHdrCrc = CrcMem((SnByte*)&tFootHdr, sizeof(SnRs485Hdr)-1);
	}

    fclose(ptFile);

	return 0;
}

// Returns 0 if file was good, -1 otherwise
int ReadMduHexFile(char *pcMduFile, SnQByte qType, SnQByte qMduStart, SnQByte qMduSize, SnQByte qMduBoot)
{
    int iStatus;
    FILE* ptFile;
    
	//pbMduProgram = malloc(qMduSize);
	//if (pbMduProgram == 0) {
		//return -1;
	//}
	memset(pbMduProgram, 0xff, qMduSize);

    ptFile = fopen(pcMduFile,"r");
    if(!ptFile) {
        fprintf(stderr, "ERROR: Could not open file %s\n", pcMduFile);
        return -1;
    }

	do {
        iStatus = ReadIntelHexLine(ptFile, qMduStart, pbMduProgram);
	} while (iStatus == 1);

    fclose(ptFile);

	pbMduProgram[qMduBoot + sizeof(SnWord)] = CrcMem(pbMduProgram, qMduBoot + sizeof(SnWord));

	tMduHdr.qType = qType;
	tMduHdr.wVersion = *(SnWord *)&pbMduProgram[qMduBoot];
	tMduHdr.bProgramFlashCrc = CrcMem(pbMduProgram, qMduSize);
	tMduHdr.bHdrCrc = CrcMem((SnByte*)&tMduHdr, sizeof(SnRs485Hdr)-1);

	if (iStatus == 2) {
		return 0;
	} else {
		return -1;
	}
}

// Read S record file for motor controller board
int ReadSRec(void)
{
 	FILE *ptIn;
    int iNumChars;
    char pcData[514+1]; // largest line possible in S record file, excluding eol, plus space for null
    int iLine;
    int i;
    SnByte bCS;
    SnByte bVal;
    SnByte bNibble;
    SnWord wVal;
    int iRecLen;
    SnBool ySawS0 = FALSE;  // Sticky
    SnBool ySawS3 = FALSE;  // Sticky
    SnBool ySawS7 = FALSE;  // Sticky
    SnBool yIsS3;           // Transient
    SnAddr aAddr;
    SnWord* pwD;
    SnQByte qCurrentProgramFlashLength;
    SnQByte qCurrentDataFlashLength;
    SnQByte qTrash = 0; // Shut compiler up
    SnQByte* pqCurrentProgramFlashLength;
    SnQByte* pqCurrentDataFlashLength;
    SnAddr aEntryPoint;
    SnByte bNumProgramFlashPages;
    SnByte bDataFlashCrc;
    SnQByte qVersion;

    // Initialize both program and data flash buffers to all 0xF
    (void)memset(&pwProgramFlash[0],0xff,PROGRAM_FLASH_HALF_SIZE * 2);
    (void)memset(&pwDataFlash[0],0xff,DATA_FLASH_SIZE * 2);
    qProgramFlashLength = 0;
    qDataFlashLength = 0;

    ptIn = fopen(pcSRecFile, "rt");
    if (ptIn == NULL) {
        fprintf(stderr, "Can't open file '%s' for reading.\n", pcSRecFile);
        fclose(ptIn);
        return -1;
    }
    for(iLine = 1; ; iLine++) {
        iNumChars = ReadLine(pcSRecFile,iLine,ptIn,&pcData[0]);
        if(!iNumChars) {
            break;
        }
        if(iNumChars & 1) {
            fprintf(stderr,"Error: File %s, line %d: Invalid S record - odd number of characters\n",pcSRecFile,iLine);
            fclose(ptIn);
            return -1;
        }
        // S:1 <Type>:1 <Record Length>:2 <Address>:8 <Data>:n <Checksum>:1
        if(pcData[0] != 'S') {
            fprintf(stderr,"Error: File %s, line %d: Invalid S record - does not start with 'S'\n",pcSRecFile,iLine);
            fclose(ptIn);
            return -1;
        }
        switch(pcData[1]) {
        default:
            fprintf(stderr,"Error: File %s, line %d: Unsupported S record - must be one of S[037]\n",pcSRecFile,iLine);
            fclose(ptIn);
            return -1;
        case '0':   // S0 is a header, address is normally 0, data can be anything
            pcData[iNumChars] = 0;
            if(strcmp(&pcData[2],"110000000050524F4752414D264441544196") != 0) {// This header has address 0 and data "PROGRAM&DATA"
                fprintf(stderr,"Error: File %s, line %d: Unsupported S0 header, make sure file contains both program and data space records\n",
                    pcSRecFile,iLine);
                fclose(ptIn);
                return -1;
            }
            ySawS0 = TRUE;
            continue;
        case '3':   // S3 is a 4-byte address
            ySawS3 = TRUE;
            yIsS3 = TRUE;
            break;
        case '7':   // S7 is termination for S3, address may contain entry point, no data
            ySawS7 = TRUE;
            yIsS3 = FALSE;
            break;
        }

        // Scan S record, collecting record length, address and data as appropriate, and finally verifying the checksum
        // NOTE: Because we start from i=2, the phase 3 check to collect wVal will break for the very first word, however, that
        // is ok, because the first word collected will be unused. However, to silence the compiler, initialize wVal to 0.
        wVal = 0;
        for(i = 2; i < iNumChars; i++) {
            if(('0' <= pcData[i]) && (pcData[i] <= '9')) {
                bNibble = (SnByte)(pcData[i] - '0');
            } else if(('a' <= pcData[i]) && (pcData[i] <= 'f')) {
                bNibble = (SnByte)(pcData[i] - 'a' + 10);
            } else if(('A' <= pcData[i]) && (pcData[i] <= 'F')) {
                bNibble = (SnByte)(pcData[i] - 'A' + 10);
            } else {
                fprintf(stderr,"Error: File %s, line %d: non-hex character '%c'\n",pcSRecFile,iLine,pcData[i]);       
                fclose(ptIn);
                return -1;
            }
            if(i & 1) {
                bVal |= bNibble;
                if((i & 3) == 3) {
                    if(i < 12) {
                        wVal |= (SnWord)bVal;
                    } else {
                        wVal |= (SnWord)bVal << 8;
                    }
                    if(i == 7) {
                        aAddr = (SnAddr)wVal << 16;
                    } else if(i == 11) {
                        aAddr |= wVal;

                        if(yIsS3) {
                            // Normalize address
                            if(aAddr >= 0x02000000) {
                                // Data flash
                                aAddr -= 0x02000000;
                                if((0x2000 <= aAddr) && (aAddr <= 0x2FFF)) {
                                    pwD = &pwDataFlash[aAddr - 0x2000];
                                    qCurrentProgramFlashLength = 0;
                                    pqCurrentProgramFlashLength = &qTrash;
                                    qCurrentDataFlashLength = aAddr - 0x2000;
                                    pqCurrentDataFlashLength = &qCurrentDataFlashLength;
                                } else {
                                    fprintf(stderr,"Warning: File %s, line %d: Ignoring line for data space address 0x%lx\n",
                                        pcSRecFile,iLine,aAddr);   
                                    break;
                                }
                            } else {
                                // Program flash
                                if(aAddr < 0x20000) {
                                    pwD = &pwProgramFlash[aAddr];
                                    qCurrentProgramFlashLength = aAddr;
                                    pqCurrentProgramFlashLength = &qCurrentProgramFlashLength;
                                    qCurrentDataFlashLength = 0;
                                    pqCurrentDataFlashLength = &qTrash;
                                } else {
                                    fprintf(stderr,"Warning: File %s, line %d: Ignoring line for program space address 0x%lx\n",
                                        pcSRecFile,iLine,aAddr);
                                    break;
                                }
                            }
                        } else {
                            // Must be S7, therefore, address is that of entrypoint
                            aEntryPoint = aAddr;
                        }
                    }
                } else {
                    if(i < 12) {
                        wVal = (SnWord)bVal << 8;
                    } else {
                        wVal = (SnWord)bVal;
                    }
                }
            } else {
                bVal = bNibble << 4;
            }
            if(i & 1) {
                if(i == 3) {
                    iRecLen = bVal;
                    bCS = (SnByte)iRecLen;
                    if(iNumChars != ((iRecLen * 2) + 4)) {
                        fprintf(stderr,"Error: File %s, line %d: Invalid record length %d for line length %d\n",
                            pcSRecFile,iLine,iRecLen,iNumChars);      
                        fclose(ptIn);
                        return -1;
                    }
                } else {
                    bCS += bVal;
                    if((i == (iNumChars - 1)) && (bCS != 0xff)) {
                        fprintf(stderr,"Error: File %s, line %d: Invalid checksum\n",pcSRecFile,iLine);
                        fclose(ptIn);
                        return -1;
                    }

                    if(yIsS3 && ((i & 3) == 3) && (11 < i) && (i < (iNumChars - 2))) {
                        // Store data in memory
                        *pwD++ = wVal;
                        *pqCurrentProgramFlashLength += 1;
                        *pqCurrentDataFlashLength += 1;
                    }
                }
            }
        }
        if(yIsS3) {
            if(qCurrentProgramFlashLength > qProgramFlashLength) {
                qProgramFlashLength = qCurrentProgramFlashLength;
            }
            if(qCurrentDataFlashLength > qDataFlashLength) {
                qDataFlashLength = qCurrentDataFlashLength;
            }
        }
    }

    fclose(ptIn);

    if(!ySawS0) {
        fprintf(stderr,"Error: File %s: No S0 record in file\n",pcSRecFile);
        return -1;
    }
    if(!ySawS3) {
        fprintf(stderr,"Error: File %s: No S3 record in file\n",pcSRecFile);
        return -1;
    }
    if(!ySawS7) {
        fprintf(stderr,"Error: File %s: No S7 record in file\n",pcSRecFile);
        return -1;
    }

    // Program flash length is limited to 120KB (60KW)
    if(qProgramFlashLength > (120*512)) {
        fprintf(stderr,"Error: File %s: Program flash size %ld > 60KW, not supported\n",
            pcSRecFile,qProgramFlashLength);
        return -1;
    }
    bNumProgramFlashPages = (qProgramFlashLength + 511) / 512; // 1 <= x <= 120, so fits in byte
    if(qDataFlashLength > DATA_FLASH_SIZE) {
        fprintf(stderr,"Error: File %s: Data flash size %ld > %ld, not supported\n",
            pcSRecFile,qDataFlashLength,DATA_FLASH_SIZE);
        return -1;
    }

    // DATA_FLASH_SIZE - 14,13 contain qVersion (filled in by app, little endian)
    // Report version number as courtesy
    qVersion = *(SnQByte*)&pwDataFlash[DATA_FLASH_SIZE - 14];
    fprintf(stdout,"File %s: Version is (bAppType=0x%02x bMajor=0x%02x bMinor=0x%02x bBuildNumber=0x%02x)\n",
        pcSRecFile,(SnByte)(qVersion>>24),(SnByte)(qVersion>>16),(SnByte)(qVersion>>8),(SnByte)qVersion);

    // DATA_FLASH_SIZE - 12 contains entry point
    if(aEntryPoint > 0xffff) {
        fprintf(stderr,"Error: File %s: Entry point 0x%lx >= 0xffff, not supported\n",
            pcSRecFile,aEntryPoint);
        return -1;
    }
    pwDataFlash[DATA_FLASH_SIZE - 12] = (SnWord)aEntryPoint;

    // DATA_FLASH_SIZE - 11 contains bProgramFlashCrc (LSB) and bAlign=0 (MSB)
    pwDataFlash[DATA_FLASH_SIZE - 11] = (SnWord)CrcMem((SnByte*)&pwProgramFlash[0],(SnQByte)bNumProgramFlashPages*1024);

    // DATA_FLASH_SIZE - 10 contains bNumProgramFlashPages (LSB) and bDataFlashCrc=0 (MSB) (little endian)
    // bDataFlashCrc will be modified later
    pwDataFlash[DATA_FLASH_SIZE - 10] = (SnWord)bNumProgramFlashPages;

    // DATA_FLASH_SIZE - 9 thru DATA_FLASH_SIZE - 1 should contain all FFFF
    for(i = 0; i < 9; i++) {
        pwDataFlash[DATA_FLASH_SIZE - 9 + i] = 0xffff;
    }

    // Now store data flash crc, which spans data flash thru the crc location - 1
    wVal = pwDataFlash[DATA_FLASH_SIZE - 10];
    wVal &= 0x00ff;
    wVal |= CrcMem((SnByte*)&pwDataFlash[0],DATA_FLASH_SIZE*2-18-1) << 8; // byte size
    pwDataFlash[DATA_FLASH_SIZE - 10] = wVal;

    // Fill in motor header
    if((qVersion >> 24) != HDR_MOTOR_NEXTGEN && (qVersion >> 24) != HDR_MOTOR_BASIC) {
        fprintf(stderr,"File %s: bAppType=%d is not a motor header type\n",
            pcSRecFile,(SnByte)(qVersion >> 24));
        return -1;
    }
    tMotorHdr.qType = (qVersion >> 24);
    tMotorHdr.bAlign = 0;
    tMotorHdr.bNumProgramFlashPages = bNumProgramFlashPages;
    tMotorHdr.bProgramFlashCrc = CrcMem((SnByte*)&pwProgramFlash[0],(int)bNumProgramFlashPages*1024);    // To page boundary, byte units
        // Note that the following data flash crc is over all of data flash
    tMotorHdr.bDataFlashCrc = CrcMem((SnByte*)&pwDataFlash[0],DATA_FLASH_SIZE*2);// byte size
    tMotorHdr.bMajorVers = (SnByte)(qVersion >> 16);
    tMotorHdr.bMinorVers = (SnByte)(qVersion >> 8);
    tMotorHdr.bBuildVers = (SnByte)qVersion;
    tMotorHdr.bHdrCrc = CrcMem((SnByte*)&tMotorHdr,sizeof(tMotorHdr)-1);

	return 0;
}

void RunLengthCompress(SnQByte *pqSrc, SnQByte qSrcLen, SnQByte *pqDst, SnQByte *pqDstLen)
{
	SnQByte qDstLen = 0;
	SnQByte qRunLen = 0;
	SnQByte *pqStart;
	SnQByte qMatch;

	pqStart = pqDst++;
	qDstLen += 4;
	while (qSrcLen > 0) {
		qMatch = *pqSrc++;
		qSrcLen -= 4;
		if (qSrcLen >= 8 && qMatch == *pqSrc && qMatch == *(pqSrc+1)) {
			if (qRunLen > 0) {
				*pqStart = qRunLen;
				pqStart = pqDst++;
				qDstLen += 4;
			}
			*pqDst++ = qMatch;
			qDstLen += 4;
			qRunLen = 1;
			do {
				pqSrc++;
				qSrcLen -= 4;
				qRunLen++;
			} while (qSrcLen >= 4 && *pqSrc == qMatch);
			*pqStart = qRunLen | 0x80000000;
			if (qSrcLen >= 4) {
				pqStart = pqDst++;
				qDstLen += 4;
			}
			qRunLen = 0;
		} else {
			*pqDst++ = qMatch;
			qDstLen += 4;
			qRunLen++;
		}
	}
	if (qRunLen > 0) {
		*pqStart = qRunLen;
	}

	*pqDstLen = qDstLen;
}

// Return 0 on success
int Parse(int argc,char *argv[])
{
    int i;
    char* pcDot;

	tUpgrade.qType = HDR_WINCE_NEXTGEN;

	for(i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-splash") == 0) {
            pcSplashFile = argv[++i];
        } else if(strcmp(argv[i], "-base") == 0) {
			sscanf(argv[++i], "%i", &qSplashScreenRamStart);
        } else if(strcmp(argv[i], "-nk") == 0) {
            pcNkFile = argv[++i];
        } else if(strcmp(argv[i], "-srec") == 0) {
            pcSRecFile = argv[++i];
        } else if(strcmp(argv[i], "-foot") == 0) {
            pcFootFile = argv[++i];
        } else if(strcmp(argv[i], "-sj") == 0) {
			if (pcRlMduFile) {
				fprintf(stderr, "Only one Mdu update per update file.\n");
				return 1;
			}
			pcSjMduFile = argv[++i];
        } else if(strcmp(argv[i], "-rl") == 0) {
			if (pcSjMduFile) {
				fprintf(stderr, "Only one Mdu update per update file.\n");
				return 1;
			}
            pcRlMduFile = argv[++i];
        } else if(strcmp(argv[i], "-booterr") == 0) {
            pcBootErrorFile = argv[++i];
        } else if(strcmp(argv[i], "-out") == 0) {
            pcOutFile = argv[++i];
		} else if(strcmp(argv[i], "-basic") == 0) {
			tUpgrade.qType = HDR_WINCE_BASIC;
        } else if(strcmp(argv[i], "-vers") == 0) {
			int iMajor, iMinor, iBuild;
			int iNumFields = sscanf(argv[++i], "%d.%d.%d", &iMajor, &iMinor, &iBuild);

			if (iNumFields < 2 || iNumFields > 3) {
				fprintf(stderr, "Invalid format for Version Number: '%s'.\n", argv[i]);
				fprintf(stderr, "Use X.X or X.X.X\n");
				return 1;
			}
            if (iNumFields >= 2) {
				tUpgrade.bMajorVers = iMajor;
				tUpgrade.bMinorVers = iMinor;
			}
			if (iNumFields == 3) {
				tUpgrade.bBuildVers = iBuild;
			} else {
				tUpgrade.bBuildVers = 0;
            }
        } else {
            fprintf(stderr, "Unknown option '%s'\n", argv[i]);
            return 1;
        }
    }

    if(pcSplashFile) {
        pcDot = strrchr(pcSplashFile, '.');
        if(pcDot) {
            if(strcmp(pcDot+1, "bmp") != 0) {
                fprintf(stderr, "Unknown extension for splash file '%s'\n", pcSplashFile);
                return 1;
            }
        }
    }
    if(pcBootErrorFile) {
        pcDot = strrchr(pcBootErrorFile, '.');
        if(pcDot) {
            if(strcmp(pcDot+1, "bmp") != 0) {
                fprintf(stderr, "Unknown extension for boot error file '%s'\n", pcBootErrorFile);
                return 1;
            }
        }
    }

    return 0;
}

int ProcessBootError(void)
{
	char pcBootErrorOutFile[1024];
	void *pxRaw, *pxCompressed;
 	SnQByte qRawLen, qCompressedLen, qCol;
	SnQByte *pqData;
	BmpHdr tBmp;
	SnByte bCrc;
	char *pcPtr;
	FILE *ptOut;

	if (ReadBmp(pcBootErrorFile, &tBmp, &qRawLen, &pxRaw) < 0) {
		return -1;
	}

	pxCompressed = malloc(MAX_COMPRESSED_SIZE(qRawLen));
	if (pxCompressed == NULL) {
        fprintf(stderr,
            "Error allocating %d bytes of memory.\n", qRawLen);
		return -1;
	}

	RunLengthCompress(pxRaw, qRawLen, pxCompressed, &qCompressedLen);
	bCrc = CrcMem(pxCompressed, qCompressedLen);

	strcpy(pcBootErrorOutFile, pcBootErrorFile);
	pcPtr = strrchr(pcBootErrorOutFile, '.') + 1;
	*pcPtr++ = 'c';
	*pcPtr = 0;

	ptOut = fopen(pcBootErrorOutFile, "w");
	if (ptOut == NULL) {
		fprintf(stderr, "Can't open file '%s' for writing.\n", pcBootErrorOutFile);
		return -1;
	}

	fprintf(ptOut, "#include \"SnTypes.h\"\n\n");
	fprintf(ptOut, "const SnQByte qBootErrorScreenLen = %d;\n\n", qCompressedLen);
	fprintf(ptOut, "const SnByte bBootErrorScreenCrc = %d;\n\n", bCrc);
	fprintf(ptOut, "const SnQByte pqBootErrorScreen[] = {\n");

	qCol = 0;
	pqData = pxCompressed;
	qCompressedLen >>= 2;
	while (qCompressedLen-- > 0) {
		fprintf(ptOut, " 0x%08x,", *pqData++);
		if (++qCol == 8) {
			fprintf(ptOut, "\n");
			qCol = 0;
		}
	}

	fprintf(ptOut, "};\n");

	return 0;
}

int main(int argc, char *argv[])
{
	void *pxNkIn, *pxSplashIn;
	FILE *ptOut;

    if(Parse(argc,argv)) {
        return -1;
    }


    if(!pcNkFile && !pcSplashFile && !pcSRecFile && !pcBootErrorFile &&
	   !pcFootFile && !pcSjMduFile && !pcRlMduFile) {
        // Nothing to do
        fprintf(stderr,"Nothing to do!\n");
        return -1;
    }
    if(pcNkFile && (ReadBootWindowsCE() < 0)) {
        return -1;
    }
    if(pcSplashFile && (ReadSplash() < 0)) {
        return -1;
    }
    if(pcSRecFile && (ReadSRec() < 0)) {
        return -1;
    }
    if(pcBootErrorFile && (ProcessBootError() < 0)) {
        return -1;
    }
    if(pcFootFile && (ReadFootFile() < 0)) {
        return -1;
    }
    if(pcSjMduFile && (ReadMduHexFile(pcSjMduFile, HDR_RS485_MDU_SJ, MDU_SJ_APP_START,
	  MDU_SJ_APP_MAX_SIZE_BYTES, MDU_SJ_BOOTINFO_OFFSET) < 0)) {
        return -1;
    }
    if(pcRlMduFile && (ReadMduHexFile(pcRlMduFile, HDR_RS485_MDU_RL, MDU_RL_APP_START,
	  MDU_RL_APP_MAX_SIZE_BYTES, MDU_RL_BOOTINFO_OFFSET) < 0)) {
        return -1;
    }

	if (pcNkFile || pcSRecFile || pcFootFile || pcSjMduFile || pcRlMduFile) {
		ptOut = fopen(pcOutFile, "wb");
		if (ptOut == NULL) {
			fprintf(stderr, "Can't open file '%s' for writing.\n", pcOutFile);
			return -1;
		}

		if(pcSRecFile) {
			printf("Motor Hdr Offset: 0x%08lx\n",ftell(ptOut));
			if(fwrite(&tMotorHdr, sizeof(SnMotorHdr),1,ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
			printf("Motor Program Offset: 0x%08lx\n",ftell(ptOut));
			if(fwrite(&pwProgramFlash[0], (int)tMotorHdr.bNumProgramFlashPages * 1024,1,ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
			printf("Motor Data Offset: 0x%08lx\n",ftell(ptOut));
			if(fwrite(&pwDataFlash[0], DATA_FLASH_SIZE*2,1,ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
		}

		if(pcNkFile) {
			pxNkCompressed = malloc(MAX_COMPRESSED_SIZE(tUpgrade.qNkRamLen));
			if (pcSplashFile)
				pxSplashCompressed = malloc(MAX_COMPRESSED_SIZE(tUpgrade.qSplashRamLen));

			if (pxNkCompressed)
				RunLengthCompress(pxNkRaw, tUpgrade.qNkRamLen, pxNkCompressed, &tUpgrade.qNkRomLen);
			if (pxSplashCompressed)
				RunLengthCompress(pxSplashRaw, tUpgrade.qSplashRamLen, pxSplashCompressed, &tUpgrade.qSplashRomLen);

			if (pxNkCompressed == NULL || tUpgrade.qNkRomLen > tUpgrade.qNkRamLen) {
				pxNkIn = pxNkRaw;
				tUpgrade.qNkRomLen = tUpgrade.qNkRamLen;
				tUpgrade.bNkCopy = COPY_UNCOMPRESSED;
				printf("Problem in compressing NK image, using uncompressed data...\n");
			} else {
				pxNkIn = pxNkCompressed;
				tUpgrade.bNkCopy = COPY_RL_COMPRESSED;
				printf("'%s' %d -> %d\n", pcNkFile, tUpgrade.qNkRamLen, tUpgrade.qNkRomLen);
			}
			if (pcSplashFile) {
				if (pxSplashCompressed == NULL || tUpgrade.qSplashRomLen > tUpgrade.qSplashRamLen) {
					pxSplashIn = pxSplashRaw;
					tUpgrade.qSplashRomLen = tUpgrade.qSplashRamLen;
					tUpgrade.bSplashCopy = COPY_UNCOMPRESSED;
					printf("Problem in compressing Splash image, using uncompressed data...\n");
				} else {
					pxSplashIn = pxSplashCompressed;
					tUpgrade.bSplashCopy = COPY_RL_COMPRESSED;
					printf("'%s' %d -> %d\n", pcSplashFile, tUpgrade.qSplashRamLen, tUpgrade.qSplashRomLen);
				}
			}

			tUpgrade.bNkCrc = CrcMem(pxNkIn, tUpgrade.qNkRomLen);
			if (pcSplashFile)
				tUpgrade.bSplashCrc = CrcMem(pxSplashIn, tUpgrade.qSplashRomLen);
			tUpgrade.bHdrCrc = CrcMem((SnByte *)&tUpgrade, sizeof(SnWinCeHdr) - 1);

			if(fwrite(&tUpgrade, sizeof(SnWinCeHdr), 1, ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
			if (pcSplashFile) {
				printf("Splash Base: 0x%08lx\n", qSplashScreenRamStart);
				printf("Splash Offset: 0x%08lx\n", ftell(ptOut));
				if(fwrite(pxSplashIn, (tUpgrade.qSplashRomLen + 3) & ~0x3, 1, ptOut) != 1) {
					fprintf(stderr,"Error writing output file\n");
					fclose(ptOut);
					return -1;
				}
			}
	            
			printf("Nk Offset: 0x%08lx\n", ftell(ptOut));
			if(fwrite(pxNkIn, (tUpgrade.qNkRomLen + 3) & ~0x3, 1, ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
		}

		if(pcFootFile) {
			printf("Foot Hdr Offset: 0x%08lx\n",ftell(ptOut));
			if(fwrite(&tFootHdr, sizeof(SnRs485Hdr),1,ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
			printf("Foot Program Offset: 0x%08lx\n",ftell(ptOut));
			if(fwrite(&pbFootProgram[0],qMaxFootAppSize,1,ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
			if (pbFootProgram) {
				free(pbFootProgram);
				pbFootProgram = 0;
			}
		}
		if(pcSjMduFile) {
			printf("Small Joint Mdu Hdr Offset: 0x%08lx\n",ftell(ptOut));
			if(fwrite(&tMduHdr, sizeof(SnRs485Hdr),1,ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
			printf("Small Joint Mdu Program Offset: 0x%08lx\n",ftell(ptOut));
			if(fwrite(&pbMduProgram[0],MDU_SJ_APP_MAX_SIZE_BYTES,1,ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
		}
		if(pcRlMduFile) {
			printf("Reliant Mdu Hdr Offset: 0x%08lx\n",ftell(ptOut));
			if(fwrite(&tMduHdr, sizeof(SnRs485Hdr),1,ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
			printf("Reliant Mdu Program Offset: 0x%08lx\n",ftell(ptOut));
			if(fwrite(&pbMduProgram[0],MDU_RL_APP_MAX_SIZE_BYTES,1,ptOut) != 1) {
				fprintf(stderr,"Error writing output file\n");
				fclose(ptOut);
				return -1;
			}
		}
		printf("File size: 0x%08lx\n",ftell(ptOut));

		fclose(ptOut);
	}

	return 0;
}
