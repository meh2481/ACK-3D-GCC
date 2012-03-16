//ACK-3D Source
//ACKRTNRecode.h - stuff needed by the routine recodes in ACKRTNRecode.c
//Copyright (c) 2012 Mark Hutcheson
//sort of, anyway. Lary Myers did most of the work back in 1995 or so
#ifndef ACKRR_H
#define ACKRR_H

#include "ack3d.h"
#include "ackext.h"

void ShowCol(void);                     // Routines for drawing a slice
void ShowColMask(void);                 // column by column
void ShowColNS(void);
void ShowColMaskNS(void);
void ShowColLow(void);
void ShowColMaskLow(void);


//External variables needed by the functions in ACKRTNRecode.c
extern short   gWinEndX;
extern short   gWinStartX;
extern long    WallDistTable[VIEW_WIDTH];
extern UCHAR*  gScrnBuffer;
extern short   gWinHeight;
extern UCHAR   *scVid;         // Variables used in low level routines for
extern UCHAR   *scWall;                // building and drawing slices
extern UCHAR   *scPal;
extern short   scdst;
extern short   scwht;
extern short   scmulti;
extern short   sctopht;
extern short   scbotht;
extern short   scsavwht;
extern short   scmulcnt;
extern UCHAR   *scsavVid;
extern USHORT    scbNum;
extern UCHAR   *scMulData;
extern UCHAR   *scColumn;
extern UCHAR   *gPtr;
extern UCHAR   *gmPtr;
extern short   gBitmapNumber;
extern short   gBitmapColumn;
extern short   gyBitmapNumber;
extern short   gyBitmapColumn;
extern long    gWallDistance;
extern short   gmPos;
extern DOORS   *gDoor;
extern DOORS   *gDoorPosn;
extern short   wFound;
extern UCHAR   *mgPtr;
extern short   gWinFullWidth;          // Global variables for setting up a viewport
extern long    gWinDWORDS;             // These are the global variables used by the
extern long    gWinStartOffset;                // low-level assembly language routines to draw slices
extern short   gWinStartX;
extern short   gWinStartY;
extern short   gWinEndX;
extern short   gWinEndY;
extern short   gWinHeight;
extern short   gWinHalfHeight;
extern short   gWinWidth;
extern short   gCenterRow;
extern short   gCenterOff;
extern long    gBottomOff;
extern UCHAR   *gScrnBufferCenter;
extern UCHAR   *gScrnBuffer;
extern UCHAR   *gBkgdBuffer;
extern UCHAR   *gPalTable;
extern short   gMultiWalls;
extern short   *LowerTable[2048];

//External functions needed by the functions in ACKRTNRecode.c
extern void    (*FloorCeilRtn)(void);
void CheckDoors(void);
void xRaySetup(void);
void yRaySetup(void);
void BuildSlice(void);
void FindObject(void);

//Function prototypes for ACKRTNRecode.c functions that are only used in ACKRTNRecode.c
void CheckHitMap(void);
void DrawWalls(void);
unsigned char GetLightShadedPixel(unsigned char cPixel);    //MEH Get a pixel's correct lightshaded value
void ShowColumn(char bTrans, char bLightShaded);            //MEH All the ShowColX() routines call this one, for code reuse

//MEH Variables I'm adding for ease-of-use
SLICE* gCurSlice;
int    g_iCurScreenSpot;//From 0 to 319
unsigned int ShadingRegion;
extern ColorRange  ranges[64];
//extern ACKENG* ae;

/*extern long scPtr;
extern UCHAR *bmWall;

extern long bmDistance;
extern long BackDropRows;
extern USHORT ScreenOffset;

extern long xPglobal;
extern long yPglobal;
extern long xBegGlobal;
extern long yBegGlobal;

extern ACKENG *aeGlobal;
extern USHORT *xGridGlobal;
extern USHORT *yGridGlobal;
extern long xPglobalHI;
extern long yPglobalHI;
extern ULONG *rbaTable;
extern short rsHandle;

extern long LastX1;
extern long LastY1;
extern long iLastX;
extern long iLastY;

extern long x_xPos;
extern long x_yPos;
extern long x_xNext;
extern long x_yNext;
extern long y_xPos;
extern long y_yPos;
extern long y_xNext;
extern long y_yNext;

extern short MaxDistance;
extern short LightFlag;
extern short ErrorCode;

extern long xMapPosn;
extern long yMapPosn;

extern USHORT *Grid;
extern USHORT *ObjGrid;
extern SLICE Slice[VIEW_WIDTH];
extern SLICE *sPtr;

extern short TotalSpecial;

extern short DistanceTable[MAX_DISTANCE + 1];
extern long *AdjustTable;

extern short xSecretmPos;
extern short xSecretmPos1;
extern short xSecretColumn;

extern short ySecretmPos;
extern short ySecretmPos1;
extern short ySecretColumn;

extern short TotalSecret;
extern short ViewColumn;

extern long *SinTable;
extern long *CosTable;

extern long *LongTanTable;
extern long *LongInvTanTable;
extern long InvCosTable[INT_ANGLE_360];
extern long InvSinTable[INT_ANGLE_360];
extern long *LongCosTable;
extern long *ViewCosTable;

extern long *xNextTable;
extern long *yNextTable;

extern short LastFloorAngle = -1;
extern short LastFloorX;
extern short LastFloorY;
extern short LastMapPosn;
extern short LastObjectHit;
extern short TotalObjects;
extern short FoundObjectCount;
extern UCHAR ObjectsSeen[MAX_OBJECTS + 1];
extern short MoveObjectCount;
extern UCHAR MoveObjectList[MAX_OBJECTS + 1];
extern UCHAR ObjNumber[MAX_OBJECTS + 1];
extern USHORT ObjRelDist[MAX_OBJECTS + 1];
extern short ObjColumn[MAX_OBJECTS + 1];
extern short ObjAngle[MAX_OBJECTS + 1];

extern UCHAR WorkPalette[768];
extern UCHAR *BackArray[INT_ANGLE_360];
extern short Resolution;

extern long Flooru;
extern long Floorv;
extern long Floordu;
extern long Floordv;
extern long Floorkx;
extern long Floorky;
extern long Floorku;
extern long Floorkv;
extern long Floorkdu;
extern long Floorkdv;
extern UCHAR *Floorbm;
extern UCHAR *Floorscr;
extern UCHAR *FloorscrTop;
extern UCHAR *Floorptr2;
extern UCHAR *Floors1;
extern UCHAR *Floors2;
extern long Floorht;
extern long Floorwt;
extern short Floorvht;
extern short Flooreht;
extern long FloorLastbNum;
extern long FloorLastbm;

extern short ViewHeight = 31;
extern short CeilingHeight = 31;
extern short LastWallHeight;
extern short PlayerAngle;
extern short ViewAngle;
extern USHORT SysFlags;
extern UCHAR **WallbMaps;
extern UCHAR *VidTop;
extern UCHAR *VidBottom;
extern short BotRowTable[320];
extern USHORT FloorMap[4096];
extern USHORT CeilMap[4096];
extern UCHAR  HitMap[4096];

extern UCHAR *VidSeg;
extern char *scantables[96];
extern UCHAR	AckKeys[128];	// Buffer for keystrokes
extern long AckTimerCounter;*/

#endif

