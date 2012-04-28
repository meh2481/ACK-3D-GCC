// Header file for Station Escape Demo Program
// (c) 1995 Ack Software (Lary Myers)

#ifndef ACKSE_H
#define ACKSE_H

#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif
#include "AckWrapper.h"

typedef unsigned short WORD;
typedef unsigned char UCHAR;
#define UINT unsigned int

int MessageLoop(void);
void AckWndProc( SDL_Event event );
bool CheckStatus(void);
int LoadBackDrop(void);
int AckProcessInfoFile(void);
void AckPaint(); // Actual paint routine for 3D Window
void ProcessKeys(SDL_Event event);
void DoCycle(void);
void ObjectCollision(NEWOBJECT* pObj, UCHAR nIndex, short nResult); //Our callback for when an object hits something
void MoveObjects(void);
void CheckFiring(short nObjIndex,NEWOBJECT *pObj);
void CheckBadGuys(UCHAR nIndex);
void HurtPlayer(int iAmount);   //Hurt the player by specified amount

#define REPAIR_OBJECTS_NEEDED 3    // Number of parts needed to win
#define REPAIR_POSITION       3769 // Location on map to win

#define BOMB_START_INDEX    32      // Starting object number
#define BOMB_END_INDEX      40      // Ending object number

//MEH Stretch an image. But it's too slow.
//void StretchImage(unsigned char* src, unsigned char* dest,
//                  unsigned int srcW, unsigned int srcH, unsigned int destW, unsigned int destH);


#endif  // defined ACKSE_H
