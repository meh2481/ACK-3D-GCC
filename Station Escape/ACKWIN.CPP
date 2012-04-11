// Assorted routines that can be used in Windows based ACK applications
// These routines are not necessarily tied to the ACK Map Editor, but are
// more generic in nature.
// (c) 1995 ACK Software (Lary Myers)
//#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "ackwin.h"
#include "ack3d.h"
#include <stdio.h>

extern "C" {
#include "keys.h"
};

//MEH
extern int         nWindowW;
extern int         nWindowH;

#include "ackres.h"
extern    ACKENG        *ae;            // Interface structure
extern    bool          bNoShow;        // Do not display if true
//extern    HWND          hwndApp;
extern    UCHAR         kbPressed[];
extern    void          *pAckBuffer;
//extern    HDC           AckBuffer;
//extern    int           nBitmapW;
//extern    int           nBitmapH;                       //WinG bitmap dimensions.
extern    short         nRepairObjectCount;
extern    SDL_Rect      rcResolution;

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
    printf("Unable to allocate memory for ACK-3D\n");
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
    printf("ACK-3D Error: Cannot open KIT.OVL\n");
    return(-1);
    }

result = AckInitialize(ae);
if (result)
    {
    printf("ACK-3D Error: Unable to initialize ACK engine\n");
    return(-1);
    }

AckCloseResource();
//
AckRegisterStructure(ae);

return(0);
}

//****************************************************************************
// Use this routine when a window WM_SIZE message is received. Pass the new
// size of the window in the lParam variable. As you can see below, the current
// size is hard-coded to a 320x200 size window for purposes of the Map Editor.
// The current ACK engine only supports sizes up to 320x200.
//****************************************************************************
/*void AckDoSize(HWND hWnd,LPARAM lParam)
{
    HBITMAP hbm;
    int Counter;

nBitmapW = 319; // We'll hard-code the size for now since the Map Editor
nBitmapH = 199; // is a non-sizable window.
// Make sure the width is on an even boundary for the size it is set to.
nBitmapW = ((nBitmapW+3)/4)*4;

if (AckBuffer)
    {
        hbm = (HBITMAP)SelectObject(AckBuffer, hbm);
        DeleteObject(hbm);

    PatBlt(AckBuffer, 0,0,nBitmapW,nBitmapH, BLACKNESS);

    if (ae != NULL)
        {
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

    AckBuffer = GetDC(FindWindow("ACK3DVIEW", NULL));
    hbm = CreateBitmap(320, 200, 1, 8, &pAckBuffer);

    SelectObject(AckBuffer, hbm);

    PatBlt(AckBuffer, 0,0,nBitmapW,nBitmapH, BLACKNESS);
    if (ae != NULL)
        {
        AckRegisterStructure(ae);
        }
    }
}
*/

//****************************************************************************
// Use this routine to process a drawing cycle of the engine. The best place
// to call this is within the message loop handler itself (see ACKEDIT.CPP for
// and example). The routine will check the current status of the keyboard and
// move the POV accordingly.
//****************************************************************************
void DoCycle(void)
{
    short   Amt,MoveAngle;
    short   nResult;

if (bNoShow)
    return;

nResult = 0;
Amt = 16;
if (kbPressed[kbCtrl])
    Amt = 32;

if (kbPressed[kbLeftArrow])
    {
    MoveAngle = (short)(ae->PlayerAngle - INT_ANGLE_90);
    if (MoveAngle < 0)
        MoveAngle += (short)INT_ANGLE_360;
    nResult = AckMovePOV(MoveAngle,Amt);
    }

if (kbPressed[kbRightArrow])
    {
    MoveAngle = (short)(ae->PlayerAngle + INT_ANGLE_90);
    if (MoveAngle >= INT_ANGLE_360)
        MoveAngle -= (short)INT_ANGLE_360;
    nResult = AckMovePOV(MoveAngle,Amt);
    }

if (kbPressed[kbDownArrow])
    {
    MoveAngle = (short)(ae->PlayerAngle + INT_ANGLE_180);
    if (MoveAngle >= INT_ANGLE_360)
        MoveAngle -= (short)INT_ANGLE_360;
    nResult = AckMovePOV(MoveAngle,Amt);
    }

    //Open any doors for the player - here's the spot to do it if we want automatic doors
    //AckCheckDoorOpen(ae->xPlayer,ae->yPlayer,ae->PlayerAngle);

if (kbPressed[kbUpArrow])
    nResult = AckMovePOV(ae->PlayerAngle,Amt);

//MEH Make moving the mouse pan the view
int xCursor, yCursor;
SDL_GetMouseState(&xCursor, &yCursor);
int iDistMoved = xCursor - rcResolution.w / 2;
if(iDistMoved != 0)
{
    ae->PlayerAngle += iDistMoved;
    if(ae->PlayerAngle >= INT_ANGLE_360)
        ae->PlayerAngle -= INT_ANGLE_360;
    if(ae->PlayerAngle < 0)
        ae->PlayerAngle += INT_ANGLE_360;
}
SDL_WarpMouse(rcResolution.w / 2, rcResolution.h / 2);

//Draw the view
//HDC hdc = GetDC(hwndApp);
AckPaint();
//ReleaseDC(hwndApp, hdc);

//TODO: This is specific to Station Escape, and it doesn't work too well. Replace later
if (nResult == POV_OBJECT)
    {
    nResult = AckGetObjectHit();
    if (nResult >= 20 && nResult <= 22)
        {
        ae->ObjList[nResult]->Active = 0;
        nRepairObjectCount++;
        }
    }
}


//****************************************************************************
// Use this routine to process the windows messages WM_KEYDOWN and WM_KEYUP
// as they are received. It will set a global array of virtual keyboard elements
// to correspond to the key being pressed or released. The routine DoCycle will
// then look at this keyboard array to determine if the POV should be moved
// or rotated.
//****************************************************************************
void ProcessKeys(SDL_Event event)
{

switch (event.type)
    {
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
            {
            //MEH WASD instead of arrows
            case SDLK_w:
                kbPressed[kbUpArrow] = 1;
                break;
            case SDLK_s:
                kbPressed[kbDownArrow] = 1;
                break;
            case SDLK_a:
                kbPressed[kbLeftArrow] = 1;
                break;
            case SDLK_d:
                kbPressed[kbRightArrow] = 1;
                break;
            case SDLK_LCTRL:
                kbPressed[kbCtrl] = 1;
                break;
            case SDLK_ESCAPE:
                kbPressed[kbEsc] = 1;
                break;
            case SDLK_SPACE:
                AckCheckDoorOpen(ae->xPlayer,ae->yPlayer,ae->PlayerAngle);
                break;
            case SDLK_LSHIFT:
                kbPressed[kbLeftShift] = 1;
                break;
            case SDLK_TAB:
                kbPressed[kbAlt] = 1;
                break;
            default:
                break;
            }
        break;

    case SDL_KEYUP:
        switch (event.key.keysym.sym)
            {
            case SDLK_w:
                kbPressed[kbUpArrow] = 0;
                break;
            case SDLK_s:
                kbPressed[kbDownArrow] = 0;
                break;
            case SDLK_a:
                kbPressed[kbLeftArrow] = 0;
                break;
            case SDLK_d:
                kbPressed[kbRightArrow] = 0;
                break;
            case SDLK_LCTRL:
                kbPressed[kbCtrl] = 0;
                break;
            case SDLK_ESCAPE:
                kbPressed[kbEsc] = 0;
                break;
            case SDLK_SPACE:
                kbPressed[kbSpace] = 0;
                break;
            case SDLK_LSHIFT:
                kbPressed[kbLeftShift] = 0;
                break;
            case SDLK_TAB:
                kbPressed[kbAlt] = 0;
                break;
            default:
                break;
            }
        break;
    }

}

