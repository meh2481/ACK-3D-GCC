//ACK-3D Source
//ACKRTNRecode.h - stuff needed by the routine recodes in ACKRTNRecode.c
//Copyright (c) 2012 Mark Hutcheson
//sort of, anyway. Lary Myers did most of the work back in 1995 or so
#ifndef ACKRR_H
#define ACKRR_H

#include "ACK3D.H"
#include "ACKEXT.H"

void ShowCol(void);                     // Routines for drawing a slice
void ShowColMask(void);                 // column by column
void ShowColNS(void);
void ShowColMaskNS(void);
void ShowColLow(void);
void ShowColMaskLow(void);


//External variables needed by the functions in ACKRTNRecode.c
extern short   gWinEndX;
extern short   gWinStartX;
extern int32_t    WallDistTable[VIEW_WIDTH];
extern UCHAR*  gScrnBuffer;
extern short   gWinHeight;
//extern UCHAR   *scVid;         // Variables used in low level routines for
//extern UCHAR   *scWall;                // building and drawing slices
//extern UCHAR   *scPal;
//extern short   scdst;
//extern short   scwht;
//extern short   scmulti;
//extern short   sctopht;
//extern short   scbotht;
//extern short   scsavwht;
//extern short   scmulcnt;
//extern UCHAR   *scsavVid;
//extern USHORT    scbNum;
//extern UCHAR   *scMulData;
//extern UCHAR   *scColumn;
extern UCHAR   *gPtr;
extern UCHAR   *gmPtr;
extern short   gBitmapNumber;
extern short   gBitmapColumn;
extern short   gyBitmapNumber;
extern short   gyBitmapColumn;
extern int32_t    gWallDistance;
extern short   gmPos;
extern DOORS   *gDoor;
extern DOORS   *gDoorPosn;
extern short   wFound;
extern UCHAR   *mgPtr;
extern short   gWinFullWidth;          // Global variables for setting up a viewport
extern int32_t    gWinDWORDS;             // These are the global variables used by the
extern int32_t    gWinStartOffset;                // low-level assembly language routines to draw slices
extern short   gWinStartX;
extern short   gWinStartY;
extern short   gWinEndX;
extern short   gWinEndY;
extern short   gWinHeight;
extern short   gWinHalfHeight;
extern short   gWinWidth;
extern short   gCenterRow;
extern short   gCenterOff;
extern int32_t    gBottomOff;
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

/*extern int32_t scPtr;
extern UCHAR *bmWall;

extern int32_t bmDistance;
extern int32_t BackDropRows;
extern USHORT ScreenOffset;

extern int32_t xPglobal;
extern int32_t yPglobal;
extern int32_t xBegGlobal;
extern int32_t yBegGlobal;

extern ACKENG *aeGlobal;
extern USHORT *xGridGlobal;
extern USHORT *yGridGlobal;
extern int32_t xPglobalHI;
extern int32_t yPglobalHI;
extern ULONG *rbaTable;
extern short rsHandle;

extern int32_t LastX1;
extern int32_t LastY1;
extern int32_t iLastX;
extern int32_t iLastY;

extern int32_t x_xPos;
extern int32_t x_yPos;
extern int32_t x_xNext;
extern int32_t x_yNext;
extern int32_t y_xPos;
extern int32_t y_yPos;
extern int32_t y_xNext;
extern int32_t y_yNext;

extern short MaxDistance;
extern short LightFlag;
extern short ErrorCode;

extern int32_t xMapPosn;
extern int32_t yMapPosn;

extern USHORT *Grid;
extern USHORT *ObjGrid;
extern SLICE Slice[VIEW_WIDTH];
extern SLICE *sPtr;

extern short TotalSpecial;

extern short DistanceTable[MAX_DISTANCE + 1];
extern int32_t *AdjustTable;

extern short xSecretmPos;
extern short xSecretmPos1;
extern short xSecretColumn;

extern short ySecretmPos;
extern short ySecretmPos1;
extern short ySecretColumn;

extern short TotalSecret;
extern short ViewColumn;

extern int32_t *SinTable;
extern int32_t *CosTable;

extern int32_t *LongTanTable;
extern int32_t *LongInvTanTable;
extern int32_t InvCosTable[INT_ANGLE_360];
extern int32_t InvSinTable[INT_ANGLE_360];
extern int32_t *LongCosTable;
extern int32_t *ViewCosTable;

extern int32_t *xNextTable;
extern int32_t *yNextTable;

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

extern int32_t Flooru;
extern int32_t Floorv;
extern int32_t Floordu;
extern int32_t Floordv;
extern int32_t Floorkx;
extern int32_t Floorky;
extern int32_t Floorku;
extern int32_t Floorkv;
extern int32_t Floorkdu;
extern int32_t Floorkdv;
extern UCHAR *Floorbm;
extern UCHAR *Floorscr;
extern UCHAR *FloorscrTop;
extern UCHAR *Floorptr2;
extern UCHAR *Floors1;
extern UCHAR *Floors2;
extern int32_t Floorht;
extern int32_t Floorwt;
extern short Floorvht;
extern short Flooreht;
extern int32_t FloorLastbNum;
extern int32_t FloorLastbm;

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
extern int32_t AckTimerCounter;*/

#endif

