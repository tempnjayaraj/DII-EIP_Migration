#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SnTypes.h"

typedef struct _foo {
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

int ReadBmp(char *pcBmpFile, BmpHdr *ptBmp, SnQByte *pqLen, void **ppxRaw)
{
	SnQByte qLen;
    SnWord *pwRaw;
	void *pxRaw;
	SnWord wID;
	int iX, iY, iXPad;
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

	iXPad = (ptBmp->qWidth * 3) & 3;
	if (iXPad)
		iXPad = 4 - iXPad;
	pwRaw = pxRaw;
	for (iY = 0; iY < ptBmp->qHeight; iY++) {
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
//			*pwRaw++ = (pbBGR[2] << 10) | (pbBGR[1] << 5) | pbBGR[0];
		}
		if (iXPad) {
			SnByte pbPad[4];
			if(fread(&pbPad[0], 1, iXPad, ptIn) != iXPad) {
				fprintf(stderr, "Error reading padding in file '%s'.\n", pcBmpFile);
				break;
			}
		}
	}

    fclose(ptIn);

	*ppxRaw = pxRaw;
	*pqLen = qLen;
	
	return 0;
}

int WriteBmp(char *pcBmpFile, BmpHdr *ptBmp, SnQByte qLen, void *pxData)
{
	SnWord wID = 0x4D42;
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

	ptBmp->qFileSize = sizeof(SnWord) + sizeof(BmpHdr) + qLen;
	ptBmp->wBitPerPixel = 16;
	ptBmp->qSizeOfImageData = (qLen + 3) & ~3;

	if (fwrite(ptBmp, 1, sizeof(BmpHdr), ptOut) != sizeof(BmpHdr)) {
        fprintf(stderr, "Error writing file header '%s'.\n", pcBmpFile);
        return -1;
	}
	if (fwrite(pxData, 1, qLen, ptOut) != qLen) {
		fprintf(stderr, "Error writing pixel data in file '%s'.\n", pcBmpFile);
		return -1;
	}

	fclose(ptOut);

	return 0;
}

int main(int argc, char *argv[])
{
	WIN32_FIND_DATA FileData;   // Data structure describes the file found
	HANDLE hSearch;             // Search handle returned by FindFirstFile
	char pcSrcSearch[1024];
	char pcSrcFile[1024];
	char pcDstFile[1024];
	SnQByte qLen;
	void *pxData;
	BmpHdr tBmp;
	long lFile;
	int iStat;

	if (argc != 3){
		fprintf(stderr, "Usage: %s [24bit Input Dir] [16bit Output Dir]\n", argv[0]);
		return -1;
	}

	// Start searching for .bmp files in the src directory.
	sprintf(pcSrcSearch, "%s\\*.bmp", argv[1]);

	hSearch = FindFirstFile (pcSrcSearch, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Usage: %s [24bit Input Dir] [16bit Output Dir]\n", argv[0]);
		return -1;
	}
	do {
		sprintf(pcSrcFile, "%s\\%s", argv[1], FileData.cFileName);
		if (ReadBmp(pcSrcFile, &tBmp, &qLen, &pxData) < 0) {
			return -1;
		}
		sprintf(pcDstFile, "%s\\%s", argv[2], FileData.cFileName);
		if (WriteBmp(pcDstFile, &tBmp, qLen, pxData) < 0) {
			return -1;
		}
	} while(FindNextFile (hSearch, &FileData));

	return 0;
}
