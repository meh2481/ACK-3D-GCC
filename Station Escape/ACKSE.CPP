// Example source for using ACK-3D in Windows
// (c) 1995 ACK Software (Lary Myers)
//#define  STRICT
#include <windows.h>
//#pragma hdrstop
#include <stdlib.h>
#include <string.h>
//#include "wing.h"
//#include "wingdll.h"
#include "ackwin.h"
#include "ackse.h"
#include <stdio.h>
#include <fstream> //MEH for file streams. Much easier for me to use

#define IDM_RES1    1100
#define IDM_RES2    1101
#define IDM_RES3    1102
#define IDM_HELP    1103
#define IDM_BACK    1104
#define IDM_RES4    1105
#define IDM_RES5    1106

#include "ack3d.h"
extern "C" {
#include "keys.h"
//#include "ack3d.h"
void AckDoubleBuffer(UCHAR *Src,UCHAR *Dest);
extern  UCHAR   colordat[];
};

// Some globals to use
ACKENG      *ae = 0;        // Interface structure
HWND        hwndApp;
UCHAR       kbPressed[128];
//WinGdll     WinG;
header      BufferHeader;
void        *pAckBuffer = 0;
HDC         AckBuffer = 0;
HPALETTE    hpalApp;
RGBQUAD     ColorTable[256];
pal         LogicalPalette = {0x300,256};
HBITMAP     gbmOldMonoBitmap = 0;
int         StretchFactor = 1;
int         nBitmapW;
int         nBitmapH;        // WinG bitmap dimensions.
int         nWindowW;
int         nWindowH;
short       nMoveSpeed;
short       nTurnSpeed;
short       nIntroCount;
short       nInfoMode;
HINSTANCE   hInstGlobal;
BOOL        bNoShow;        // Do not display if TRUE
BOOL        bInStatus;      // Recursive flag for status checks
UCHAR       *pIntroBuf;
short       nSecondsLeft;
BOOL        bUpdateTime;
UINT        nTimerID;
WORD        nTimerCount;
short       nRepairObjectCount;
UCHAR       SavePal[768];

ColorRange  ranges[64] = {
  {16,15},
  {32,16},
  {48,16},
  {64,16},
  {80,16},
  {96,8},
  {104,8},
  {112,8},
  {120,8},
  {128,8},
  {136,8},
  {144,8},
  {152,8},
  {160,8},
  {168,8},
  {176,8},
  {184,16},
  {200,16},
  {216,16},
  {232,16},
  {248,16},
  {0,0}
  };

// Keeps the list of movable objects
short   nMoveObjectList[] = {
  6,7,8,9,10,11,12,
  18,19,
  0}; // Denotes end of table

  //MEH buffer for holding our pallette data. Read in from "pallette.txt"
  PalletteSlot pallette[256];

  //MEH 24-bit screen buffer, for Win32 reasons
   unsigned char OffscreenBuffer[4*320*200]; //4 chars per pixel (rgba), 320 pixels by 200 pixels

HDC hOffscreenDC = 0;
HBITMAP hOffscreenBitmap;

//****************************************************************************
// The main program window
//****************************************************************************
int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
  int     result;
  HBITMAP hbm;
  char    szBuf[40];

pIntroBuf = NULL;
// Register our Window class if this is the first time in
if (!hPrevInstance)
  if (!Register(hThisInstance))
    return(-1);

// Prime the ACK-3D engine with our startup parameters
if (InitACKEngine())
  return(-1);

hInstGlobal = hThisInstance;

ReadInPallette();//MEH read in the pallette data from "pallette.txt"

// Open our default resource file and load bitmaps, etc.
AckOpenResource("pics.dtf");
result = AckProcessInfoFile();
if (result)
  {
  wsprintf(szBuf,"Error %d in resource",result);
  MessageBox(NULL,szBuf,"Error",MB_OK);
  AckCloseResource();
  return(-1);
  }

srand(GetTickCount());
memcpy(SavePal,colordat,768);
//pIntroBuf = AckReadPCX((char *)2);
LoadBackDrop(); //MEH WHY WASN'T THIS CALLED HERE??!?! RGH
AckCloseResource();
bUpdateTime = FALSE;
nTimerCount = 0;
nRepairObjectCount = 0;
nSecondsLeft = (60 * 60) + (60 * 30);
AckSetupPalRanges(ae, ranges);

// Create the actual window to display the rendered view
if (!CreateAll(hThisInstance))
  return(-1);

nIntroCount = 0;

// Here we loop until the application closes
result = MessageLoop();

// Free up our buffers we used from WinG
if (AckBuffer)
  {
  hbm = (HBITMAP)SelectObject(AckBuffer, gbmOldMonoBitmap);
  DeleteObject(hbm);
  DeleteDC(AckBuffer);
  }

if(hpalApp)
  DeleteObject(hpalApp);

DeleteObject(hOffscreenDC);
DeleteObject(hOffscreenBitmap);

// Shutdown WinG DLL
//WinG.Free();
return(result);
}

//****************************************************************************
// Registers the window class.
//****************************************************************************
BOOL Register(HINSTANCE hInst)
{
  WNDCLASS wndclass;

wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
wndclass.lpfnWndProc   = AckWndProc;
wndclass.cbClsExtra    = 0;
wndclass.cbWndExtra    = 0;
wndclass.hInstance     = hInst;
wndclass.hIcon         = LoadIcon(hInst,"ACKICON.ICO");
wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
wndclass.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
wndclass.lpszMenuName  = NULL;
wndclass.lpszClassName = "ACK3DVIEW";

if (!RegisterClass(&wndclass))
  return(FALSE);
return(TRUE);
}

//****************************************************************************
// Loads the WinG DLL and creates the display window for the application.
//****************************************************************************
BOOL CreateAll(HINSTANCE hInst)
{
  int     windx, windy;
  HMENU   hMenu;

//if (!WinG.Load())
//  {
//  MessageBox(0,"Can't find WING32.DLL","Error!",MB_OK);
//  return(FALSE);
//  }

nBitmapW = 319;                 // Default size (window client size to use)
nBitmapH = 199;

windx = windy = GetSystemMetrics(SM_CXBORDER)*2;
windx += nBitmapW;
windy += nBitmapH + GetSystemMetrics(SM_CYCAPTION) - 1;
nWindowW = windx;
nWindowH = windy;

// Note that in this example we're not allowing any resizing borders
// This helps to keep things alittle more constrained with the current
// ACK-3D engine.
hwndApp = CreateWindow ("ACK3DVIEW",       // Class name
                "ACK-3D Station Escape",   // Caption
                WS_POPUP|WS_BORDER|WS_CAPTION|WS_VISIBLE|WS_SYSMENU,
                0,                      // Position
                0,
                windx,windy,            // Size
                (HWND)NULL,             // Parent window
                (HMENU)NULL,            // use class menu
                hInst,                  // handle to window instance
                (LPSTR)NULL );          // no params to pass on

if (!hwndApp)
  {
  MessageBox(NULL,"Unable to create 3D window","ERROR",MB_OK);
  return(FALSE);
  }

// Now we'll display the window and setup the WinG buffer when we
// receive the WM_SIZE message.
ShowWindow(hwndApp,SW_NORMAL);
hMenu = GetSystemMenu(hwndApp,FALSE);
// Add in the menu items for the game window
AppendMenu(hMenu,MF_SEPARATOR,0,NULL);
AppendMenu(hMenu,MF_STRING,IDM_RES1,"320 x 200");
AppendMenu(hMenu,MF_STRING,IDM_RES2,"480 x 300");
AppendMenu(hMenu,MF_STRING,IDM_RES3,"640 x 400");
AppendMenu(hMenu,MF_STRING,IDM_RES4,"800 x 600");
AppendMenu(hMenu,MF_STRING,IDM_RES5,"1280 x 800");
AppendMenu(hMenu,MF_SEPARATOR,0,NULL);
AppendMenu(hMenu,MF_STRING,IDM_HELP,"&Help");
AppendMenu(hMenu,MF_STRING,IDM_BACK,"&Background story");
return(TRUE);
}

//****************************************************************************
// Here is where we loop until the application terminates.
//****************************************************************************
int MessageLoop(void)
{
    int iCurTicks = GetTickCount();
  MSG msg;

for (;;)
  {
  if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE))
    {
    if (msg.message == WM_QUIT)
      break;

    TranslateMessage(&msg);
    DispatchMessage(&msg);
    }
  else
    {
        //MEH Enough of this 1000 fps! Time for limited fps
        int iNewTick = GetTickCount();
        if(iNewTick - iCurTicks > 30) //~30 fps
        {
            iCurTicks = iNewTick;
            DoCycle();  // Display the 3D window
        }

    }
  }
return msg.wParam;
}

//****************************************************************************
// Turns light shading on and off.
//****************************************************************************
void LightOnOff(short nMode)
{
  BOOL    bSet = FALSE;

if (!nMode && ae->LightFlag == SHADING_OFF)
  {
  ae->LightFlag = SHADING_ON;
  bSet = TRUE;
  }

if (nMode && ae->LightFlag == SHADING_ON)
  {
  ae->LightFlag = SHADING_OFF;
  bSet = TRUE;
  }

if (bSet)
  {
  AckSetupPalRanges(ae,ranges);
  AckRegisterStructure(ae);
  }
}

//****************************************************************************
// Checks to see if the bomb thrown by the player hits one of the bad guys.
//****************************************************************************
void CheckBadGuys(UCHAR nIndex)
{
  short   i;
  UCHAR   ObjIndex;
  NEWOBJECT   *pObj;

i = 0;
while (nMoveObjectList[i])  // Check the list of movable objects
  {
  ObjIndex = nMoveObjectList[i++];
  if (ObjIndex == nIndex)
    {
    pObj = ae->ObjList[ObjIndex];
    if (pObj != NULL)
      {
      pObj->Active = 0;  // Bad guy has been hit--make inactive
      }
    break;
    }
  }
}

//****************************************************************************
// Locates the first inactive bomb.
//****************************************************************************
UCHAR FindBombObject(void)
{
  UCHAR       index;
  NEWOBJECT   *pObj;

for (index = BOMB_START_INDEX; index < BOMB_END_INDEX; index++)
  {
  pObj = ae->ObjList[index];
  if (pObj != NULL && pObj->Active == 0)
    return(index);
  }
return(0);
}

//****************************************************************************
// Moves the active bombs.
//****************************************************************************
void MoveBombs(void)
{
  UCHAR   Index;
  UCHAR   nHitIndex;
  short   nResult;
  NEWOBJECT   *pBomb,*pSman;

for (Index = BOMB_START_INDEX; Index <= BOMB_END_INDEX; Index++)
  {
  pBomb = ae->ObjList[Index];
  if (pBomb != NULL)
    {
    if (pBomb->Active)
      {
      nHitIndex = 0;
      // See if bomb will hit something
      nResult = AckMoveObjectPOV(Index,pBomb->Dir,pBomb->Speed);
      if (nResult != POV_NOTHING)
        {
        if (Index == BOMB_END_INDEX && nResult != POV_PLAYER)
          {
           pBomb->Active = 0;
           if (nResult == POV_OBJECT)
             CheckBadGuys(AckGetObjectHit());
           }
         else
           {
           if (nResult == POV_OBJECT)
             nHitIndex = AckGetObjectHit();
           if (nHitIndex != pBomb->id)
             {
              pBomb->Active = 0;
              pSman = ae->ObjList[pBomb->id];
              if (pSman != NULL)
                pSman->id = 0;
              pBomb->id = 0;
              }
            }
        }
      }
    }
  }
}

//****************************************************************************
// See if a bad guy is not cuurently firing a bomb. If so, set up the bomb
// for firing.
//****************************************************************************
void CheckFiring(short nObjIndex,NEWOBJECT *pObj)
{
  UCHAR   Index;
  NEWOBJECT   *pBomb;
  long    dx,dy;

if (pObj->id)   // Bad guy is currently firing
  return;

Index = FindBombObject();  // Get the first inactive bomb

if (Index)
  {  // Set all of the attributes for firing the bomb
  pObj->id = Index;
  dx = ae->xPlayer - pObj->x;
  dy = ae->yPlayer - pObj->y;
  pBomb = ae->ObjList[Index];
  pBomb->Dir = AckGetObjectAngle(dx,dy);
  pBomb->x = pObj->x;
  pBomb->y = pObj->y;
  pBomb->id = nObjIndex;
  pBomb->Speed = 32;
  pBomb->Active = 1;
  pBomb->mPos = (short)((pBomb->y & 0xFFC0) + (pBomb->x >> (short)6));
  }
}

//****************************************************************************
// Move the objects in the game.
//****************************************************************************
void MoveObjects(void)
{
  short       i,nIndex,nResult;
  short       nRow,nCol;
  NEWOBJECT   *pObj;

i = 0;
while (nMoveObjectList[i])
  {
  nIndex = nMoveObjectList[i];
  i++;
  pObj = ae->ObjList[nIndex];
  if (pObj != NULL && pObj->Active && (pObj->Flags & OF_MOVEABLE))
    {
    AckCheckDoorOpen(pObj->x,pObj->y,pObj->Dir);
    nResult = AckMoveObjectPOV(nIndex,pObj->Dir,pObj->Speed);
    if (nResult != POV_NOTHING)
      {
      if (nResult == POV_XWALL || nResult == POV_YWALL)
        {
        nResult = AckGetWallHit();
        if (nResult & DOOR_TYPE_SPLIT)
          {
          continue;
          }
        }
        // Turn object to get away from the door
        pObj->Dir += (short)((rand() % INT_ANGLE_180));
        if (pObj->Dir >= INT_ANGLE_360)
          pObj->Dir -= (short)INT_ANGLE_360;
        }
    else
      CheckFiring(nIndex,pObj);
    }
  }

MoveBombs();  // Move the active bombs

// This code determines if the player is in a special location
// of the game map and toggles the light shadding if the player
// reaches this region. This is used to create a unique
// dynamic light shading effect
nRow = (short)(ae->yPlayer >> 6);
nCol = (short)(ae->xPlayer >> 6);
if (nRow == 33)
  {
  if (nCol == 8)
    LightOnOff(0);
  else
  if (nCol == 10)
    LightOnOff(1);
  }

if (nCol == 24)
  {
  if (nRow == 5)
    LightOnOff(0);
  else
  if (nRow == 7)
    LightOnOff(1);
  }
}

//****************************************************************************
// Fires off a bomb at the player.
//****************************************************************************
void ShootBomb(void)
{
  NEWOBJECT   *pBomb;

// Get the bomb to fire
pBomb = ae->ObjList[BOMB_END_INDEX];
if (pBomb == NULL)
  return;
if (pBomb->Active)
  return;

// Set the bomb up so that it will come toward the player
pBomb->Dir = ae->PlayerAngle;
pBomb->x = ae->xPlayer;
pBomb->y = ae->yPlayer;
pBomb->Speed = 32;
pBomb->Active = 1;
pBomb->mPos = (short)((pBomb->y & 0xFFC0) + (pBomb->x >> (short)6));
}

//****************************************************************************
// Checks the status of the game and detrmines if the the game should be
// timed out or if the player has won.
//****************************************************************************
void CheckStatus(HWND hWnd)
{
  BOOL    bResult = FALSE;
  short   nPos;

if (bInStatus)
  return;
bInStatus = TRUE;

if (!nSecondsLeft)
  {
  nInfoMode = IM_TIMEOUT;
  DoInfoDlg(hInstGlobal,hWnd);
  bResult = TRUE;
  }

if (nRepairObjectCount == REPAIR_OBJECTS_NEEDED)
  {
  nPos = (short)((ae->yPlayer & 0xFFC0) + (ae->xPlayer >> 6));
  if (nPos == REPAIR_POSITION)
    {
    nInfoMode = IM_WON;
    DoInfoDlg(hInstGlobal,hWnd);
    bResult = TRUE;
    }
  }

if (bResult)
  {
  PostQuitMessage(0);
  }
bInStatus = FALSE;
}

//****************************************************************************
// Displays the current scene in the opened window.
//****************************************************************************
#pragma argsused
void AckPaint(HWND hWnd,HDC hdc)
{
  short   nHours,nMin,nSec,t;
  char    szBuf[40];

if (ae != NULL && nIntroCount == 0)
  {
  AckCheckObjectMovement();   // Check object animation
  MoveObjects();              // Application object movement
  AckBuildView();             // Render the 3D view
  }

// Check for any introduction graphics to display
if (nIntroCount)
  {
  /*if (pIntroBuf != NULL && AckBuffer != NULL)
    {
    memcpy(pAckBuffer,&pIntroBuf[4],64000);
    AckFree(pIntroBuf);
    pIntroBuf = NULL;
    }
  nIntroCount--;*/
  nIntroCount = 0;  //Segfaults on memcpy(), and I don't want this anyway.
  }
// Let WinG display the buffer into our window. We can use a StretchFactor
// to display larger windows if so desired, at the cost of speed. The
// fastest way is to simply BitBlt the buffer into our window.
//HBITMAP hbm = CreateBitmap(320, 200, 1, 8, ae->ScreenBuffer);
//SelectObject(AckBuffer, hbm);
//PatBlt(AckBuffer, 0,0,nBitmapW,nBitmapH, WHITENESS);
/*for(int i = 0; i < PAL_SIZE; i++)
{
    ae->ScreenBuffer[i] = ae->PalTable[i];
}*/

//Fill in our screenbuffer from ACK's buffer
unsigned char* pBufferData = OffscreenBuffer;
for(int r = 0; r < 320; r++)
{
    for(int c = 0; c < 200; c++) //AHAHA JOKE
    {
        *pBufferData++ = pallette[ae->ScreenBuffer[c+200*r]].r;
        *pBufferData++ = pallette[ae->ScreenBuffer[c+200*r]].g;
        *pBufferData++ = pallette[ae->ScreenBuffer[c+200*r]].b;
        *pBufferData++ = 255; //Alpha = 255 by default. You could probably do crazy stuff with this, but I'm not going to.
    }
}
//Create our offscreen HDC if we haven't yet
if(!hOffscreenDC)
{
    hOffscreenDC = CreateCompatibleDC(hdc);
    hOffscreenBitmap = CreateCompatibleBitmap(hdc, 320, 200);
    SelectObject(hOffscreenDC, hOffscreenBitmap);
}
//PatBlt(hOffscreenDC, 0,0,nBitmapW,nBitmapH, BLACKNESS);
HICON ic = CreateIcon(hInstGlobal, 320, 200, 1, 32, NULL, (BYTE*)(OffscreenBuffer));
if(ic == NULL)
    printf("Icon NULL\n");
DrawIconEx(hOffscreenDC, 0, 0, ic, 320, 200, NULL, NULL, DI_NORMAL);
//DrawIcon(hOffscreenDC, 0, 0, LoadIcon(NULL, IDI_APPLICATION));
//BitBlt(hdc,0,0,nBitmapW,nBitmapH,hOffscreenDC,0,0,SRCCOPY);
//PatBlt(AckBuffer, 0,0,nBitmapW,nBitmapH, BLACKNESS);
if (StretchFactor != 1)
  {
  StretchBlt(hdc,0,0,nWindowW,nWindowH, // Use the current window size
                   hOffscreenDC,0,0,nBitmapW,nBitmapH,SRCCOPY);
  }
else
  BitBlt(hdc,0,0,nBitmapW,nBitmapH,hOffscreenDC,0,0,SRCCOPY);
DestroyIcon(ic);
if (bUpdateTime)
  {
  bUpdateTime = FALSE;
  nHours = (short)(nSecondsLeft / 3600);
  t = (short)(nHours * 3600);
  nMin = (short)((nSecondsLeft - t) / 60);
  nSec = (short)(nSecondsLeft - t - (nMin * 60));
  wsprintf(szBuf,"Station Escape - %02d:%02d:%02d",nHours,nMin,nSec);
  SetWindowText(hWnd,szBuf);
  }
}

//****************************************************************************
// Adjust the actual window size to account for borders and a caption bar
//****************************************************************************
void SetNewWindowSize(HWND hWnd)
{
  int     windx,windy;

windx = windy = GetSystemMetrics(SM_CXBORDER)*2;
nWindowW += windx;
nWindowH += windy + GetSystemMetrics(SM_CYCAPTION) - 1;
SetWindowPos(hWnd,NULL,0,0,nWindowW,nWindowH,
             SWP_NOMOVE|SWP_NOZORDER|SWP_NOCOPYBITS);
}

//****************************************************************************
// Message handler for the 3D window
//****************************************************************************
LRESULT CALLBACK _export AckWndProc(HWND hWnd,UINT iMessage,
                                    WPARAM wParam,LPARAM lParam)
{
  HDC         hDC;
  PAINTSTRUCT ps;

switch (iMessage)
  {
  case WM_CREATE:
    nTimerID = SetTimer(hWnd,1,1000,NULL);
    break;

  case WM_DESTROY:
    if (nTimerID)
      KillTimer(hWnd,nTimerID);
    nTimerID = 0;
    PostQuitMessage(0);
    break;

  case WM_TIMER:
    nTimerCount++;
    bUpdateTime = TRUE;
    if (nSecondsLeft)
      nSecondsLeft--;
    CheckStatus(hWnd);
    break;

  case WM_SIZE:
    AckDoSize(hWnd,lParam);
    break;

  case WM_PAINT:
    BeginPaint(hWnd,&ps);
    AckPaint(hWnd,ps.hdc);
    EndPaint(hWnd,&ps);
    break;

  case WM_KEYDOWN:
    if (wParam == VK_F1)
      {
      nInfoMode = IM_HELP;
      DoInfoDlg(hInstGlobal,hWnd);
      break;
      }

    if (wParam == VK_F2)
      {
      ae->SysFlags ^= SYS_SOLID_FLOOR;
      AckRegisterStructure(ae);
      break;
      }

    if (wParam == VK_F3)
      {
      ae->SysFlags ^= SYS_SOLID_CEIL;
      AckRegisterStructure(ae);
      break;
      }

    if (wParam == VK_TAB)
      {
      ShootBomb();
      break;
      }

// Fall through here
  case WM_KEYUP:
    nIntroCount = 0;
    ProcessKeys(iMessage,wParam);
    break;

  case WM_RBUTTONDOWN:
    AckCheckDoorOpen(ae->xPlayer,ae->yPlayer,ae->PlayerAngle);
    break;

  case WM_LBUTTONDOWN:
    nIntroCount = 0;
    ProcessMouse(hWnd,1,LOWORD(lParam),HIWORD(lParam));
    SetCapture(hWnd);
    break;

  case WM_MOUSEMOVE:
    if (wParam & MK_LBUTTON)
      ProcessMouse(hWnd,1,LOWORD(lParam),HIWORD(lParam));
    break;

  case WM_LBUTTONUP:
    ProcessMouse(hWnd,0,LOWORD(lParam),HIWORD(lParam));
    ReleaseCapture();
    break;

  case WM_PALETTECHANGED:
    if (hWnd == (HWND)wParam)
      break;
    // Fall through here
  case WM_QUERYNEWPALETTE:
    hDC = GetDC(hWnd);
    if (hpalApp)
      SelectPalette(hDC,hpalApp,FALSE);

    RealizePalette(hDC);
    ReleaseDC(hWnd,hDC);
    return FALSE;

  case WM_SYSCOMMAND:
    switch (wParam)
      {
      case IDM_RES1:
        if (StretchFactor != 1)
          {
          StretchFactor = 1;
          nWindowW = 320;
          nWindowH = 200;
          SetNewWindowSize(hWnd);
          }
        break;

      case IDM_RES2:
        if (StretchFactor != 2)
          {
          StretchFactor = 2;
          nWindowW = 480;
          nWindowH = 300;
          SetNewWindowSize(hWnd);
          }
          break;

      case IDM_RES3:
        if (StretchFactor != 3)
          {
          StretchFactor = 3;
          nWindowW = 640;
          nWindowH = 400;
          SetNewWindowSize(hWnd);
          }
          break;
      case IDM_RES4:
        if (StretchFactor != 4)
          {
          StretchFactor = 4;
          nWindowW = 800;
          nWindowH = 600;
          SetNewWindowSize(hWnd);
          }
          break;

      case IDM_RES5:
        if (StretchFactor != 5)
          {
          StretchFactor = 5;
          nWindowW = 1280;
          nWindowH = 800;
          SetNewWindowSize(hWnd);
          }
          break;

      case IDM_HELP:
        nInfoMode = IM_HELP;
        DoInfoDlg(hInstGlobal,hWnd);
        break;

      case IDM_BACK:
        nInfoMode = IM_BACK;
        DoInfoDlg(hInstGlobal,hWnd);
        break;

      default:
        break;
      }
    return DefWindowProc( hWnd, iMessage, wParam, lParam );

  default:
    return DefWindowProc( hWnd, iMessage, wParam, lParam );
  }
return 0;
}

void ReadInPallette()
{
    std::ifstream infile("pallette.txt");
    if(infile.fail())
        return;
//printf("reading in pallette\n");
    for(int i = 0; i < 256; i++)
    {
        unsigned int r, g, b;
        PalletteSlot p = {0, 0, 0};
        infile >> r >> g >> b;
        p.r = (unsigned char)b; //Flip blue and red, because Windows wants the icon data in BGRA format
        p.g = (unsigned char)g;
        p.b = (unsigned char)r;
        p.a = 255;
        //printf("%d, %d, %d\n", p.r, p.g, p.b);
        pallette[i] = p;
    }
    infile.close();
}









//**** End of Source ****

