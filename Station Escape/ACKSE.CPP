// Example source for using ACK-3D in Windows
// (c) 1995 ACK Software (Lary Myers)
//#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "ackwin.h"
#include "ackse.h"
#include <stdio.h>
#include <fstream> //MEH for file streams. Much easier for me to use
//MEH Use SDL now! Yay!
#ifdef __APPLE__
#include <SDL/SDL.h>
#include <SDL/SDL_rotozoom.h>
#else
#include <SDL.h>
#include <SDL_rotozoom.h>
#endif

/*#define IDM_RES1    1100
#define IDM_RES2    1101
#define IDM_RES3    1102
//#define IDM_HELP    1103
//#define IDM_BACK    1104
#define IDM_RES4    1105
#define IDM_RES5    1106*/

#include "ack3d.h"
extern "C" {
#include "keys.h"
void AckDoubleBuffer(UCHAR *Src,UCHAR *Dest);
extern  UCHAR   colordat[];
};

// Some globals to use
SDL_Surface* MainScreen;
ACKENG      *ae = 0;        // Interface structure
//HWND        hwndApp;
UCHAR       kbPressed[128];
void        *pAckBuffer = 0;
//HDC         AckBuffer = 0;
//HPALETTE    hpalApp;
//RGBQUAD     ColorTable[256];
//HBITMAP     gbmOldMonoBitmap = 0;
int         StretchFactor = 1;
//int         nBitmapW;
//int         nBitmapH;        // WinG bitmap dimensions.
int         nWindowW;
int         nWindowH;
//short       nIntroCount;
short       nInfoMode;
//HINSTANCE   hInstGlobal;
bool        bNoShow;        // Do not display if true
bool        bInStatus;      // Recursive flag for status checks
//UCHAR       *pIntroBuf;
short       nSecondsLeft;
//bool        bUpdateTime;
UINT        nTimerID;
//WORD        nTimerCount;
short       nRepairObjectCount;
UCHAR       SavePal[768];

ColorRange  ranges[64] = {
    {16,16},
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
    {184,8},
    {192,16},
    {208,16},
    {224,16},
    {240,15},
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
   unsigned char* OffscreenBuffer; //For holding our offscreen buffer data
   #define  ACK3D_SCREENWIDTH    320
   #define  ACK3D_SCREENHEIGHT   200
   #define  BYTES_PER_PIXEL     4

   SDL_Rect rcResolution;

//HDC hOffscreenDC = 0;
//HBITMAP hOffscreenBitmap;

//****************************************************************************
// The main program window
//****************************************************************************
/*int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)*/
#undef main
int main(int argc, char* argv[])
{
  int     result;
//  HBITMAP hbm;
  char    szBuf[40];

    //Initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    //Grab the maximum supported screen resolution
    SDL_Rect **modes;
    modes=SDL_ListModes(NULL, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);

    if(modes == (SDL_Rect **)0){
      printf("No modes available for fullscreen\n");
      return -1;
    }

    rcResolution.w = 800;
    rcResolution.h = 600;     //800x600 is reasonable default

    if(modes != (SDL_Rect **)-1)    //And if it supports all resolutions, use 800x600
    {
        //Find the lowest resolution above or at 320x200
        int iNumModes = 0;
        for(int i = 0; modes[i]; i++)
            iNumModes++;
        for(int i = iNumModes-1; i >= 0; i--)
        {
            if(modes[i]->w >= 320 && modes[i]->h >= 200)
            {
                rcResolution.w = modes[i]->w;
                rcResolution.h = modes[i]->h;
                break;  //Done, found one larger than or equal to 320x200
            }
        }
    }

    printf("Resolution: %dx%d\n", rcResolution.w, rcResolution.h);
    //Create our handle to the screen
    MainScreen = SDL_SetVideoMode(rcResolution.w, rcResolution.h, 32, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
    if ( !MainScreen )
    {
        printf("Unable to set video resolution: %s\n", SDL_GetError());
        return 1;
    }

  //Allocate memory for our offscreen buffer
  OffscreenBuffer = (unsigned char*)malloc(ACK3D_SCREENWIDTH * ACK3D_SCREENHEIGHT * BYTES_PER_PIXEL);

//pIntroBuf = NULL;
// Register our Window class if this is the first time in
//if (!hPrevInstance)
//  if (!Register(hThisInstance))
//    return(-1);

//MEH Hide the cursor and set it to the center of the viewport window
SDL_WarpMouse(rcResolution.w / 2, rcResolution.h / 2);
SDL_ShowCursor(SDL_DISABLE);

//hInstGlobal = hThisInstance;

//---------------Begin ACK-3D specific, rather than Windows-specific, code
// Prime the ACK-3D engine with our startup parameters
if (InitACKEngine())
  return(-1);

// Open our default resource file and load bitmaps, etc.
AckOpenResource("pics.dtf");
result = AckProcessInfoFile();
if (result)
  {
  printf("ACK-3D Error: %d in resource file",result);
  AckCloseResource();
  return(-1);
  }
LoadBackDrop(); //MEH WHY WASN'T THIS CALLED HERE??!?! RGH
AckCloseResource();
AckSetupPalRanges(ae, ranges);
//---------------MEH End ACK-3D - specific code
ReadInPallette();//MEH read in the pallette data from "pallette.txt"

srand(SDL_GetTicks());

//bUpdateTime = false;
//nTimerCount = 0;
nRepairObjectCount = 0;
nSecondsLeft = (60 * 60) + (60 * 30);


// Create the actual window to display the rendered view
//if (!CreateAll(hThisInstance))
//  return(-1);

//nIntroCount = 0;

// Here we loop until the application closes
result = MessageLoop();

//Kill all the ACK-3D buffers and stuffz
AckWrapUp(ae);

// Free up our buffers we used from WinG
/*if (AckBuffer)
  {
  hbm = (HBITMAP)SelectObject(AckBuffer, gbmOldMonoBitmap);
  DeleteObject(hbm);
  DeleteDC(AckBuffer);
  }*/

//DeleteObject(hOffscreenDC);
//DeleteObject(hOffscreenBitmap);

//MEH Show cursor again
SDL_ShowCursor(SDL_ENABLE);

//Delete allocated memory for offscreen buffer
free(OffscreenBuffer);

return(result);
}

//****************************************************************************
// Registers the window class.
//****************************************************************************
/*bool Register(HINSTANCE hInst)
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
  return(false);
return(true);
}*/

//****************************************************************************
// Loads the WinG DLL and creates the display window for the application.
//****************************************************************************
/*bool CreateAll(HINSTANCE hInst)
{
  int     windx, windy;
  HMENU   hMenu;

//nBitmapW = 319;                 // Default size (window client size to use)
//nBitmapH = 199;

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
  return(false);
  }

// Now we'll display the window and setup the WinG buffer when we
// receive the WM_SIZE message.
ShowWindow(hwndApp,SW_NORMAL);
hMenu = GetSystemMenu(hwndApp,false);
// Add in the menu items for the game window
AppendMenu(hMenu,MF_SEPARATOR,0,NULL);
AppendMenu(hMenu,MF_STRING,IDM_RES1,"320 x 200");
AppendMenu(hMenu,MF_STRING,IDM_RES2,"480 x 300");
AppendMenu(hMenu,MF_STRING,IDM_RES3,"640 x 400");
AppendMenu(hMenu,MF_STRING,IDM_RES4,"800 x 600");
AppendMenu(hMenu,MF_STRING,IDM_RES5,"1280 x 800");
return(true);
}*/

//****************************************************************************
// Here is where we loop until the application terminates.
//****************************************************************************
int MessageLoop(void)
{
    int iCurTicks = SDL_GetTicks();
    SDL_Event event;

for (;;)
  {
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
        break;

    //See if game should end
    if(CheckStatus())
        break;

    //Also exit if pressing Escape
    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        break;

    //Process this event
    AckWndProc(event);

    //MEH Enough of this huge framerate! Time for limited fps
    int iNewTick = SDL_GetTicks();
    if(iNewTick - iCurTicks > 30) //~30 fps
    {
        iCurTicks = iNewTick;
        DoCycle();  // Display the 3D window
    }
  }
return 0;
}

//****************************************************************************
// Turns light shading on and off.
//****************************************************************************
void LightOnOff(short nMode)
{
  bool    bSet = false;

if (!nMode && ae->LightFlag == SHADING_OFF)
  {
  ae->LightFlag = SHADING_ON;
  bSet = true;
  }

if (nMode && ae->LightFlag == SHADING_ON)
  {
  ae->LightFlag = SHADING_OFF;
  bSet = true;
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
// Checks the status of the game and determines if the the game should be
// timed out or if the player has won. Returns true if game should end.
//****************************************************************************
bool CheckStatus()
{
  short   nPos;
  static int iCurTick = SDL_GetTicks();
  int iLastTick;

if (bInStatus)
  return false; //Keep going
bInStatus = true;

//See if another second has elapsed
iLastTick = iCurTick;
iCurTick = SDL_GetTicks();
if(iCurTick - iLastTick >= 1000)    //Another second has elapsed
{
    if (nSecondsLeft)
      nSecondsLeft--;   //Decrement seconds
    iCurTick += (iLastTick - iCurTick); //Hang on to how much we went over
}

if (!nSecondsLeft)
  {
  printf("Game over. Ran out of time.\n");
  return true;
  }

if (nRepairObjectCount == REPAIR_OBJECTS_NEEDED)
  {
  nPos = (short)((ae->yPlayer & 0xFFC0) + (ae->xPlayer >> 6));
  if (nPos == REPAIR_POSITION)
    {
    printf("You win! YAY!\n");
    return true;
    }
  }
  bInStatus = false;
return false;       //Keep going
}

//****************************************************************************
// Displays the current scene in the opened window.
//****************************************************************************
//#pragma argsused
void AckPaint()
{
  short   nHours,nMin,nSec,t;
  char    szBuf[40];

if (ae != NULL)
  {
  AckCheckObjectMovement();   // Check object animation
  MoveObjects();              // Application object movement
  AckBuildView();             // Render the 3D view
  }

// Check for any introduction graphics to display
//if (nIntroCount)
//  {
//  nIntroCount = 0;  //Segfaults on memcpy(), and I don't want this anyway.
//  }

//Create our offscreen HDC if we haven't yet
/*if(!hOffscreenDC)
{
    hOffscreenDC = CreateCompatibleDC(hdc);
    hOffscreenBitmap = CreateCompatibleBitmap(hdc, 320, 200);
    SelectObject(hOffscreenDC, hOffscreenBitmap);
}*/

/*if (StretchFactor != 1)
  {
    //Fill in our screenbuffer from ACK's buffer
    unsigned char* pBufferData = OffscreenBuffer;

    //Try out my stretch function instead of StretchBlt()
    unsigned char* cTempImg = (unsigned char*)malloc(ptCurResolution.y * ptCurResolution.x);    //Create offscreen buffer large enough to hold the image result
    //Stretch the image
    StretchImage(ae->ScreenBuffer, cTempImg, 320, 200, ptCurResolution.x, ptCurResolution.y);

    for(int r = 0; r < ptCurResolution.x; r++)
    {
        for(int c = 0; c < ptCurResolution.y; c++)
        {
            *pBufferData++ = pallette[cTempImg[c+ptCurResolution.y*r]].r;
            *pBufferData++ = pallette[cTempImg[c+ptCurResolution.y*r]].g;
            *pBufferData++ = pallette[cTempImg[c+ptCurResolution.y*r]].b;
            *pBufferData++ = 255; //Alpha = 255 by default. You could probably do crazy stuff with this, but I'm not going to.
        }
    }

    //Create icon and draw it
    HICON ic = CreateIcon(hInstGlobal, ptCurResolution.x, ptCurResolution.y, 1, 32, NULL, (BYTE*)(OffscreenBuffer));
    if(ic == NULL)
        printf("Icon NULL\n");
    else
        DrawIconEx(hdc, 0, 0, ic, ptCurResolution.x, ptCurResolution.y, NULL, NULL, DI_NORMAL);

    //Free our memory
    free(cTempImg);
    if(ic != NULL)
        DestroyIcon(ic);
  }
else
{*/
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

    Uint32 rmask, gmask, bmask, amask;

    // SDL interprets each pixel as a 32-bit number, so our masks must depend on the endianness (byte order) of the machine
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
    #else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
    #endif
    SDL_Surface* tempSurface = SDL_CreateRGBSurface(MainScreen->flags, 320, 200, 32, rmask, gmask, bmask, amask);

    SDL_LockSurface(tempSurface);

    //Copy over
    for(int r = 0; r < 320 * 200 * 4; r++)
    {
        ((unsigned char*)tempSurface->pixels)[r] = OffscreenBuffer[r];
    }

    SDL_UnlockSurface(tempSurface);

    //Use SDL_rotozoom to fill screen
    SDL_Surface* tempZoomed = zoomSurface(tempSurface,
                                          (float)(rcResolution.w) / (float)(ACK3D_SCREENWIDTH),
                                          (float)(rcResolution.h) / (float)(ACK3D_SCREENHEIGHT),
                                          SMOOTHING_OFF);


    /*SDL_Surface* bmp = SDL_LoadBMP("cb.bmp");
    if (!bmp)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
    }*/

    // centre the bitmap on screen
    SDL_Rect dstrect;
    dstrect.x = 0;//(rcResolution.w - tempSurface->w) / 2;
    dstrect.y = 0;//(rcResolution.h - tempSurface->h) / 2;

    SDL_BlitSurface(tempZoomed, 0, MainScreen, &dstrect);  //draw the temp surface to the screen

    /*HICON ic = CreateIcon(hInstGlobal, 320, 200, 1, 32, NULL, (BYTE*)(OffscreenBuffer));
    if(ic == NULL)
        printf("Icon NULL\n");
    DrawIconEx(hOffscreenDC, 0, 0, ic, 320, 200, NULL, NULL, DI_NORMAL);
    if(StretchFactor == 1)
        BitBlt(hdc,0,0,nBitmapW,nBitmapH,hOffscreenDC,0,0,SRCCOPY);
    else
        StretchBlt(hdc,0,0,ptCurResolution.x,ptCurResolution.y,hOffscreenDC,0,0,nBitmapW,nBitmapH,SRCCOPY);
    DestroyIcon(ic);*/

//if (bUpdateTime)
//  {
//  bUpdateTime = false;
  nHours = (short)(nSecondsLeft / 3600);
  t = (short)(nHours * 3600);
  nMin = (short)((nSecondsLeft - t) / 60);
  nSec = (short)(nSecondsLeft - t - (nMin * 60));
  sprintf(szBuf,"Station Escape - %02d:%02d:%02d",nHours,nMin,nSec);
  //SetWindowText(hWnd,szBuf);
//  }
SDL_Flip(MainScreen);   //Flip the double-buffered window to the screen
SDL_FreeSurface(tempSurface);
SDL_FreeSurface(tempZoomed);
}

//****************************************************************************
// Adjust the actual window size to account for borders and a caption bar
//****************************************************************************
/*void SetNewWindowSize(HWND hWnd)
{
  int     windx,windy;
ptCurResolution.x = nWindowW;
ptCurResolution.y = nWindowH;
windx = windy = GetSystemMetrics(SM_CXBORDER)*2;
nWindowW += windx;
nWindowH += windy + GetSystemMetrics(SM_CYCAPTION) - 1;
SetWindowPos(hWnd,NULL,0,0,nWindowW,nWindowH,
             SWP_NOMOVE|SWP_NOZORDER|SWP_NOCOPYBITS);
}*/

//****************************************************************************
// Message handler for the 3D window
//****************************************************************************
void AckWndProc( SDL_Event event )
{
  //HDC         hDC;
  //PAINTSTRUCT ps;
//  POINT       ptCursor;

switch (event.type)
  {
  /*case WM_CREATE:
    nTimerID = SetTimer(hWnd,1,1000,NULL);
    break;

  case WM_DESTROY:
    if (nTimerID)
      KillTimer(hWnd,nTimerID);
    nTimerID = 0;
    PostQuitMessage(0);
    break;

  case WM_TIMER:
//    nTimerCount++;
//    bUpdateTime = true;
    if (nSecondsLeft)
      nSecondsLeft--;
    CheckStatus();
    break;

  case WM_SIZE:
    AckDoSize(hWnd,lParam);
    break;

  case WM_PAINT:
    BeginPaint(hWnd,&ps);
    AckPaint(hWnd,ps.hdc);
    EndPaint(hWnd,&ps);
    break;*/

  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_TAB)
      {
      ShootBomb();
      break;
      }

    //MEH End game if pressing Esc
    /*if(wParam == VK_ESCAPE)   //elsewhere
    {
        if (nTimerID)
            KillTimer(hWnd,nTimerID);
        nTimerID = 0;
        PostQuitMessage(0);
    }*/

// Fall through here
  case SDL_KEYUP:
//    nIntroCount = 0;
    ProcessKeys(event);
    break;

  case SDL_MOUSEBUTTONDOWN:
    if(event.button.button == SDL_BUTTON_RIGHT)
        AckCheckDoorOpen(ae->xPlayer,ae->yPlayer,ae->PlayerAngle);
    if(event.button.button == SDL_BUTTON_LEFT)
        ShootBomb();
    break;

  /*case WM_LBUTTONDOWN:
    ShootBomb();
    break;

  case WM_MOUSEMOVE:
    break;

  case WM_LBUTTONUP:
    break;

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

//      case IDM_HELP:
//        nInfoMode = IM_HELP;
//        DoInfoDlg(hInstGlobal,hWnd);
//        break;

//      case IDM_BACK:
//        nInfoMode = IM_BACK;
//        DoInfoDlg(hInstGlobal,hWnd);
//        break;

      default:
        break;
      }
    return DefWindowProc( hWnd, iMessage, wParam, lParam );*/

  default:
    break;//return DefWindowProc( hWnd, iMessage, wParam, lParam );
  }
}

void ReadInPallette()
{
    std::ifstream infile("pallette.txt");
    if(infile.fail())
        return;
    for(int i = 0; i < 256; i++)
    {
        unsigned int r, g, b;
        PalletteSlot p = {0, 0, 0};
        infile >> r >> g >> b;
        p.r = (unsigned char)r;
        p.g = (unsigned char)g;
        p.b = (unsigned char)b;
        p.a = 255;
        pallette[i] = p;
    }
    infile.close();
}

/*
#include <fstream>
using std::endl;
using std::ofstream;
//MEH stretch this image onto a larger one - WAY TOO SLOW
void StretchImage(unsigned char* src, unsigned char* dest,
                  unsigned int srcW, unsigned int srcH, unsigned int destW, unsigned int destH)
{
    ofstream ofile("output.txt");
    ofile << "w: " << destW << ", h: " << destH << endl;
    //Do some sanity checking first
    if(srcW > destW ||
       srcH > destH)
    {
        printf("Error in StretchImage(): Cannot stretch an image smaller than the original.\n");
        return;
    }

    if(srcW == 0 ||
       srcH == 0 ||
       destW == 0 ||
       destH == 0)
    {
        printf("Error in StretchImage(): Cannot stretch an image to/from 0 width or height.\n");
        return;
    }

    for(int i = 0; i < destH; i++)
    {
        for(int j = 0; j < destW; j++)
            dest[j + i*destW] = 0;
    }

    //Grab the target stretch factor x/y
    float fTargetStretchX = (float)(destW) / (float)(srcW);
    float fTargetStretchY = (float)(destH) / (float)(srcH);

    ofile << "stretchX: " << fTargetStretchX << " stretchY: " << fTargetStretchY << endl;

    //Our running totals of where we're at
    float fCurrentX = fTargetStretchX;
    float fCurrentY = fTargetStretchY;

    unsigned char* pCurrentDest = dest; //Where on the destination image we are
    unsigned char* pCurrentSrc = src;

    unsigned int iWhereAtY = 0;
    //unsigned int iWhereAtX = 0;
    int iTemp = 0;

    //Loop through the source image, copying what we can into the dest image
    for(int ySrc = 0; ySrc < srcH; ySrc++)  //Loop through y first
    {
        //if((int)(pCurrentDest - dest) > 100000 && iTemp < 5)
        //{
            ofile << "y: " << ySrc << endl;
            ofile << "destpos: " << (int)(pCurrentDest - dest) << endl;
            iTemp++;
        //}
        fCurrentX = fTargetStretchX;    //Reset our current x
        //pCurrentDest = dest + iWhereAtY * destW;    //and our current x dest
        for(int xSrc = 0; xSrc < srcW; xSrc++)  //Then loop through x
        {
            int iCurrentX = fCurrentX;  //No interpolation means we only deal with integer values per pixel
            //Loop through how many pixels we should write for one pixel of this row
            for(int i = 0; i < iCurrentX; i++)
            {
                //if((int)(pCurrentDest - dest) > 100000 && iTemp < 5)
                //{
                    //ofile << xSrc << ", " << ySrc << "xscale: " << fTargetStretchX << " yscale: " << fTargetStretchY << endl;
                    //ofile << (int)(pCurrentDest - dest) << endl;
                //}
                *pCurrentDest = *pCurrentSrc;
                pCurrentDest++;
            }
            pCurrentSrc++;  //Move to next pixel in source
            fCurrentX -= iCurrentX; //Remove the pixels we wrote
            fCurrentX += fTargetStretchX;   //Add back in for next pass. Fractional parts should add up to all work out
        }

        //Copy this past row in the vertical direction yrepeat times
        //pCurrentDest = dest + (iWhereAtY + 1) * destW;
        int iCurrentY = fCurrentY;
        for(int j = 0; j < iCurrentY - 1; j++)
        {

            for(int i = 0; i < destW; i++)
            {
                *pCurrentDest = *(pCurrentDest - destW);
                pCurrentDest++;
            }

        }
        //Add back in our stretch leftover stuff
        fCurrentY -= iCurrentY;
        fCurrentY += fTargetStretchY;
        //iWhereAtY++;
    }

}*/








//**** End of Source ****

