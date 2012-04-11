// Header file for Station Escape Demo Program
// (c) 1995 Ack Software (Lary Myers)

#ifndef ACKSE_H
#define ACKSE_H

#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

//Define stuff that's generally defined in windows.h
/*typedef struct POINTTYPE {
    int x, y;
} POINT;*/
typedef unsigned short WORD;
typedef unsigned char UCHAR;
#define UINT unsigned int


//bool Register(HINSTANCE hInst);
//bool CreateAll(HINSTANCE hInst);
int MessageLoop(void);
void AckWndProc( SDL_Event event );
bool CheckStatus(void);
int LoadBackDrop(void);
int AckProcessInfoFile(void);

//#define IM_TIMEOUT  3       // Ran out of time
//#define IM_WON      4       // Won the game!

#define REPAIR_OBJECTS_NEEDED 3    // Number of parts needed to win
#define REPAIR_POSITION       3769 // Location on map to win

#define BOMB_START_INDEX    32      // Starting object number
#define BOMB_END_INDEX      40      // Ending object number

//MEH structure for holding RGB data
typedef struct {
    unsigned char r, g, b, a;
} PalletteSlot;

//MEH read in pallette data from "pallette.txt"
void ReadInPallette(void);

//MEH Stretch an image. But it's too slow.
//void StretchImage(unsigned char* src, unsigned char* dest,
//                  unsigned int srcW, unsigned int srcH, unsigned int destW, unsigned int destH);


#endif  // defined ACKSE_H
