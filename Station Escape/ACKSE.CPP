// Example source for using ACK-3D in Windows
// (c) 1995 ACK Software (Lary Myers)
#include <stdlib.h>
#include <string.h>
#include "ACKSE.H"
#include <stdio.h>
#include <fstream> //MEH for file streams. Much easier for me to use
//MEH Use SDL now! Yay!
#ifdef __APPLE__
#include <SDL/SDL.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxPrimitives.h>
#else
#include <SDL.h>
#include <SDL_rotozoom.h>
#include <SDL_gfxPrimitives.h>
#endif

#include "KEYS.H"
void AckDoubleBuffer(UCHAR *Src,UCHAR *Dest);
extern  UCHAR   colordat[];

// Some globals to use
SDL_Surface* MainScreen;
UCHAR       kbPressed[128];
int         StretchFactor = 1;
int         nWindowW;
int         nWindowH;
short       nInfoMode;
bool        bNoShow;        // Do not display if true
bool        bInStatus;      // Recursive flag for status checks
short       nSecondsLeft;
UINT        nTimerID;
short       nRepairObjectCount;
UCHAR       SavePal[768];

//MEH Ack wrapper class variable
AckWrapper* g_Ack3D;

// Keeps the list of movable objects
short   nMoveObjectList[] = {
  6,7,8,9,10,11,12,
  18,19,
  0}; // Denotes end of table

  //MEH 24-bit screen buffer, for Win32 reasons
//   unsigned char* OffscreenBuffer; //For holding our offscreen buffer data
   #define  ACK3D_SCREENWIDTH   320
   #define  ACK3D_SCREENHEIGHT  200
   #define  BYTES_PER_PIXEL     4

   #define  START_REPAIR_OBJECT 20
   #define  END_REPAIR_OBJECT   22

   SDL_Rect rcResolution;

   bool bHurtThisCycle; //If the player has been hit this cycle, flash the screen red


   //Global variables to deal with health
   #define MAX_HEALTH           320
   #define HEALTH_PLASMA_HIT    5
   #define HEALTH_SPACEMAN_HIT  10

   int g_iHealth = MAX_HEALTH;

//****************************************************************************
// The main program window
//****************************************************************************
#undef main
int main(int argc, char* argv[])
{
    int     result;

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

    //printf("Resolution: %dx%d\n", rcResolution.w, rcResolution.h);
    //Create our handle to the screen
    MainScreen = SDL_SetVideoMode(rcResolution.w, rcResolution.h, 32, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
    if ( !MainScreen )
    {
        printf("Unable to set video resolution: %s\n", SDL_GetError());
        return 1;
    }

//MEH Hide the cursor and set it to the center of the viewport window
SDL_WarpMouse(rcResolution.w / 2, rcResolution.h / 2);
SDL_ShowCursor(SDL_DISABLE);

//---------------MEH Begin ACK-3D specific, rather than SDL-specific, code
g_Ack3D = new AckWrapper("pics.dtf");
g_Ack3D->AssignObjectCallback(ObjectCollision); //Set our collision callback function
//---------------MEH End ACK-3D - specific code

srand(SDL_GetTicks());

nRepairObjectCount = 0;
nSecondsLeft = (60 * 60) + (60 * 30);

// Here we loop until the application closes
result = MessageLoop();

//MEH Show cursor again
SDL_ShowCursor(SDL_ENABLE);

//Clean up the ACK-3D engine
delete g_Ack3D;

return(result);
}

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
        DoCycle();  // Display the 3D window, check key presses, etc
    }
  }
return 0;
}

//****************************************************************************
// Our collision callback - called when an object moves each cycle
//****************************************************************************
void ObjectCollision(NEWOBJECT* pObj, UCHAR nIndex, short nResult)
{
    //First test: see if it's a spaceman
    bool bSpaceman = false;
    for(int i = 0; nMoveObjectList[i]; i++)
    {
        if(nMoveObjectList[i] == nIndex)
        {
            bSpaceman = true;
            break;
        }
    }
    if(bSpaceman)
    {
        //FOLLOW PLAYER! AHAHA
        short dx = ae->xPlayer - pObj->x;
        short dy = ae->yPlayer - pObj->y;
        short dir = AckGetObjectAngle(dx,dy);
        //But only if within two tile widths
        if(sqrt(dx * dx + dy * dy) > GRID_WIDTH * 2)
            pObj->Dir = dir;
        if(sqrt(dx * dx + dy * dy) < GRID_WIDTH / 2)    //If close to player, bounce away and hurt player
        {
            pObj->Dir = WrapAngle(dir - INT_ANGLE_180);
            HurtPlayer(HEALTH_SPACEMAN_HIT);
        }
        //If it is a spaceman, and he hit something
        if (nResult != POV_NOTHING)
        {
            if(nResult == POV_XWALL)
            {
                //Just slide off walls
            }
            if(nResult == POV_YWALL)
            {

            }
            if(nResult == POV_OBJECT)
            {
                //Turn around (bounce off objects)
                pObj->Dir += INT_ANGLE_180;
                if (pObj->Dir >= INT_ANGLE_360)
                    pObj->Dir -= (short)INT_ANGLE_360;
            }
        }
        //Make the spaceman fire if he's not within a couple tile widths of player (Firing point blank kinda screws the system)
        if(sqrt(dx * dx + dy * dy) > GRID_WIDTH * 2.5)
            CheckFiring(nIndex,pObj);

    }

    //Check if it's a plasma bolt
    if(nIndex >= BOMB_START_INDEX && nIndex <= BOMB_END_INDEX)
    {
        if (nResult != POV_NOTHING)
        {
            if (nIndex == BOMB_END_INDEX && nResult != POV_PLAYER)
            {
                pObj->Active = 0;
                if (nResult == POV_OBJECT)
                    CheckBadGuys(AckGetObjectHit());
            }
            else
            {
                if(nIndex != BOMB_END_INDEX && nResult == POV_PLAYER)
                {
                    HurtPlayer(HEALTH_PLASMA_HIT);
                }
                int nHitIndex = 0;
                if (nResult == POV_OBJECT)
                    nHitIndex = AckGetObjectHit();
                if (nHitIndex != pObj->id)
                {
                    pObj->Active = 0;
                    NEWOBJECT* pSman = ae->ObjList[pObj->id];
                    if (pSman != NULL)
                        pSman->id = 0;
                    pObj->id = 0;
                }
            }
        }
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
// See if a bad guy is not currently firing a bomb. If so, set up the bomb
// for firing.
//****************************************************************************
void CheckFiring(short nObjIndex,NEWOBJECT *pObj)
{
  UCHAR   Index;
  NEWOBJECT   *pBomb;
  int32_t    dx,dy;

if (pObj->id)   // Bad guy is currently firing
  return;

Index = FindBombObject();  // Get the first inactive bomb

if (Index)
  {  // Set all of the attributes for firing the bomb
  pObj->id = Index;
  dx = ae->xPlayer - pObj->x;
  dy = ae->yPlayer - pObj->y;
  pBomb = ae->ObjList[Index];
  pBomb->x = pObj->x + ((dx > 0) ? (32) : (-32));
  pBomb->y = pObj->y + ((dy > 0) ? (32) : (-32));
  dx = ae->xPlayer - pBomb->x;
  dy = ae->yPlayer - pBomb->y;
  pBomb->Dir = AckGetObjectAngle(dx,dy);
  pBomb->id = nObjIndex;
  pBomb->Speed = 32;
  pBomb->Active = 1;
  pBomb->mPos = GetMapPosn(pBomb->x,pBomb->y);
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
pBomb->mPos = GetMapPosn(pBomb->x,pBomb->y);
}

//****************************************************************************
// Checks the status of the game and determines if the the game should be
// timed out or if the player has won. Returns true if game should end.
//****************************************************************************
bool CheckStatus()
{
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
  if (GetMapPosn(ae->xPlayer, ae->yPlayer) == REPAIR_POSITION)
    {
    printf("You win! YAY!\n");
    return true;
    }
  }

  //Check and see if player ran out of health (died)
  if(g_iHealth <= 0)
  {
      printf("The spacemen gloat as they point their guns at your head and blast your life away.\nYou have died...\n");
      return true;      //game over
  }

  bInStatus = false;
return false;       //Keep going
}

//****************************************************************************
// Displays the current scene in the opened window.
//****************************************************************************
void AckPaint()
{
    //Fill in our screenbuffer from ACK's buffer
    const unsigned char* pBufferData = g_Ack3D->RenderView();

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
        ((unsigned char*)tempSurface->pixels)[r] = pBufferData[r];
    }

    SDL_UnlockSurface(tempSurface);

    //Draw health left in form of a green/yellow/red bar at bottom of screen
    int r = 0;
    int g = 255;    //Green if above half health
    int b = 0;

    //Yellow if less than half health
    if(g_iHealth < MAX_HEALTH / 2)
    {
        r = 255;
        g = 255;
        b = 0;
    }
    //Red if less than 1/5th health
    if(g_iHealth < MAX_HEALTH / 5)
    {
        r = 255;
        g = 0;
        b = 0;
    }

    //Draw the rectangle, 5 pixels tall
    boxRGBA(tempSurface, 0, 195, g_iHealth, 200, r, g, b, 128);

    //Use SDL_rotozoom to fill screen
    SDL_Surface* tempZoomed = zoomSurface(tempSurface,
                                          (float)(rcResolution.w) / (float)(ACK3D_SCREENWIDTH),
                                          (float)(rcResolution.h) / (float)(ACK3D_SCREENHEIGHT),
                                          SMOOTHING_OFF);   //Pixellated rather than blurry

    // draw bitmap starting at 0,0
    SDL_Rect dstrect;
    dstrect.x = 0;
    dstrect.y = 0;

    SDL_BlitSurface(tempZoomed, 0, MainScreen, &dstrect);  //draw the temp surface to the screen

    //If we've been hit, flash screen red for a sort of visual cue
    if(bHurtThisCycle)
    {
        boxRGBA(MainScreen, 0, 0, rcResolution.w, rcResolution.h, 255, 0, 0, 128);
    }

    //Print time left... no simple way to do it without SDL_TTF or a lot of extra work
  /*nHours = (short)(nSecondsLeft / 3600);
  t = (short)(nHours * 3600);
  nMin = (short)((nSecondsLeft - t) / 60);
  nSec = (short)(nSecondsLeft - t - (nMin * 60));
  sprintf(szBuf,"Station Escape - %02d:%02d:%02d",nHours,nMin,nSec); */

SDL_Flip(MainScreen);   //Flip the double-buffered window to the screen
SDL_FreeSurface(tempSurface);
SDL_FreeSurface(tempZoomed);
}

//****************************************************************************
// Message handler for the 3D window
//****************************************************************************
void AckWndProc( SDL_Event event )
{

switch (event.type)
  {
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_TAB)
      {
      ShootBomb();
      break;
      }

// Fall through here
  case SDL_KEYUP:
    ProcessKeys(event);
    break;

  case SDL_MOUSEBUTTONDOWN:
    if(event.button.button == SDL_BUTTON_RIGHT)
        g_Ack3D->OpenDoorForPlayer();
    if(event.button.button == SDL_BUTTON_LEFT)
        ShootBomb();
    break;

  default:
    break;
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
    short   Amt,nRow,nCol;

    Amt = 16;
    if (kbPressed[kbCtrl])
        Amt = 32;

    if (kbPressed[kbLeftArrow])
        g_Ack3D->MovePlayer(Amt, -INT_ANGLE_90);

    if (kbPressed[kbRightArrow])
        g_Ack3D->MovePlayer(Amt, INT_ANGLE_90);

    if (kbPressed[kbDownArrow])
        g_Ack3D->MovePlayer(Amt, INT_ANGLE_180);

    if (kbPressed[kbUpArrow])
        g_Ack3D->MovePlayer(Amt);

    //Open any doors for the player - here's the spot to do it if we want automatic doors
    //AckCheckDoorOpen(ae->xPlayer,ae->yPlayer,ae->PlayerAngle);

    //MEH Make moving the mouse pan the view
    int xCursor, yCursor;
    SDL_GetMouseState(&xCursor, &yCursor);
    int iDistMoved = xCursor - rcResolution.w / 2;
    //Of course, we can multiply this by a scaling factor if we want to, but one ACK-3D 0.2-degree increment per pixel seems to work fine
    if(iDistMoved != 0)
        g_Ack3D->AddToPlayerAngle(iDistMoved);
    SDL_WarpMouse(rcResolution.w / 2, rcResolution.h / 2);

    bHurtThisCycle = false; //Haven't been hit this cycle yet
    //Update the objects and such
    g_Ack3D->Update();
    //Draw the view
    AckPaint();

    //Check and see if we're on top of any of the objects we need!
    short nPlayerPos = GetMapPosn(ae->xPlayer, ae->yPlayer);
    for(int i = START_REPAIR_OBJECT; i <= END_REPAIR_OBJECT; i++)
    {
        short nObjPos = GetMapPosn(ae->ObjList[i]->x, ae->ObjList[i]->y);
        if(nPlayerPos == nObjPos && ae->ObjList[i]->Active)
        {
            ae->ObjList[i]->Active = 0;
            nRepairObjectCount++;
        }
    }

    // This code determines if the player is in a special location
    // of the game map and toggles the light shading if the player
    // reaches this region. This is used to create a unique
    // dynamic light shading effect
    nRow = (short)(ae->yPlayer >> 6);
    nCol = (short)(ae->xPlayer >> 6);
    if (nRow == 33)
      {
      if (nCol == 8)
        g_Ack3D->SetLightShading(ACK_LIGHTSHADING_ON);
      else if (nCol == 10)
        g_Ack3D->SetLightShading(ACK_LIGHTSHADING_OFF);
      }

    if (nCol == 24)
      {
      if (nRow == 5)
        g_Ack3D->SetLightShading(ACK_LIGHTSHADING_ON);
      else if (nRow == 7)
        g_Ack3D->SetLightShading(ACK_LIGHTSHADING_OFF);
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
                g_Ack3D->OpenDoorForPlayer();//(ae->xPlayer,ae->yPlayer,ae->PlayerAngle);
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

void HurtPlayer(int iAmount)
{
    g_iHealth -= iAmount;
    bHurtThisCycle = true;      //Ouch! Flash screen red
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

