// Assorted routines that can be used in Windows based ACK applications
// These routines are not necessarily tied to the ACK Map Editor, but are
// more generic in nature.
// (c) 1995 ACK Software (Lary Myers)
#define  STRICT
#include <windows.h>
//#pragma hdrstop
#include <stdlib.h>
#include <string.h>
//#include "wing.h"
//#include "wingdll.h"
#include "ackwin.h"
#include "ack3d.h"
#include <stdio.h>

extern "C" {
#include "keys.h"

//extern  short   rsHandle;
extern  ULONG   *rbaTable;
extern  short   Resolution;
extern  UCHAR   colordat[];
extern  UCHAR   *BackArray[];
extern  unsigned short FloorMap[];
extern  unsigned short CeilMap[];
extern  unsigned short *ObjGrid;
};

#include "ackres.h"
#ifndef SPEED_AMOUNT
    #define SPEED_AMOUNT 2
#endif


extern    ACKENG        *ae;            // Interface structure
extern    short         nMoveSpeed;
extern    short         nTurnSpeed;
extern    HINSTANCE     hInstGlobal;
extern    BOOL          bNoShow;        // Do not display if TRUE
extern    short         nViewType;      // Current view in grid window
extern    short         nEditType;      // Editing walls or objects
extern    HWND          hwndApp;
extern    UCHAR         kbPressed[];
//extern    WinGdll       WinG;
extern    header        BufferHeader;
extern    void          *pBuffer;
extern    HDC           Buffer;
extern    void          *pAckBuffer;
extern    HDC           AckBuffer;
extern    void          *pMapBuffer;
extern    HDC           MapBuffer;
extern    HPALETTE      hpalApp;
extern    int           nBitmapW;
extern    int           nBitmapH;                       //WinG bitmap dimensions.
extern    RGBQUAD       ColorTable[];
extern    pal           LogicalPalette;
extern    short         nRepairObjectCount;

//MEH Since moving is something that modifies the map
void SetUndoPoint(void);

//****************************************************************************
// Here is a generic ACK Initialization proceedure, with Windows modifications
// for error reporting, etc. Replace the initial values below with your own
// starting values.
//****************************************************************************
short InitACKEngine(void)
{
    short   result;

ae = (ACKENG *)AckMalloc(sizeof(ACKENG));
if (ae == NULL)
    {
    MessageBox(NULL,"Unable to allocate memory","ACK Error",MB_OK);
    return(-1);
    }
memset(ae,0,sizeof(ACKENG));

ae->WinStartX = 0;
ae->WinStartY = 0;
ae->WinEndX = 319;
ae->WinEndY = 199;

ae->LightFlag = SHADING_OFF;
ae->xPlayer = 192;
ae->yPlayer = 640;
ae->PlayerAngle = 0;
ae->TopColor = 0;
ae->BottomColor = 24;
ae->DoorSpeed = 6;
ae->NonSecretCode = 1;
ae->FloorBitmap = 50;
ae->CeilBitmap = 47;

ae->PlayerHeight = DEFAULT_PLAYER_HEIGHT;

result = AckOpenResource("KIT.OVL");
if (result)
    {
    MessageBox(NULL,"Error Opening KIT.OVL","ACK Error",MB_OK);
    return(-1);
    }

//
//if (ae->ScreenBuffer != NULL)
//    {
//    AckFree(ae->ScreenBuffer);
//    ae->ScreenBuffer = NULL;
//    }
//
result = AckInitialize(ae);
if (result)
    {
    MessageBox(NULL,"Error Initializing","ACK Error",MB_OK);
    return(-1);
    }

AckCloseResource();
//
AckRegisterStructure(ae);

return(0);
}

//****************************************************************************
// Use this routine to create the screen palette based on the RGB contents
// of the colordat[] array used by the ACK engine. The global LogicalPalette
// array is assumed to contain the entries for setting the palette.
//****************************************************************************
void CreateAckPalette(void)
{
        int i;
        int nColors;
    UCHAR   col;

#ifdef _USE_SYSTEM_PALETTE
   HDC Screen;
   RGBQUAD *pColorTable;

    Screen = GetDC(0);
    GetSystemPaletteEntries(Screen,0,256,LogicalPalette.aEntries);
    ReleaseDC(0,Screen);

    for (i=0; i<256; i++)
        {
        ColorTable[i].rgbRed = LogicalPalette.aEntries[i].peRed;
        ColorTable[i].rgbGreen = LogicalPalette.aEntries[i].peGreen;
        ColorTable[i].rgbBlue = LogicalPalette.aEntries[i].peBlue;
        ColorTable[i].rgbReserved = 0;
        }

#else

    ClearSystemPalette();

    nColors = 0;

    for (i=0; i<256; i++)
        {
        col = colordat[nColors++];
        col <<= 2;
        ColorTable[i].rgbRed = col;
        col = colordat[nColors++];
        col <<= 2;
        ColorTable[i].rgbGreen = col;
        col = colordat[nColors++];
        col <<= 2;
        ColorTable[i].rgbBlue = col;
        ColorTable[i].rgbReserved = 0;
        }

#endif

if (hpalApp)
    DeleteObject(hpalApp);

hpalApp = CreateIdentityPalette(ColorTable, 256);
}


//****************************************************************************
// Use this routine when a window WM_SIZE message is received. Pass the new
// size of the window in the lParam variable. As you can see below, the current
// size is hard-coded to a 320x200 size window for purposes of the Map Editor.
// The current ACK engine only supports sizes up to 320x200.
//****************************************************************************
//#pragma argsused
void AckDoSize(HWND hWnd,LPARAM lParam)
{
    HBITMAP hbm;
    int Counter;

//nBitmapW = LOWORD(lParam);
//nBitmapH = HIWORD(lParam);
nBitmapW = 319; // We'll hard-code the size for now since the Map Editor
nBitmapH = 199; // is a non-sizable window.
// Make sure the width is on an even boundary for the size it is set to.
nBitmapW = ((nBitmapW+3)/4)*4;

if (AckBuffer)
    {
        /*BufferHeader.Header.biWidth = nBitmapW;
        BufferHeader.Header.biHeight = -nBitmapH;   // -height for top-down display

        hbm = CreateBitmap(320, 200, 1, 8, &pAckBuffer);
        //hbm = WinG.pCreateBitmap(AckBuffer,
        //        (BITMAPINFO *)&BufferHeader, &pAckBuffer); */

        hbm = (HBITMAP)SelectObject(AckBuffer, hbm);
        DeleteObject(hbm);

    PatBlt(AckBuffer, 0,0,nBitmapW,nBitmapH, BLACKNESS);

    if (ae != NULL)
        {
//        ae->ScreenBuffer = (UCHAR *)pAckBuffer;
        ae->WinEndX = (short)nBitmapW;
        ae->WinEndY = (short)nBitmapH;
        if (ae->WinEndX > 319)
            ae->WinEndX = 319;
        if (ae->WinEndY > 199)
            ae->WinEndY = 199;
        AckRegisterStructure(ae);
        }
    }
else
    {
        /*if (WinG.pRecommendDIBFormat((BITMAPINFO *)&BufferHeader))
        {
                BufferHeader.Header.biBitCount = 8;
                BufferHeader.Header.biCompression = BI_RGB;
            }
        else
            {
                BufferHeader.Header.biSize = sizeof(BITMAPINFOHEADER);
                BufferHeader.Header.biPlanes = 1;
                BufferHeader.Header.biBitCount = 8;
                BufferHeader.Header.biCompression = BI_RGB;
                BufferHeader.Header.biSizeImage = 0;
                BufferHeader.Header.biClrUsed = 0;
                BufferHeader.Header.biClrImportant = 0;
        //    }

    BufferHeader.Header.biWidth = nBitmapW;
    BufferHeader.Header.biHeight = -nBitmapH; */

    CreateAckPalette();

    /*for (Counter = 0;Counter < 256;Counter++)
        {
            BufferHeader.aColors[Counter].rgbRed   = ColorTable[Counter].rgbRed;
            BufferHeader.aColors[Counter].rgbGreen = ColorTable[Counter].rgbGreen;
            BufferHeader.aColors[Counter].rgbBlue  = ColorTable[Counter].rgbBlue;
            BufferHeader.aColors[Counter].rgbReserved = 0;
        } */

    AckBuffer = GetDC(FindWindow("ACK3DVIEW", NULL));//CreateDC(TEXT("DISPLAY"),NULL,NULL,NULL);
    hbm = CreateBitmap(320, 200, 1, 8, &pAckBuffer);
    //hbm = WinG.pCreateBitmap(AckBuffer,
    //                            (BITMAPINFO *)&BufferHeader, &pAckBuffer);

    SelectObject(AckBuffer, hbm);

    PatBlt(AckBuffer, 0,0,nBitmapW,nBitmapH, BLACKNESS);
    if (ae != NULL)
        {
//        ae->ScreenBuffer = (UCHAR *)pAckBuffer;
//printf("created AckBuffer\n");
        AckRegisterStructure(ae);
        }
    }
}


//****************************************************************************
// Use this routine to process a drawing cycle of the engine. The best place
// to call this is within the message loop handler itself (see ACKEDIT.CPP for
// and example). The routine will check the current status of the keyboard and
// move the POV accordingly.
//****************************************************************************
void DoCycle(void)
{
    short   Amt,TurnAmt,MoveAngle;
    short   nResult;

if (bNoShow)
    return;

nResult = 0;
Amt = 16;
if (kbPressed[kbCtrl])
    Amt = 32;

Amt += nMoveSpeed;
TurnAmt = (short)(INT_ANGLE_6 + nTurnSpeed);

if (kbPressed[kbLeftArrow])
    {
    ae->PlayerAngle -= TurnAmt;
    if (ae->PlayerAngle < 0)
        ae->PlayerAngle += (short)INT_ANGLE_360;
    }

if (kbPressed[kbRightArrow])
    {
    ae->PlayerAngle += TurnAmt;
    if (ae->PlayerAngle >= INT_ANGLE_360)
        ae->PlayerAngle -= (short)INT_ANGLE_360;
    }

if (kbPressed[kbDownArrow])
    {
    MoveAngle = (short)(ae->PlayerAngle + INT_ANGLE_180);
    if (MoveAngle >= INT_ANGLE_360)
        MoveAngle -= (short)INT_ANGLE_360;
    nResult = AckMovePOV(MoveAngle,Amt);
    }

    //Open any doors for the player
    //AckCheckDoorOpen(ae->xPlayer,ae->yPlayer,ae->PlayerAngle);

if (kbPressed[kbUpArrow])
    nResult = AckMovePOV(ae->PlayerAngle,Amt);


HDC hdc = GetDC(hwndApp);
AckPaint(hwndApp, hdc);
ReleaseDC(hwndApp, hdc);

nMoveSpeed -= (short)3;
// Keep the movement within reasonable bounds. This process
// will simulate acceleration and deceleration of the POV.
if (nMoveSpeed < 0)
    nMoveSpeed = 0;
if (nMoveSpeed > 16)
    nMoveSpeed = 16;
nTurnSpeed -= (short)3;
// Do the same thing with turn speed. By experimenting with
// the number above, various acceleration and deceleration
// levels can be achieved.
if (nTurnSpeed < 0)
    nTurnSpeed = 0;
if (nTurnSpeed > 90)
    nTurnSpeed = 90;

/*if (nResult == POV_OBJECT)
    {
    nResult = AckGetObjectHit();
    if (nResult >= 20 && nResult <= 22)
        {
        ae->ObjList[nResult]->Active = 0;
        nRepairObjectCount++;
        }
    }*/
}


//****************************************************************************
// Use this routine to process the windows messages WM_KEYDOWN and WM_KEYUP
// as they are received. It will set a global array of virtual keyboard elements
// to correspond to the key being pressed or released. The routine DoCycle will
// then look at this keyboard array to determine if the POV should be moved
// or rotated.
//****************************************************************************
void ProcessKeys(UINT iMessage,WPARAM wParam)
{

switch (iMessage)
    {
    case WM_KEYDOWN:
        switch (wParam)
            {
            case VK_UP:
                kbPressed[kbUpArrow] = 1;
                nMoveSpeed += (short)SPEED_AMOUNT;
                break;
            case VK_DOWN:
                kbPressed[kbDownArrow] = 1;
                nMoveSpeed += (short)SPEED_AMOUNT;
                break;
            case VK_LEFT:
                kbPressed[kbLeftArrow] = 1;
                nTurnSpeed += (short)SPEED_AMOUNT;
                break;
            case VK_RIGHT:
                kbPressed[kbRightArrow] = 1;
                nTurnSpeed += (short)SPEED_AMOUNT;
                break;
            case VK_CONTROL:
                kbPressed[kbCtrl] = 1;
                break;
            case VK_ESCAPE:
                kbPressed[kbEsc] = 1;
                break;
            case VK_SPACE:
                AckCheckDoorOpen(ae->xPlayer,ae->yPlayer,ae->PlayerAngle);
                break;
            case VK_SHIFT:
                kbPressed[kbLeftShift] = 1;
                break;
            case VK_TAB:
                kbPressed[kbAlt] = 1;
                break;
            default:
                break;
            }
//        DoCycle();
        break;

    case WM_KEYUP:
        switch (wParam)
            {
            case VK_UP:
                SetUndoPoint();
                nMoveSpeed = 0;
                kbPressed[kbUpArrow] = 0;
                break;
            case VK_DOWN:
                SetUndoPoint();
                nMoveSpeed = 0;
                kbPressed[kbDownArrow] = 0;
                break;
            case VK_LEFT:
                SetUndoPoint();
                nTurnSpeed = 0;
                kbPressed[kbLeftArrow] = 0;
                break;
            case VK_RIGHT:
                SetUndoPoint();
                nTurnSpeed = 0;
                kbPressed[kbRightArrow] = 0;
                break;
            case VK_CONTROL:
                kbPressed[kbCtrl] = 0;
                break;
            case VK_ESCAPE:
                kbPressed[kbEsc] = 0;
                break;
            case VK_SPACE:
                kbPressed[kbSpace] = 0;
                break;
            case VK_SHIFT:
                kbPressed[kbLeftShift] = 0;
                break;
            case VK_TAB:
                kbPressed[kbAlt] = 0;
                break;
            default:
                break;
            }
//        DoCycle();
        break;
    }

}

//****************************************************************************
// Use this routine to process the location of the mouse within the 3D window.
// It will check the mouse position to determine if the left button is being
// pressed near the edges of the window and then will emulate a key being
// pressed accordingly. The DoCycle routine will then handle the emulated
// key as if the keyboard were actually pressed.
//****************************************************************************
void ProcessMouse(HWND hWnd,short nType,short xPos,short yPos)
{
    //MEH Ahh, forget it. Clicking to move sucked, anyway.

    /*short   xMax,yMax;
    RECT    rect;

    memset(kbPressed,0,128);    // Clear out all previous key presses

    if (!nType)                 // For now, a mouse released will just return
    {
        if(nMoveSpeed || nTurnSpeed)    //MEH And if we were moving, the map has been modified
        {
            SetUndoPoint();
        }
        nMoveSpeed = nTurnSpeed = 0;
        return;
    }

    GetClientRect(hWnd,&rect);
    xMax = (short)rect.right;
    yMax = (short)rect.bottom;

    if (yPos < 60)          // Simulate an up arrow if near the top of the window
    {
        kbPressed[kbUpArrow] = 1;
        nMoveSpeed += (short)SPEED_AMOUNT;
    }
    if (yPos > (yMax-60))   // Simulate down arrow if near the bottom
    {
        kbPressed[kbDownArrow] = 1;
        nMoveSpeed += (short)SPEED_AMOUNT;
    }
    if (xPos < 60)          // Simulate left arrow if near left edge
    {
        kbPressed[kbLeftArrow] = 1;
        nTurnSpeed += (short)SPEED_AMOUNT;
    }

    if (xPos > (xMax-60))   // Simulate right arrow if near right edge
    {
        kbPressed[kbRightArrow] = 1;
        nTurnSpeed += (short)SPEED_AMOUNT;
    }*/
}
