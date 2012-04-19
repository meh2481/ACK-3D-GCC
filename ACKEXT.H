/*		   ACK-3D ( Animation Construction Kit 3D )		      */
#ifndef ACKEXT_H
#define ACKEXT_H

#include "ACKENG.H"
extern	UCHAR	    *BackArray[];
extern	int32_t	    xPglobal;
extern	int32_t	    yPglobal;
extern	int32_t	    xBegGlobal;
extern	int32_t	    yBegGlobal;
extern	int32_t	    BackDropRows;
extern	ACKENG	    *aeGlobal;
extern	USHORT	    *xGridGlobal;
extern	USHORT	    *yGridGlobal;
extern	int32_t	    xPglobalHI;
extern	int32_t	    yPglobalHI;
extern	ULONG	    *rbaTable;

extern	int32_t	    bmDistance;


extern	short	    rsHandle;
extern	int32_t	    LastX1;
extern	int32_t	    LastY1;
extern	int32_t	    iLastX;
extern	int32_t	    iLastY;
extern	short	    MaxDistance;
extern	short	    ErrorCode;
extern	short	    LightFlag;

extern	int32_t	    xMapPosn;
extern	int32_t	    yMapPosn;

extern	short	    DefZone[];
extern	short	    AckLightZones[];
extern	UCHAR	    *HtTable[];
extern	USHORT	    *Grid;
extern	USHORT	    *ObjGrid;
extern	UCHAR	    HitMap[];
extern	UCHAR	    *BitmapXferPtr;
extern	short	    TotalSpecial;
extern	short	    DistanceTable[];
extern	int32_t	    *AdjustTable;
extern	short	    xSecretmPos;
extern	short	    xSecretmPos1;
extern	short	    xSecretColumn;
extern	short	    ySecretmPos;
extern	short	    ySecretmPos1;
extern	short	    ySecretColumn;
extern	short	    TotalSecret;
extern	short	    ViewColumn;
extern	int32_t	    *SinTable;
extern	int32_t	    *CosTable;
extern	int32_t	    *LongTanTable;
extern	int32_t	    *LongInvTanTable;
extern	int32_t	    InvCosTable[];
extern	int32_t	    InvSinTable[];
extern	int32_t	    *LongCosTable;
extern	int32_t	    *ViewCosTable;
extern	int32_t	    *xNextTable;
extern	int32_t	    *yNextTable;
extern	UCHAR	    ObjectsSeen[];
extern	UCHAR	    MoveObjectList[];
extern	short	    TotalObjects;
extern	short	    FoundObjectCount;
extern	short	    MoveObjectCount;
extern	short	    LastObjectHit;
extern	short	    LastMapPosn;
extern	UCHAR	    ObjNumber[];
extern	USHORT	    ObjRelDist[];
extern	short	    ObjColumn[];
extern	short	    ObjAngle[];
extern	short	    DirAngle[];
extern	UCHAR	    LightMap[];

extern	USHORT	    FloorMap[];
extern	USHORT	    CeilMap[];
extern	SLICE	    Slice[];
extern	USHORT	    ScreenOffset;
extern	short	    LastFloorAngle;
extern	short	    LastFloorX;
extern	short	    LastFloorY;
extern	int32_t	    Flooru;
extern	int32_t	    Floorv;
extern	int32_t	    Floordu;
extern	int32_t	    Floordv;
extern	int32_t	    Floorkx;
extern	int32_t	    Floorky;
extern	int32_t	    Floorku;
extern	int32_t	    Floorkv;
extern	int32_t	    Floorkdu;
extern	int32_t	    Floorkdv;
extern	UCHAR	    *Floorbm;
extern	UCHAR	    *Floorscr;
extern	UCHAR	    *FloorscrTop;
extern	UCHAR	    *Floorptr2;
extern	UCHAR	    *Floors1;
extern	UCHAR	    *Floors2;
extern	int32_t	    Floorht;
extern	int32_t	    Floorwt;
extern	short	    Floorvht;
extern	short	    Flooreht;
extern	short	    ViewAngle;
extern	short	    ViewHeight;
extern	short	    CeilingHeight;
extern	short	    Resolution;
extern	short	    LastWallHeight;
extern	short	    PlayerAngle;
extern	short	    ViewAngle;
extern	USHORT	    SysFlags;
extern	SLICE	    Slice[];
extern	SLICE	    *sPtr;
extern	UCHAR	    **WallbMaps;
extern	UCHAR	    *VidTop;
extern	UCHAR	    *VidBottom;
extern	short	    BotRowTable[];
extern	USHORT	    FloorMap[];
extern	USHORT	    CeilMap[];
extern	char	    *scantables[];

#endif
