//#include <windows.h>
#include <stdio.h>

#include "ACK3D.H"
#include "ACKENG.H"

int32_t scPtr;
UCHAR *bmWall;

int32_t bmDistance;
int32_t BackDropRows;
USHORT ScreenOffset;

int32_t xPglobal;
int32_t yPglobal;
int32_t xBegGlobal;
int32_t yBegGlobal;

ACKENG *aeGlobal;
USHORT *xGridGlobal;
USHORT *yGridGlobal;
int32_t xPglobalHI;
int32_t yPglobalHI;
ULONG *rbaTable;
short rsHandle;

int32_t LastX1;
int32_t LastY1;
int32_t iLastX;
int32_t iLastY;

int32_t x_xPos;
int32_t x_yPos;
int32_t x_xNext;
int32_t x_yNext;
int32_t y_xPos;
int32_t y_yPos;
int32_t y_xNext;
int32_t y_yNext;

short MaxDistance;
short LightFlag;
short ErrorCode;

int32_t xMapPosn;
int32_t yMapPosn;

USHORT *Grid;
USHORT *ObjGrid;
SLICE Slice[VIEW_WIDTH];
SLICE *sPtr;

short TotalSpecial;

short DistanceTable[MAX_DISTANCE + 1];
int32_t *AdjustTable;

short xSecretmPos;
short xSecretmPos1;
short xSecretColumn;

short ySecretmPos;
short ySecretmPos1;
short ySecretColumn;

short TotalSecret;
short ViewColumn;

int32_t *SinTable;
int32_t *CosTable;

int32_t *LongTanTable;
int32_t *LongInvTanTable;
int32_t InvCosTable[INT_ANGLE_360];
int32_t InvSinTable[INT_ANGLE_360];
int32_t *LongCosTable;
int32_t *ViewCosTable;

int32_t *xNextTable;
int32_t *yNextTable;

short LastFloorAngle = -1;
short LastFloorX;
short LastFloorY;
short LastMapPosn;
short LastObjectHit;
short TotalObjects;
short FoundObjectCount;
UCHAR ObjectsSeen[MAX_OBJECTS + 1];
short MoveObjectCount;
UCHAR MoveObjectList[MAX_OBJECTS + 1];
UCHAR ObjNumber[MAX_OBJECTS + 1];
USHORT ObjRelDist[MAX_OBJECTS + 1];
short ObjColumn[MAX_OBJECTS + 1];
short ObjAngle[MAX_OBJECTS + 1];
short DirAngle[] =
{INT_ANGLE_270, INT_ANGLE_315, 0,
 INT_ANGLE_45, INT_ANGLE_90,
 INT_ANGLE_135, INT_ANGLE_180,
 INT_ANGLE_225};

UCHAR WorkPalette[768];
UCHAR *BackArray[INT_ANGLE_360];
short Resolution;

int32_t Flooru;
int32_t Floorv;
int32_t Floordu;
int32_t Floordv;
int32_t Floorkx;
int32_t Floorky;
int32_t Floorku;
int32_t Floorkv;
int32_t Floorkdu;
int32_t Floorkdv;
UCHAR *Floorbm;
UCHAR *Floorscr;
UCHAR *FloorscrTop;
UCHAR *Floorptr2;
UCHAR *Floors1;
UCHAR *Floors2;
int32_t Floorht;
int32_t Floorwt;
short Floorvht;
short Flooreht;
int32_t FloorLastbNum;
int32_t FloorLastbm;

short ViewHeight = 31;
short CeilingHeight = 31;
short LastWallHeight;
short PlayerAngle;
short ViewAngle;
USHORT SysFlags;
UCHAR **WallbMaps;
UCHAR *VidTop;
UCHAR *VidBottom;
short BotRowTable[320];
USHORT FloorMap[4096];
USHORT CeilMap[4096];
UCHAR  HitMap[4096];

UCHAR *VidSeg;
char *scantables[96];
UCHAR	AckKeys[128];	// Buffer for keystrokes
int32_t AckTimerCounter;

// **** End of Data ****

