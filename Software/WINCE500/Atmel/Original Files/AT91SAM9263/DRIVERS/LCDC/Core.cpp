//-----------------------------------------------------------------------------
//! \addtogroup DRIVERS
//! @{	
//
//  All rights reserved ADENEO SAS 2005
//!
//-----------------------------------------------------------------------------
//! \file		AT91SAM9263/DRIVERS/LCDC/Core.cpp
//!
//! \brief		Implementation of LCDC61hw class, heritating from DDGPE abstract class
//!
//! \if subversion
//!   $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9263EK_v100_rc4/PLATFORM/COMMON/SRC/ARM/ATMEL/AT91SAM9263/DRIVERS/LCDC/Core.cpp $
//!   $Author: Tenneyd $
//!   $Revision: 2 $
//!   $Date: 2/04/09 8:41a $
//! \endif
//!
//-----------------------------------------------------------------------------
//! \addtogroup LCDC
//! @{
//
//! \addtogroup LCDC63
//! @{	


//! \addtogroup Core
//! @{
#include "AT91SAM9263_LCDC.h"
#include "LCDC2DGE.h"

#define LCDC_HCLK_ENABLE    (1 << 17)
#define LCDC_CORECLK_ENABLE (1 << 12)
#define LCDC_PCKR_ID        4

#define NO_HARDWARE_BLT_THRESOLD	(1000)
//-----------------------------------------------------------------------------
//! \brief		Constructor
//!
//! Initialize graphic mode, VideoMem and cursor. 
//-----------------------------------------------------------------------------
LCDC63hw::LCDC63hw(DWORD dwBaseVideoMemStartAddress,DWORD dwVideoMemHeight, DWORD dwVideoMemWidth, DWORD dwVideoMemBusWidth):LCDC6xhw(dwBaseVideoMemStartAddress, dwVideoMemHeight, dwVideoMemWidth)
{
	DEBUGMSG(1, (TEXT("-> LCDC63hw constructor")));

	m_dwVideoMemBusWidth = dwVideoMemBusWidth;
	m_p2DGE = new LCDC2DGE(m_pLCDC);
	InitializeCriticalSection(&m_cs);

	DEBUGMSG(1, (TEXT("<- LCDC63hw constructor")));
}
//-----------------------------------------------------------------------------
//! \brief		Destructor
//!
//-----------------------------------------------------------------------------
LCDC63hw::~LCDC63hw()
{
}


void LCDC63hw::ApplyPowerState(CEDEVICE_POWER_STATE dx)
{
	switch(dx) 
	{
			case D0:        // turn the 2DGE on
			case D1:		
			case D2:
				/// todo ...
				break;
			case D3:		// turn the 2DGE off
			case D4:
				/// todo ...
				break;
	}
	LCDC6xhw::ApplyPowerState(dx);
}


void LCDC63hw::Config2DEngine(void)
{
	RETAILMSG(1, (L"Config2DEngine\r\n"));
	m_pLCDC->LCDC_DMACON = DMA_2DEN;
	m_pLCDC->LCDC_DMA2DCFG = m_VidMemStrideByte - (m_nScreenWidthSave * m_pMode->Bpp / 8);
	m_pLCDC->LCDC_DMACON |= DMA_UPDT;
}

#ifdef USE_HW_ACCELERATION

//-----------------------------------------------------------------------------
//! \brief		Perform the actions necessary to switch to a new graphic mode
//!
//! Not really implemented yet. It only reconfigures the 2DGE
//-----------------------------------------------------------------------------
SCODE LCDC63hw::SetMode(int modeId, HPALETTE* pPaletteHandle)
{
	
	T_BPP_2DGE temp_BPP;
	SCODE sResult = LCDC6xhw::SetMode(modeId,pPaletteHandle);
	
	m_dwPixelSize = m_pMode->Bpp; //for BPP_16
	switch (m_pMode->Bpp)
	{
	case 8:
		temp_BPP = BPP_8;
		break;
		
	case 16:
		temp_BPP = BPP_16;
		break;
		
	case 24:
		temp_BPP = BPP_24;
		break;
	
	case 32:
		temp_BPP = BPP_32;
		break;
		
	default:
		temp_BPP = BPP_16;
		break;
	}
	
	
	m_p2DGE->Configure(temp_BPP, (m_VidMemStrideByte*8) / m_VidMemBpp,m_dwVideoMemBusWidth,m_pPhyVidMemAddr);
	
	
	return sResult;
}

__forceinline void ReorganizeCoordinate(register RECTL *pInRect,register RECTL *pOutRect)
{
	
	if (pInRect->left > pInRect->right)
	{		
		pOutRect->left = pInRect->right;
		pOutRect->right = pInRect->left;
	}
	else
	{
		pOutRect->right = pInRect->right;
		pOutRect->left = pInRect->left;
	}

	if (pInRect->top > pInRect->bottom)
	{		
		pOutRect->top = pInRect->bottom;
		pOutRect->bottom = pInRect->top;
	}
	else
	{
		pOutRect->bottom = pInRect->bottom;
		pOutRect->top = pInRect->top;
	}
}

#ifdef USE_HW_BLT_ACCELERATION
//-----------------------------------------------------------------------------
//! \brief		Implements HW blit operation on 9263 2D Chip
//!
//! You can had other HW operation
//! by changing calling conditions in BltPrepare, and adding support in HwBlt function
//-----------------------------------------------------------------------------
SCODE LCDC63hw::DrawBlt(GPEBltParms *pBltParms)
{
	
	RECTL rSrc,rDst;
	enum {
		DirectCopy,
		LineFromBottomCopy,
		LineFromTopCopy,
		ColumnFromRightCopy,
		ColumnFromLeftCopy,
	} eCopyType;

 	SCODE S_Result = S_OK;
	BOOL bAccelerate = FALSE;
	T_BLT_CMD  myBltCmd; // We'll manage up to 4 Cmds
	BOOL bInvertDestfirst = FALSE;
	
	BOOL bUnaryOperation = FALSE;
	hwSurf *pDst = (hwSurf*)pBltParms->pDst;	// cast for readability
	hwSurf *pSrc = (hwSurf*)pBltParms->pSrc;	// cast for readability

	eCopyType = DirectCopy;

	ReorganizeCoordinate(pBltParms->prclDst,&rDst);
	
	
	
	if (!m_pPrimarySurface->Rotate())
	{
		// If destination is in VideoMemory, we can do smthg good :)
		if ( pBltParms->pDst->InVideoMemory() && ((pBltParms->bltFlags & (BLT_TRANSPARENT | BLT_STRETCH | BLT_ALPHABLEND)) == 0) )
		{

			myBltCmd.SzX   = (WORD)(rDst.right-rDst.left);
			myBltCmd.SzY   = (WORD)(rDst.bottom-rDst.top);

			if ((myBltCmd.SzX * myBltCmd.SzY) < NO_HARDWARE_BLT_THRESOLD)
			{
				bAccelerate = FALSE;
				goto shortcutToNoAcceleration;
			}

			// Default value for BltCmd
			myBltCmd.RegAddr1	= 0x0000;
			myBltCmd.NbWord1	= 0x0006;
			myBltCmd.RegAddr2	= 0x0024;
			myBltCmd.NbWord2	= 0x0002;
			
			myBltCmd.Cmd		= BLOCK_TRANSFERT | NO_UPDATE_Y | NO_UPDATE_X | ABSOLUTE;
			
			
			if ((myBltCmd.SzY == 0) || (myBltCmd.SzX == 0))
			{
				return S_OK;
			}
			myBltCmd.DstX  = (WORD)(rDst.left + pDst->Left());
			myBltCmd.DstY  = (WORD)(rDst.top + pDst->Top());
			
			// Test blt formula
			switch(pBltParms->rop4)
			{
				
				// We just had to deal with Dest buffer, 'cause of unary functions
			case 0x0000:	// 0		Dest = 0					-> BLACK FILL
			case 0x5555:	// Dn		Dest = NOT Dest				-> DSTINVERT
			case 0xFFFF:	// 1		Dest = 1					-> WHITE FILL
				
				myBltCmd.SrcX = myBltCmd.DstX;
				myBltCmd.SrcY = myBltCmd.DstY;
				
				bAccelerate = TRUE;
				bUnaryOperation = TRUE;
				break;
				
				// We had to check Src buffer, 'cause of binary functions			
			case 0x3333:	// Sn		Dest = NOT Src				-> NOTSRCCOPY
			case 0x6666:	// DSx		Dest = Src XOR Dest			-> SRCINVERT
			case 0x8888:	// DSa		Dest = Src AND Dest			-> SRCAND
			case 0xCCCC:	// S		Dest = Src					-> SRCCOPY
			case 0xEEEE:	// DSo		Dest = Src OR dest			-> SRCPAINT
			case 0x1111:	//			dest = (NOT src) AND (NOT dest) -> NOTSRCERASE 
				
			case 0x2222:	// DSna		Dest = (NOT Src) AND Dest
			case 0x4444: // dest = source AND (NOT dest ) -> SRCERASE
			case 0xBBBB: // dest = (NOT source) OR dest  -> MERGEPAINT
				
				bUnaryOperation = FALSE;
				// Source must be also in VideoMemory
				if ( pBltParms->pSrc->InVideoMemory() )
				{
					ReorganizeCoordinate(pBltParms->prclSrc,&rSrc);
					myBltCmd.SrcX  = (WORD)(rSrc.left + pSrc->Left());
					myBltCmd.SrcY  = (WORD)(rSrc.top + pSrc->Top());
					
					// Known Issue: The memory width for the frame buffer is 32 bits and thus the DMA alignment is 32 bits. (2 pixels in our case) In consequence the absisses must have the same parity
					if ((m_dwPixelSize < m_dwVideoMemBusWidth) && ((myBltCmd.SrcX ^ myBltCmd.DstX) & 0x1))
					{
						bAccelerate = FALSE;
						goto shortcutToNoAcceleration;
					}
					else
					{
						eCopyType = DirectCopy;
						bAccelerate = TRUE;
						
						
						
						
						// le case ou destx < srx <detsx + szx n'est pas géré !!!!!!!!!!!!!
						
						
						
						
						
						// Everything is ok for HW acceleration.
						// Take care of the order of the copy (like in memcpy and memmove)
						if ( (myBltCmd.SrcX <= myBltCmd.DstX) && (myBltCmd.SrcX + myBltCmd.SzX >= myBltCmd.DstX))
						{
							if (myBltCmd.SrcY == myBltCmd.DstY)
							{
								eCopyType = ColumnFromRightCopy;
								bAccelerate = TRUE; ///\todo : optimize this ! Should do a copy on a column based way starting from the right
							}
							else if ((myBltCmd.SrcY < myBltCmd.DstY) && (myBltCmd.SrcY + myBltCmd.SzY >= myBltCmd.DstY))
							{
								eCopyType = LineFromBottomCopy;
								bAccelerate = TRUE;
							}
							else if ((myBltCmd.SrcY > myBltCmd.DstY) && (myBltCmd.DstY + myBltCmd.SzY >= myBltCmd.SrcY))
							{
								eCopyType = LineFromTopCopy;
								bAccelerate = TRUE;
							}						
						}
						else
							if ( (myBltCmd.DstX <= myBltCmd.SrcX) && (myBltCmd.DstX + myBltCmd.SzX >= myBltCmd.SrcX))
							{
								if (myBltCmd.SrcY == myBltCmd.DstY)
								{
									eCopyType = ColumnFromLeftCopy;
									bAccelerate = TRUE; ///\todo : optimize this ! Should do a copy on a column based way starting from the left
								}
								else if ((myBltCmd.SrcY < myBltCmd.DstY) && (myBltCmd.SrcY + myBltCmd.SzY >= myBltCmd.DstY))
								{
									eCopyType = LineFromBottomCopy;
									bAccelerate = TRUE; 
								}
								else if ((myBltCmd.SrcY > myBltCmd.DstY) && (myBltCmd.DstY + myBltCmd.SzY >= myBltCmd.SrcY))
								{
									eCopyType = LineFromTopCopy;
									bAccelerate = TRUE;
								}						
							}
							
					}					
				}
				
				
				
				
				break;
				
				
				// We can't do anything, :(
			default:
				break;
		}
	}
	
	
	if (bAccelerate)
	{
		switch(pBltParms->rop4)
		{
		case 0x0000:	// 0		Dest = 0
			myBltCmd.LogOp = LOP_XOR;
			break;
			
		case 0x1111:	 // dest = (NOT src) AND (NOT dest) = src NOR dest
			myBltCmd.LogOp = LOP_NOR;
			break;
			
		case 0x3333:	// Sn		Dest = NOT Src				-> NOTSRCCOPY
			myBltCmd.LogOp = LOP_NOT;
			break;
			
		case 0x5555:	// Dn		Dest = NOT Dest				-> DSTINVERT
			myBltCmd.LogOp = LOP_NOT;
			break;
			
		case 0x6666:	// DSx		Dest = Src XOR Dest			-> SRCINVERT
			myBltCmd.LogOp = LOP_XOR;
			break;
			
		case 0x8888:	// DSa		Dest = Src AND Dest			-> SRCAND
			myBltCmd.LogOp = LOP_AND;
			break;
			
		case 0xCCCC:	// S		Dest = Src					-> SRCCOPY
			myBltCmd.LogOp = LOP_WRITE;
			break;
			
		case 0xEEEE:	// DSo		Dest = Src OR Dest			-> SRCPAINT
			myBltCmd.LogOp = LOP_OR;
			break;
			
		case 0xFFFF:	// 1		Dest = WHITE
			myBltCmd.LogOp = LOP_XNOR;
			break;
			
		case 0x2222: // Dest = (NOT Src) AND Dest = (NOT Dest) NOR Src
			bInvertDestfirst = TRUE;
			myBltCmd.LogOp = LOP_NOR;
			break;
		case 0x4444: // dest = source AND (NOT dest ) -> SRCERASE
			bInvertDestfirst = TRUE;
			myBltCmd.LogOp = LOP_AND;
			break;
		case 0xBBBB: // dest = (NOT source) OR dest  -> MERGEPAINT
			bInvertDestfirst = TRUE;
			myBltCmd.LogOp = LOP_NAND;		
			break;
			
		default:
			bAccelerate = FALSE;
			goto shortcutToNoAcceleration;
			break;
		}
	}
	
	
	
	
	
	if (bAccelerate == TRUE)
	{			
		EnterCriticalSection(&m_cs);

		WORD wTmpX,wTmpY;
		WORD wLogOp;
		
		switch (eCopyType)
		{
		case DirectCopy:
			if (bInvertDestfirst)
			{
				
				
				wTmpX = myBltCmd.SrcX;
				wTmpY = myBltCmd.SrcY;
				myBltCmd.SrcX = myBltCmd.DstX;
				myBltCmd.SrcY = myBltCmd.DstY;
				wLogOp = myBltCmd.LogOp;
				myBltCmd.LogOp = LOP_NOT;
				
				m_p2DGE->Push((WORD*)&myBltCmd, sizeof(myBltCmd)/2,FALSE); // Push it on the Fifo	
				
				myBltCmd.SrcX = wTmpX;
				myBltCmd.SrcY = wTmpY;
				myBltCmd.LogOp = wLogOp;
			}
			m_p2DGE->Push((WORD*)&myBltCmd, sizeof(myBltCmd)/2); // Push it on the Fifo				
			break;	 		
			
		case LineFromBottomCopy:				
			{
				//bAccelerate = FALSE;
				
				
				T_BLT_CMD BltCmd;
				WORD wStep = myBltCmd.DstY - myBltCmd.SrcY; //nb line per copy
				WORD wRemaining = myBltCmd.SzY;
				WORD wToCopy;
				
				BltCmd = myBltCmd;
				
				BltCmd.SrcY = myBltCmd.SrcY + myBltCmd.SzY;
				BltCmd.DstY = myBltCmd.DstY + myBltCmd.SzY;
				
				
				while (wRemaining)
				{
					
					wToCopy = MIN(wStep,wRemaining);
					
					wRemaining -= wToCopy;
					BltCmd.DstY -= wToCopy;
					BltCmd.SrcY -= wToCopy;						
					BltCmd.SzY = wToCopy;
					if (bInvertDestfirst)
					{
						
						wTmpX = BltCmd.SrcX;
						wTmpY = BltCmd.SrcY;
						BltCmd.SrcX = BltCmd.DstX;
						BltCmd.SrcY = BltCmd.DstY;
						wLogOp = BltCmd.LogOp;
						BltCmd.LogOp = LOP_NOT;
						
						m_p2DGE->Push((WORD*)&BltCmd, sizeof(BltCmd)/2,FALSE); // Push it on the Fifo	
						
						BltCmd.SrcX = wTmpX;
						BltCmd.SrcY = wTmpY;
						BltCmd.LogOp = wLogOp;
						
					}
					m_p2DGE->Push((WORD*)&BltCmd, sizeof(BltCmd)/2,FALSE); // Push it on the Fifo	
				}
				
				
				
			}
			break;		
		case LineFromTopCopy:				
			{
				//bAccelerate = FALSE;
				
				
				T_BLT_CMD BltCmd;
				WORD wStep = myBltCmd.SrcY - myBltCmd.DstY; //nb line per copy
				WORD wRemaining = myBltCmd.SzY;
				WORD wToCopy;
				
				BltCmd = myBltCmd;
				BltCmd.SzY = wStep;
				
				while (wRemaining)
				{
					
					wToCopy = MIN(wStep,wRemaining);
					
					wRemaining -= wToCopy;
					BltCmd.SzY = wToCopy;
					
					if (bInvertDestfirst)
					{
						wTmpX = BltCmd.SrcX;
						wTmpY = BltCmd.SrcY;
						BltCmd.SrcX = BltCmd.DstX;
						BltCmd.SrcY = BltCmd.DstY;
						wLogOp = BltCmd.LogOp;
						BltCmd.LogOp = LOP_NOT;
						
						m_p2DGE->Push((WORD*)&BltCmd, sizeof(BltCmd)/2,FALSE); // Push it on the Fifo	
						
						BltCmd.SrcX = wTmpX;
						BltCmd.SrcY = wTmpY;
						BltCmd.LogOp = wLogOp;
					}
					
					m_p2DGE->Push((WORD*)&BltCmd, sizeof(BltCmd)/2,FALSE); // Push it on the Fifo	
					
					BltCmd.DstY += wToCopy;
					BltCmd.SrcY += wToCopy;						
					
				}
				
				
				
			}
			break;		
			
		case ColumnFromRightCopy:
		case ColumnFromLeftCopy:
			//m_p2DGE->Push((WORD*)&myBltCmd, sizeof(myBltCmd)/2); // Push it on the Fifo
			bAccelerate = FALSE;
			break;
		}				
		LeaveCriticalSection(&m_cs);
	}
}



shortcutToNoAcceleration:

	if (bAccelerate == FALSE)
	{
			EnterCriticalSection(&m_cs);
		
		m_p2DGE->WaitForHWFifoEmpty();
		m_p2DGE->WaitForOperationComplete();
		
		if ((pBltParms->pDst == m_pPrimarySurface && m_iRotate) || (pBltParms->pSrc == m_pPrimarySurface && m_iRotate))
		{
			S_Result = EmulatedBltRotate(pBltParms);
		}
		else
		{
			S_Result =  EmulatedBlt(pBltParms);
		}

		LeaveCriticalSection(&m_cs);
	}
	
	

	return S_Result;


}
#endif //USE_HW_BLT_ACCELERATION

#ifdef USE_HW_LINE_ACCELERATION
//-----------------------------------------------------------------------------
//! \brief		Implementing HW Line drawing operation on 9263 2D chip
//-----------------------------------------------------------------------------
SCODE LCDC63hw::DrawLine(GPELineParms *pLineParms )
{
	BOOL bAccelerate = FALSE;
	SCODE S_Result = S_OK;
	static T_LINE_CMD myLineCmd = {
		0x0008, // RegAddr1
		0x0009, // NbWord1
		0,// StartY
		0,// StartX
		0,// EndY
		0,// EndX
		0x0001, // Thick
		0,// Pattern
		0,// Color
		0,// LogOp
		LINE_DRAWING | PATTERN_1D | NO_UPDATE_XY | ABSOLUTE // Cmd
	};
	// Keep clipper value
	static DWORD CurrentClipper_Hi32bits  = 0;
	static DWORD CurrentClipper_Low32bits = 0;
	
	if (pLineParms->cPels == 0)
	{
		return S_OK;
	}
	
	
	
	
	EnterCriticalSection(&m_cs);
	
	// If Dst is in VideoMemory
	if ( pLineParms->pDst->InVideoMemory() )
	{
		// Check if the line parms are HW'able
		// Compute coord of the end of the Line:
		if( pLineParms->dN == 0 ) // Line is vertical or horizontal
		{
			if  ( // check line style 
			(pLineParms->style == 0xAAAAAAAA) && (pLineParms->mix == 0x0B07) || // XOR dotted-line (Selection in WinCE)
			(pLineParms->style == 0x00000000) && (pLineParms->mix == 0x0707) || // XOR Full-line (When windows appear & disappear in WinCE)
			(pLineParms->style == 0x00000000) && (pLineParms->mix == 0x0D0D) )  // Simple Full-line
		{
			
			
			
				hwSurf *pDst = (hwSurf*)pLineParms->pDst;
			long StartX = pLineParms->xStart + pDst->Left();
			long StartY = pLineParms->yStart + pDst->Top();
			long EndX, EndY;
			

				// Draw a line starting at pLineParms->xStart, pLineParms->yStart
				// in the direction specified by pLineParms->iDir for a total of pLineParms->cPels pixels
				switch(pLineParms->iDir) // Octant number for the line
				{
				case 0: // +x (horizontal)
				case 7:
					EndX = StartX + pLineParms->cPels -1;
					EndY = StartY;
					break;
				case 1: // +y (vertical)
				case 2:
					EndX = StartX;
					EndY = StartY + pLineParms->cPels -1;
					break;
				case 3: // -x (horizontal)
				case 4:
					EndX = StartX;
					StartX = StartX - pLineParms->cPels + 1;
					EndY = StartY;
					break;
				case 5: // -y (vertical)
				case 6:
					EndX = StartX;
					EndY = StartY;
					StartY = StartY - pLineParms->cPels + 1;					
					break;
				}
				RECTL clipRect;			
			
				// The RefClipStruct is composed of all data you need to know to make a good
				// clip, and it's useful to compare different clip params in order to 
				// send only one clipping cmd to 2D HW.
				typedef struct _RefClipStruct {
					DWORD Hi32bits;
					DWORD Low32bits;
				} RefClipStruct;

				T_CLIP_CMD myClipCmd;
				// A ClipStruct is mappable as a part of ClipCmd
				RefClipStruct *pRefClipStruct = (RefClipStruct *)&(myClipCmd.MinX);


				// Create a RefClipStruct
				ReorganizeCoordinate(pLineParms->prclClip,&clipRect);
				myClipCmd.MinX		= (short)(clipRect.left		+ (long)pDst->Left());
				myClipCmd.MaxX		= (short)(clipRect.right	+ (long)pDst->Left() -1);
				myClipCmd.MinY		= (short)(clipRect.top		+ (long)pDst->Top());			
				myClipCmd.MaxY		= (short)(clipRect.bottom	+ (long)pDst->Top() -1);
			// check if coord are less than 16 bits
			// If coord > 16 bits you should create a different type of LineCmd which use Extensions registers
			if ((StartX & 0xFFFF0000) ||
				(StartY & 0xFFFF0000) ||
				(  EndX & 0xFFFF0000) ||
				(  EndY & 0xFFFF0000) )
			{
				bAccelerate = FALSE;
					goto no_accel;
			}
			
			// Check if color is less than 16 bits
			// If color > 16 bits you should create a different type of LineCmd witch use Extensions registers
			if (pLineParms->solidColor & 0xFFFF0000)
			{
				bAccelerate = FALSE;
					goto no_accel;
			}
			
			// Setting common values for LineCmd
			
			
			myLineCmd.StartX	= (WORD) StartX;
			myLineCmd.EndX		= (WORD) EndX;
			myLineCmd.StartY	= (WORD) StartY;
			myLineCmd.EndY		= (WORD) EndY;
			
			myLineCmd.Color		= (WORD)(pLineParms->solidColor & 0x0000FFFF);
			
			// Setting particular value for LineCmd
			if ( (pLineParms->style == 0xAAAAAAAA) && (pLineParms->mix == 0x0B07) ) // XOR dotted-line (Selection in WinCE)
			{
				// Setup LineCmd struct				
				myLineCmd.Pattern	= (pLineParms->styleState&0x00000001?0x5555:0xAAAA); // If styleState if odd then we should start dot-line with a 0, else with a 1
				myLineCmd.LogOp		= LOP_XOR;
				
				bAccelerate = TRUE;
					goto accel;
			}
			
			if ( (pLineParms->style == 0x00000000) && (pLineParms->mix == 0x0707) ) // XOR Full-line (When windows appear & disappear in WinCE)
			{
				// Setup LineCmd struct
				myLineCmd.Pattern	= 0xFFFF;
				myLineCmd.LogOp		= LOP_XOR;
				
				bAccelerate = TRUE;
					goto accel;
			}
			
			if ( (pLineParms->style == 0x00000000) && (pLineParms->mix == 0x0D0D) ) // Simple Full-line
			{
				// Setup LineCmd struct
				myLineCmd.Pattern	= 0xFFFF;
				myLineCmd.LogOp		= LOP_WRITE;
				
				bAccelerate = TRUE;
					goto accel;
			}
			
accel:
				
				// Don't assume that clipper is enable here, 'cause Blt may have disable it !
				// Compare new Clipper with old Clipper
				if ( pRefClipStruct->Hi32bits!=CurrentClipper_Hi32bits || pRefClipStruct->Low32bits!=CurrentClipper_Low32bits )
				{
				BOOL result= TRUE;
				
				DEBUGMSG(0, (TEXT("Line clip: MinX %d, MinY %d, MaxX %d, MaxY %d"),
					myClipCmd.MinX, myClipCmd.MinY, myClipCmd.MaxX, myClipCmd.MaxY));				
				
				WaitForNotBusy();
				
				result &= m_p2DGE->ClipSet(myClipCmd.MinX, myClipCmd.MinY, myClipCmd.MaxX, myClipCmd.MaxY);
				result &= m_p2DGE->ClipActivated(TRUE);
				
					if (result)
					{
						CurrentClipper_Hi32bits		= pRefClipStruct->Hi32bits;
						CurrentClipper_Low32bits	= pRefClipStruct->Low32bits;
					}
				}
				
				bAccelerate = m_p2DGE->Push((WORD*)&myLineCmd, sizeof(myLineCmd)/2); // Push in the Fifo																
			}
		}
		}
		
no_accel:	
		if (bAccelerate == FALSE)
		{		
		WaitForNotBusy();
		S_Result = EmulatedLine(pLineParms);	// Do a software Line drawing
		}						
		
		LeaveCriticalSection(&m_cs);
		
	return S_Result;
}
#endif //USE_HW_LINE_ACCELERATION


void LCDC63hw::WaitForNotBusy(BOOL bUseInterrupt)
{
	if (m_p2DGE)
	{
		m_p2DGE->WaitForHWFifoEmpty(bUseInterrupt);
		m_p2DGE->WaitForOperationComplete(bUseInterrupt);
	}
}


#endif // USE_HW_ACCELERATION

void LCDCProcSpecificInitPMC(void)
{
	AT91PS_PMC	pPMC;
	PHYSICAL_ADDRESS PhysicalAddress;

	PhysicalAddress.HighPart = 0;
	PhysicalAddress.LowPart = (DWORD) AT91C_BASE_PMC;
	pPMC = (AT91PS_PMC ) MmMapIoSpace(PhysicalAddress,sizeof(AT91S_PMC),FALSE);

	//Initialize the clock management
	pPMC->PMC_SCER = LCDC_HCLK_ENABLE;
	pPMC->PMC_PCKR[LCDC_PCKR_ID] = (AT91C_PMC_CSS_PLLA_CLK | 4);
	pPMC->PMC_SCER = LCDC_CORECLK_ENABLE;
	pPMC->PMC_PCER = ((unsigned int) 1 << AT91C_ID_LCDC);

	MmUnmapIoSpace((PVOID) pPMC, sizeof(AT91S_PMC));
}

void LCDCProcSpecificActivatePMC(void)
{
	AT91PS_PMC	pPMC;
	PHYSICAL_ADDRESS PhysicalAddress;
			
	PhysicalAddress.HighPart = 0;
	PhysicalAddress.LowPart = (DWORD) AT91C_BASE_PMC;
	pPMC = (AT91PS_PMC ) MmMapIoSpace(PhysicalAddress,sizeof(AT91S_PMC),FALSE);

	// Enable the clock to the LCDC
	pPMC->PMC_PCER = ((unsigned int) 1 << AT91C_ID_LCDC);

	MmUnmapIoSpace((PVOID) pPMC, sizeof(AT91S_PMC));
}

void LCDCProcSpecificDeactivatePMC(void)
{
	AT91PS_PMC	pPMC;
	PHYSICAL_ADDRESS PhysicalAddress;

	PhysicalAddress.HighPart = 0;
	PhysicalAddress.LowPart = (DWORD) AT91C_BASE_PMC;
	pPMC = (AT91PS_PMC ) MmMapIoSpace(PhysicalAddress,sizeof(AT91S_PMC),FALSE);
	
	// Disable the clock to the LCDC
	pPMC->PMC_PCDR = ((unsigned int) 1 << AT91C_ID_LCDC);

	MmUnmapIoSpace((PVOID) pPMC, sizeof(AT91S_PMC));
}

DWORD LCDCProcSpecificGetLCDCBaseAddress(void)
{
	return (DWORD)AT91C_BASE_LCDC;
}

DWORD LCDCProcSpecificGetLCDCID(void)
{
	return (DWORD)AT91C_ID_LCDC;
}
// Doxygen End of group Core
//! @} 
// Doxygen End of group LCDC63
//! @}
// Doxygen End of group LCDC
//! @}
// Doxygen End of group Driver
//! @}
////////////////////////////////////////////////////////////////////////////////
// End of $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9263EK_v100_rc4/PLATFORM/COMMON/SRC/ARM/ATMEL/AT91SAM9263/DRIVERS/LCDC/Core.cpp $
////////////////////////////////////////////////////////////////////////////////
//
