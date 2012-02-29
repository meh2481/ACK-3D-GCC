// Support routines for ACKEDIT application
#define  STRICT
#include <windows.h>
#pragma hdrstop
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <mem.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <sys\stat.h>
#include <string.h>
//#include "wing.h"
//#include "wingdll.h"

// Uncomment next line to use normal windows colors
//#define _USE_SYSTEM_PALETTE 1


//extern "C" {
#include "keys.h"
#include "ack3d.h"
extern  short   rsHandle;
extern  ULONG   *rbaTable;
extern  short   Resolution;
extern  UCHAR   colordat[];
extern  UCHAR   *BackArray[];
extern  unsigned short FloorMap[];
extern  unsigned short CeilMap[];
extern  unsigned short *ObjGrid;
extern  unsigned short *Grid;
//};

#include "ackedit.h"

extern ACKENG   *ae;
extern  short   nOperation;
extern  short   bMapIndex[];
extern  short   oMapIndex[];
extern  short   bMapType[];
extern  short   oMapType[];
extern  char    *bMapFiles[];
extern  char    *oMapFiles[];
extern  char    BackGndFile[];
extern  char    ResFileName[];
extern  HINSTANCE   hInstGlobal;
extern  BOOL    bModified;

// Globals
    int     LineNumber;
    char    LineBuffer[200];
    short   LastObjectIndex;
    int     MapResource;
    int     PalResource;
    int     ResScreenBack;
    int     ResScrollBack;
    long    TempRBA[500];

//=============================================================================
//
//=============================================================================
void gkScaleBitmap(short bmWt,short bmHt,short DstWt,short DstHt,UCHAR *bm,
                   UCHAR *DstBuffer)
{
    long    bmPixWt,bmPixHt;
    long    bmNextHt,bmNextWt;
    short   col;
    long    bmOffset,bmPos;


bmPixWt = bmWt << 16;
bmPixWt /= (long)DstWt;
bmPixHt = bmHt << 16;
bmPixHt /= (long)DstHt;

bmNextHt = 0;

while (DstHt-- > 0)
    {
    bmOffset = (bmNextHt >> 16) * bmWt;
    col = DstWt;
    bmNextWt = 0;

    while (col-- > 0)
        {
        bmPos = bmOffset + (bmNextWt >> 16);
        *DstBuffer++ = bm[bmPos];
        bmNextWt += bmPixWt;
        }

    bmNextHt += bmPixHt;
    }

}
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
void ClearAddinFiles(void)
{
    short   i;

for (i = 0; i < 255; i++)
    {
    if (oMapFiles[i] != NULL)
        {
        AckFree(oMapFiles[i]);
        oMapFiles[i] = NULL;
        }
    if (bMapFiles[i] != NULL)
        {
        AckFree(bMapFiles[i]);
        bMapFiles[i] = NULL;
        }
    }

BackGndFile[0] = '\0';
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
void AckReset(void)
{
    short   i;

for (i = 0; i < 4096; i++)
    {
    if (ae->mxGrid[i] != NULL)
        {
        AckFree(ae->mxGrid[i]);
        ae->mxGrid[i] = NULL;
        }
    if (ae->myGrid[i] != NULL)
        {
        AckFree(ae->myGrid[i]);
        ae->myGrid[i] = NULL;
        }
    ae->xGrid[i] = 0;
    ae->yGrid[i] = 0;
    }

for (i = 0; i < 255; i++)
    {
    if (ae->bMaps[i] != NULL)
        {
        AckFree(ae->bMaps[i]);
        ae->bMaps[i] = NULL;
        }
    if (ae->oMaps[i] != NULL)
        {
        AckFree(ae->oMaps[i]);
        ae->oMaps[i] = NULL;
        }
    if (ae->ObjList[i] != NULL)
        {
        AckFree(ae->ObjList[i]);
        ae->ObjList[i] = NULL;
        }

    oMapIndex[i] = 0;
    bMapIndex[i] = 0;
    }

for (i = 0; i < 640; i++)
    {
    if (BackArray[i] != NULL)
        {
        AckFree(BackArray[i]);
        BackArray[i] = NULL;
        }
    }

ResScrollBack = 0;
ClearAddinFiles();
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
void ProcessBackDrop(UCHAR *bPtr)
{
        int     i,j,pos;
        UCHAR   *aPtr;

for (i = 0; i < 320; i++)
    {
    pos = i + 4;
    aPtr = BackArray[i];
    if (aPtr == NULL)
        {
        aPtr = BackArray[i] = (UCHAR *)AckMalloc(101);
        if (aPtr == NULL)
            break;
        }
    for (j = 0; j < 100; j++)
        {
        *aPtr++ = bPtr[pos];
        pos += 320;
        }
    }

for (i = 320; i < 640; i++)
    {
    pos = (i - 320) + 32004;
    aPtr = BackArray[i];
    if (aPtr == NULL)
        {
        aPtr = BackArray[i] = (UCHAR *)AckMalloc(101);
        if (aPtr == NULL)
            break;
        }
    for (j = 0; j < 100; j++)
        {
        *aPtr++ = bPtr[pos];
        pos += 320;
        }
    }

}


//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
// Loads a background image (mountains) and processes the image into the
// separate slices for use at display time. Currently a background image
// can be 640 columns wide. This number can be made dynamic if needbe and would
// need to be changed in the routine below and in the DrawBackground routine
// in the ACK engine.
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
int LoadBackDrop(void)
{
        UCHAR   *bPtr;

if (ResScrollBack)
    {
    bPtr = AckReadPCX((char *)ResScrollBack);

    if (bPtr == NULL)
        return(8);

    ProcessBackDrop(bPtr);
    AckFree(bPtr);
    }

return(0);
}

//=============================================================================
// Same as LoadBackDrop except a string filename is used instead of a
// resource number.
//=============================================================================
int LoadNewBackDrop(char *szName)
{
    UCHAR   *bPtr;

//if(ae->bmLoadType == BMLOAD_BBM)
    bPtr = AckReadPCX(szName);
//else
//    bPtr = AckReadPCX(szName);
if (bPtr == NULL)
    return(8);

ProcessBackDrop(bPtr);
AckFree(bPtr);

return(0);
}

//=============================================================================
// Reads a text line from the resource file
//=============================================================================
int ReadLine(void)
{
    int     len;
    char    ch;

len = 0;
while (len < 200)
    {
    if (_lread(rsHandle,&LineBuffer[len],1) != 1)
        break;

    ch = LineBuffer[len];
    if (ch == 10)
        continue;

    if (ch == 13)
        break;

    len++;
    }

LineBuffer[len] = '\0';

return(len);
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
// Skips to the next parameter in a text line
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
char *GetNextParm(char *s)
{
    char    ch;

while (*s)
    {
    ch = *s++;
    if (ch == ',')
        {
        while (*s)
            {
            ch = *s;
            if (ch != ',' && ch != ' ' && ch != '\t')
                return(s);
            s++;
            }
        return(NULL);
        }

    }

return(NULL);
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
// Loads a wall bitmap specified in info file
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
int LoadWall(void)
{
    int     wnum,rnum,result;
    long    pos;
    char    *lb;


lb = LineBuffer;            // Info file buffer
wnum = atoi(lb);            // Wall number to load into

lb = GetNextParm(lb);

if (lb == NULL)
    return(-1);

rnum = atoi(lb);            // Resource number

pos = _llseek(rsHandle,0L,SEEK_CUR);
result = AckLoadWall(ae,(short)wnum,(char *)rnum);
_llseek(rsHandle,pos,SEEK_SET);

bMapIndex[wnum] = rnum;
bMapType[wnum] = ae->bmLoadType;
return(result);
}


//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
// Loads an object bitmap specified in info file
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
int LoadObject(void)
{
    int     onum,rnum,result;
    long    pos;
    char    *lb;

lb = LineBuffer;
onum = atoi(lb);            // Object bitmap number

lb = GetNextParm(lb);

if (lb == NULL)
    return(-2);

rnum = atoi(lb);            // Resource number
pos = _llseek(rsHandle,0L,SEEK_CUR);
result = AckLoadObject(ae,(short)onum,(char *)rnum);
_llseek(rsHandle,pos,SEEK_SET);

oMapIndex[onum] = rnum;
oMapType[onum] = ae->bmLoadType;
return(result);
}


//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
// Skip any leading spaces in the string
// NOTE: Actually modifies the string passed!
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
char *SkipSpaces(char *s)
{

while (*s == ' ' || *s == '\t' || *s == ',')
    strcpy(s,&s[1]);

return(s);
}


//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
// Creates and object of the desired style
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
int CreateObject(void)
{
    int     onum,vnum,speed;
    short   result,oType;
    short   NumViews,bmPerView;
    USHORT    flags;
    char    *lb;
    OBJSEQ  os;
    NEWOBJECT   *pObj;

lb = LineBuffer;

if (!strnicmp(lb,"NUMBER:",7))
    {
    lb = &lb[7];
    onum = atoi(lb);
    if (onum < 1 || onum > MAX_OBJECTS)
        return(-3);

    result = AckCreateObject(ae,(short)onum);
    if (result)
        return(result);

    LastObjectIndex = onum;
    lb = GetNextParm(lb);
    if (lb == NULL)
        return(-4);

    ae->ObjList[onum]->Speed = atoi(lb);
    return(0);
    }

onum = LastObjectIndex;         // Object number

oType = 0;

if (!strnicmp(lb,"CREATE:",7))
    {
    oType = NO_CREATE;
    lb = &lb[7];
    }

if (!strnicmp(lb,"DESTROY:",8))
    {
    oType = NO_DESTROY;
    lb = &lb[8];
    }

if (!strnicmp(lb,"WALK:",5))
    {
    oType = NO_WALK;
    lb = &lb[5];
    }

if (!strnicmp(lb,"ATTACK:",7))
    {
    oType = NO_ATTACK;
    lb = &lb[7];
    }

if (!strnicmp(lb,"INTERACT:",9))
    {
    oType = NO_INTERACT;
    lb = &lb[9];
    }

if (!oType)
    return(-5);

lb = SkipSpaces(lb);
if (lb == NULL)
    return(-6);

flags = 0;
strupr(lb);

if (strstr(lb,"ANIMATE") != NULL)
    flags |= OF_ANIMATE;

if (strstr(lb,"MOVEABLE") != NULL)
    flags |= OF_MOVEABLE;

if (strstr(lb,"PASSABLE") != NULL)
    flags |= OF_PASSABLE;

if (strstr(lb,"MULTIVIEW") != NULL)
    flags |= OF_MULTIVIEW;

if (strstr(lb,"SHOWONCE") != NULL)
    flags |= OF_ANIMONCE;

lb = GetNextParm(lb);
if (lb == NULL)
    return(-5);

NumViews = atoi(lb);
if (NumViews < 1)
    return(-6);

lb = GetNextParm(lb);
if (lb == NULL)
    return(-7);

bmPerView = atoi(lb);
if (bmPerView < 1)
    return(-7);

vnum = NumViews * bmPerView;
if (vnum > MAX_OBJ_BITMAPS)
    return(-8);

lb = GetNextParm(lb);
if (lb == NULL)
    return(-9);

vnum = 0;

while (lb != NULL && vnum < MAX_OBJ_BITMAPS)
    {
    os.bitmaps[vnum++] = atoi(lb);
    lb = GetNextParm(lb);
    }

os.bmBitmapsPerView = bmPerView;
os.flags = flags;
os.MaxBitmaps = bmPerView;
os.bmSides = NumViews;

result = AckSetupObject(ae,onum,oType,&os);

pObj = ae->ObjList[onum];
if (pObj != NULL)
    {
    ObjGrid[pObj->mPos] = onum;
    }

return(result);
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
void CheckObjects(void)
{
    short   i;
    BOOL    bFlag;
    OBJSEQ  *os;

for (i = 0; i < MAX_OBJECTS;i++)
    {
    if (ae->ObjList[i] != NULL)
        {
        bFlag = FALSE;
        os = &ae->ObjList[i]->Create;
        if (os->bitmaps[0] != 0)
            bFlag = TRUE;
        os = &ae->ObjList[i]->Destroy;
        if (os->bitmaps[0] != 0)
            bFlag = TRUE;
        os = &ae->ObjList[i]->Walk;
        if (os->bitmaps[0] != 0)
            bFlag = TRUE;
        os = &ae->ObjList[i]->Attack;
        if (os->bitmaps[0] != 0)
            bFlag = TRUE;
        os = &ae->ObjList[i]->Interact;
        if (os->bitmaps[0] != 0)
            bFlag = TRUE;

        if (bFlag == FALSE)
            {
            AckFree(ae->ObjList[i]);
            ae->ObjList[i] = NULL;
            }
        }
    }

for (i = MAX_OBJECTS-1; i > 0; i--)
    {
    if (ae->ObjList[i] != NULL)
        break;
    }

ae->MaxObjects = i;

}


//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
// Reads the ASCII info file and processes the commands.
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
int ProcessInfoFile(short QuietFlag)
{
    int     result;
    int     mode;
    long    pos;
    char    *lb;

nOperation = OP_WAIT;
SetOperationCursor();
LineNumber = 0;
// Position to start of info file within resource file
_llseek(rsHandle,rbaTable[0],SEEK_SET);

mode = result = 0;

while (!result)
    {
    LineNumber++;
    if (!ReadLine())
        continue;

    if (*LineBuffer == ';')
        continue;

    if (!strnicmp(LineBuffer,"END:",4))
        break;

    switch (mode)
        {
        case 1:         // Read walls
            if (!strnicmp(LineBuffer,"LOADTYPE:",9))
                {
                ae->bmLoadType = atoi(&LineBuffer[9]);  // Sets for GIF or BBM
                break;
                }

            if (!strnicmp(LineBuffer,"ENDBITMAPS:",11))
                mode = 4;
            else
                result = LoadWall();
            break;

        case 2:         // Object bitmaps
            if (!strnicmp(LineBuffer,"LOADTYPE:",9))    // Sets for GIF or BBM
                {
                ae->bmLoadType = atoi(&LineBuffer[9]);
                break;
                }

            if (!strnicmp(LineBuffer,"ENDBITMAPS:",11))
                mode = 5;
            else
                result = LoadObject();
            break;

        case 3:         // Create Object
            if (!strnicmp(LineBuffer,"ENDDESC:",8))
                mode = 5;
            else
                result = CreateObject();
            break;

        case 4:         // Walls topic
            if (!strnicmp(LineBuffer,"BITMAPS:",8))
                mode = 1;

            if (!strnicmp(LineBuffer,"ENDWALLS:",9))
                mode = 0;
            break;


        case 5:         // Objects topic
            if (!strnicmp(LineBuffer,"BITMAPS:",8))
                mode = 2;

            if (!strnicmp(LineBuffer,"OBJDESC:",8))
                mode = 3;

            if (!strnicmp(LineBuffer,"ENDOBJECTS:",11))
                mode = 0;
            break;


        default:
            if (!strnicmp(LineBuffer,"WALLS:",6))
                {
                mode = 4;
                break;
                }

            if (!strnicmp(LineBuffer,"OBJECTS:",8))
                {
                mode = 5;
                break;
                }

            if (!strnicmp(LineBuffer,"MAPFILE:",8))
                {
                MapResource = atoi(&LineBuffer[8]);
                pos = _llseek(rsHandle,0L,SEEK_CUR);
                result = AckReadMapFile(ae,(char *)MapResource);
                _llseek(rsHandle,pos,SEEK_SET);
                break;
                }

            if (!strnicmp(LineBuffer,"PALFILE:",8))
                {
                PalResource = atoi(&LineBuffer[8]);
                break;
                }

            if (!strnicmp(LineBuffer,"XPLAYER:",8))
                {
                ae->xPlayer = atoi(&LineBuffer[8]);
                break;
                }

            if (!strnicmp(LineBuffer,"YPLAYER:",8))
                {
                ae->yPlayer = atoi(&LineBuffer[8]);
                break;
                }

            if (!strnicmp(LineBuffer,"PLAYERANGLE:",12))
                {
                ae->PlayerAngle = atoi(&LineBuffer[12]);
                break;
                }

            if (!strnicmp(LineBuffer,"SCREENBACK:",11))
                {
                ResScreenBack = atoi(&LineBuffer[11]);
                break;
                }

            if (!strnicmp(LineBuffer,"SCROLLBACK:",11))
                {
                ResScrollBack = atoi(&LineBuffer[11]);
                break;
                }

            if (!strnicmp(LineBuffer,"TOPCOLOR:",9))
                {
                ae->TopColor = atoi(&LineBuffer[9]);
                break;
                }

            if (!strnicmp(LineBuffer,"BOTTOMCOLOR:",12))
                {
                ae->BottomColor = atoi(&LineBuffer[12]);
                break;
                }

            if (!strnicmp(LineBuffer,"SHADING:",8))
                {
                strupr(LineBuffer);
                if (strstr(&LineBuffer[8],"OFF") != NULL)
                    ae->LightFlag = SHADING_OFF;
                else
                    ae->LightFlag = SHADING_ON;
                break;
                }

            if (!strnicmp(LineBuffer,"FLOORS:",7))
                {
                strupr(LineBuffer);
                if (strstr(&LineBuffer[7],"OFF") != NULL)
                    ae->SysFlags |= SYS_SOLID_FLOOR;
                else
                    {
                    ae->SysFlags &= ~SYS_SOLID_FLOOR;
                    }
                break;
                }


            if (!strnicmp(LineBuffer,"CEILING:",8))
                {
                strupr(LineBuffer);
                if (strstr(&LineBuffer[8],"OFF") != NULL)
                    ae->SysFlags |= SYS_SOLID_CEIL;
                else
                    {
                    ae->SysFlags &= ~SYS_SOLID_CEIL;
                    }
                break;
                }

            if (!strnicmp(LineBuffer,"SINGLEBITMAP:",13))
                {
                strupr(LineBuffer);
                if (strstr(&LineBuffer[13],"ON") != NULL)
                    ae->SysFlags |= SYS_SINGLE_BMP;
                else
                    {
                    ae->SysFlags &= ~SYS_SINGLE_BMP;
                    }
                break;
                }

            if (!strnicmp(LineBuffer,"CEILBITMAP:",11))
                {
                ae->CeilBitmap = atoi(&LineBuffer[11]);
                break;
                }

            if (!strnicmp(LineBuffer,"FLOORBITMAP:",12))
                {
                ae->FloorBitmap = atoi(&LineBuffer[12]);
                break;
                }


            if (!strnicmp(LineBuffer,"RESOLUTION:",11))
                {
                Resolution = atoi(&LineBuffer[11]);
                break;
                }

            if (!strnicmp(LineBuffer,"LOADTYPE:",9))
                {
                ae->bmLoadType = atoi(&LineBuffer[9]);  // Sets for GIF or BBM
                break;
                }

            break;
        }
    }

CheckObjects();
nOperation = OP_NORMAL;
SetOperationCursor();
return(result);
}

#define WL_TEXT         1   // Simple text line
#define WL_COMMAND      2   // Text: number
#define WL_BITMAP       3   // #,# bitmap entries
#define WL_ONOFF        4   // Text: ON or OFF


//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
short WriteLine(short nType,short nParm1,short nParm2,short handle,long *rba,char *szText)
{
    short   len;
    char    szString[20];

switch (nType)
    {
    case WL_TEXT:
        sprintf(LineBuffer,"%s\r\n",szText);
        break;

    case WL_COMMAND:
        sprintf(LineBuffer,"%s %d\r\n",szText,nParm1);
        break;

    case WL_BITMAP:
        sprintf(LineBuffer,"%d,%d\r\n",nParm1,nParm2);
        break;

    case WL_ONOFF:
        if (nParm1)
            strcpy(szString,"ON");
        else
            strcpy(szString,"OFF");

        sprintf(LineBuffer,"%s %s\r\n",szText,szString);
        break;

    default:
        break;
    }

len = strlen(LineBuffer);
*rba += len;
if (_lwrite(handle,LineBuffer,len) != len)
    return(-1);

return(0);
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
short BuildObjectString(OBJSEQ *pSeq,char *szType,char *szString)
{
    short   j,len,result = 0;
    UCHAR   flags;
    char    szFlags[128];

if (pSeq->bitmaps[0] && pSeq->bmSides)
    {
    result = 1;
    strcpy(szFlags,"0");
    flags = pSeq->flags;
    if (flags & OF_PASSABLE)
        strcat(szFlags,"|PASSABLE");
    if (flags & OF_ANIMATE)
        strcat(szFlags,"|ANIMATE");
    if (flags & OF_MOVEABLE)
        strcat(szFlags,"|MOVEABLE");
    if (pSeq->bmSides > 1)
        strcat(szFlags,"|MULTIVIEW");
    if (flags & OF_ANIMONCE)
        strcat(szFlags,"|SHOWONCE");

    wsprintf(szString,"%s: %s,%d,%d",szType,szFlags,pSeq->bmSides,
            pSeq->bmBitmapsPerView);
    len = pSeq->bmSides * pSeq->bmBitmapsPerView;
    if (len > MAX_OBJ_BITMAPS)
        len = MAX_OBJ_BITMAPS;
    for (j = 0; j < len; j++)
        {
        if (!pSeq->bitmaps[j])
            break;
        wsprintf(szFlags,",%d",pSeq->bitmaps[j]);
        strcat(szString,szFlags);
        }
    }

return(result);
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
short SaveMap(short handle,long *rbaPos)
{
    short   count,i;
    long    rba;
    int     mLen,aLen;
    UCHAR   *mPtr;

// Write the map grids below, keeping track of the Relative Byte Address

aLen = GRID_ARRAY * 2;
mLen = GRID_MAX * 2;
rba = 0;

if (_lwrite(handle,(char *)Grid,mLen) != mLen)
    return(-2);

rba += mLen;

if (_lwrite(handle,(char *)ObjGrid,mLen) != mLen)
    return(-2);

rba += mLen;

if (_lwrite(handle,(char *)ae->xGrid,aLen) != aLen)
    return(-2);

rba += aLen;
if (_lwrite(handle,(char *)ae->yGrid,aLen) != aLen)
    return(-2);

rba += aLen;

if (_lwrite(handle,(char *)FloorMap,mLen) != mLen)
    return(-2);

rba += mLen;

if (_lwrite(handle,(char *)CeilMap,mLen) != mLen)
    return(-2);

rba += mLen;
count = 0;

// Write the initial count of multi-height walls
*rbaPos += rba;
rba = *rbaPos;

_lwrite(handle,(char *)&count,2);

for (i = 0; i < GRID_MAX; i++)
    {
    mPtr = ae->mxGrid[i];
    if (mPtr)
        {
        count++;
        _lwrite(handle,(char *)&i,2);          // Write the position of this wall
        _lwrite(handle,(char *)mPtr,MAX_MULTI);  // Write the bitmap numbers for this wall

    *rbaPos += (MAX_MULTI+2);
    i++;
        }
    }

if (count)
    {
    _llseek(handle,rba,SEEK_SET);           // Seek back to the count
    _lwrite(handle,(char *)&count,2);       // And write the new count
    _llseek(handle,*rbaPos,SEEK_SET);
    }


return(0);
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
short TransferFile(short handle,short InHandle,short index,long *rba)
{
    unsigned short  len,rlen,LenToRead;
    unsigned long   ulen;
    UCHAR   *pBuffer;

if (!index)
    return(-1);

LenToRead = 32768;
pBuffer = (UCHAR *)AckMalloc(LenToRead);
if (pBuffer == NULL)
    return(-2);

ulen = (ULONG)((ULONG)TempRBA[index+1] - (ULONG)TempRBA[index]);
len = (unsigned short)ulen;
_llseek(InHandle,TempRBA[index],SEEK_SET);

while (len > 0)
    {
    if (len < LenToRead)
        LenToRead = len;

    rlen = _lread(InHandle,pBuffer,LenToRead);
    if (rlen)
        {
        _lwrite(handle,(const char*)pBuffer,rlen);
        *rba += rlen;
        }

    len -= rlen;

    if (!rlen)
        break;
    }

AckFree(pBuffer);
return(0);
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
short TransferFileName(short handle,char *Name,long *rba)
{
    unsigned short  len,rlen,LenToRead;
    unsigned long   ulen;
    short   FileHandle;
    UCHAR   *pBuffer;

LenToRead = 32768;
pBuffer = (UCHAR *)AckMalloc(LenToRead);
if (pBuffer == NULL)
    return(-2);

FileHandle = _lopen(Name,OF_READ);
if (FileHandle == HFILE_ERROR)
    {
    AckFree(pBuffer);
    return(-1);
    }

while (1)
    {
    rlen = _lread(FileHandle,pBuffer,LenToRead);
    if (rlen)
        {
        _lwrite(handle,(const char*)pBuffer,rlen);
        *rba += rlen;
        }

    if (!rlen)
        break;
    }

_lclose(FileHandle);

AckFree(pBuffer);
return(0);

}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
long SaveInfoFile(short handle,long rba)
{
    short   i,index,bType;
    short   speed;
    BOOL    bGotMemory = FALSE;
    char    szString[256];
    NEWOBJECT   *pObj;
    OBJSEQ      *pSeq;


index = 1;
WriteLine(WL_TEXT,0,0,handle,&rba,";Info file created by AckEdit for Windows");

WriteLine(WL_COMMAND,index++,0,handle,&rba,"MapFile:");
WriteLine(WL_COMMAND,ae->xPlayer,0,handle,&rba,"xPlayer:");
WriteLine(WL_COMMAND,ae->yPlayer,0,handle,&rba,"yPlayer:");
WriteLine(WL_COMMAND,ae->PlayerAngle,0,handle,&rba,"PlayerAngle:");
WriteLine(WL_COMMAND,ae->TopColor,0,handle,&rba,"TopColor:");
WriteLine(WL_COMMAND,ae->BottomColor,0,handle,&rba,"BottomColor:");

WriteLine(WL_ONOFF,ae->LightFlag,0,handle,&rba,"Shading:");
WriteLine(WL_ONOFF,!(ae->SysFlags & SYS_SOLID_FLOOR),0,handle,&rba,"Floors:");
WriteLine(WL_ONOFF,!(ae->SysFlags & SYS_SOLID_CEIL),0,handle,&rba,"Ceiling:");
WriteLine(WL_ONOFF,ae->SysFlags & SYS_SINGLE_BMP,0,handle,&rba,"SingleBitmap:");
WriteLine(WL_COMMAND,ae->CeilBitmap,0,handle,&rba,"CeilBitmap:");
WriteLine(WL_COMMAND,ae->FloorBitmap,0,handle,&rba,"FloorBitmap:");

WriteLine(WL_COMMAND,index++,0,handle,&rba,"ScrollBack:");
WriteLine(WL_TEXT,0,0,handle,&rba,"; Wall definition section");
WriteLine(WL_TEXT,0,0,handle,&rba,"Walls:");
WriteLine(WL_TEXT,0,0,handle,&rba,"Bitmaps:");

bType = BMLOAD_BBM;

for (i = 1; i < MAX_WALLBMPS; i++)
    {
    if (ae->bMaps[i] != NULL)
        {
        if (bMapType[i] != bType)
            {
            bType = bMapType[i];
            WriteLine(WL_COMMAND,bType,0,handle,&rba,"LoadType:");
            }
        WriteLine(WL_BITMAP,i,index++,handle,&rba,0);
        }
    }

WriteLine(WL_TEXT,0,0,handle,&rba,"EndBitmaps:");
WriteLine(WL_TEXT,0,0,handle,&rba,"EndWalls:");
WriteLine(WL_TEXT,0,0,handle,&rba,"; Object definition section");
WriteLine(WL_TEXT,0,0,handle,&rba,"Objects:");
WriteLine(WL_TEXT,0,0,handle,&rba,"Bitmaps:");

for (i = 1; i < MAX_OBJBMPS; i++)
    {
    if (ae->oMaps[i] != NULL)
        {
        if (oMapType[i] != bType)
            {
            bType = oMapType[i];
            WriteLine(WL_COMMAND,bType,0,handle,&rba,"LoadType:");
            }
        WriteLine(WL_BITMAP,i,index++,handle,&rba,0);
        }
    }

WriteLine(WL_TEXT,0,0,handle,&rba,"EndBitmaps:");
WriteLine(WL_TEXT,0,0,handle,&rba,"; Object Number: ID,speed");
WriteLine(WL_TEXT,0,0,handle,&rba,"; followed by Sequence,flags,sides,bpv,bitmaps");
WriteLine(WL_TEXT,0,0,handle,&rba,"ObjDesc:");

for (i = 1; i < MAX_OBJECTS; i++)
    {
    pObj = ae->ObjList[i];
    if (pObj != NULL)
        {
        speed = pObj->Speed;
        wsprintf(szString,"Number: %d,%d",i,speed);
        WriteLine(WL_TEXT,0,0,handle,&rba,szString);
        if (BuildObjectString(&pObj->Create,"Create",szString))
            WriteLine(WL_TEXT,0,0,handle,&rba,szString);
        if (BuildObjectString(&pObj->Destroy,"Destroy",szString))
            WriteLine(WL_TEXT,0,0,handle,&rba,szString);
        if (BuildObjectString(&pObj->Walk,"Walk",szString))
            WriteLine(WL_TEXT,0,0,handle,&rba,szString);
        if (BuildObjectString(&pObj->Attack,"Attack",szString))
            WriteLine(WL_TEXT,0,0,handle,&rba,szString);
        if (BuildObjectString(&pObj->Interact,"Interact",szString))
            WriteLine(WL_TEXT,0,0,handle,&rba,szString);

        }
    }

WriteLine(WL_TEXT,0,0,handle,&rba,"EndDesc:");
WriteLine(WL_TEXT,0,0,handle,&rba,"EndObjects:");
WriteLine(WL_TEXT,0,0,handle,&rba,"End:");

return(rba);
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
short SaveTempInfo(HWND hWnd)
{
    short   handle;
    long    rba;

handle = _lcreat("ACKTEMP.ATF",0);
if (handle == HFILE_ERROR)
    {
    MessageBox(hWnd,"Error creating temporary resource!","ERROR",MB_OK);
    return(-1);
    }

rba = 0;
SaveInfoFile(handle,rba);

_lclose(handle);
return(0);
}


//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
short SaveResource(HWND hWnd,char *szName)
{
    short   i,handle,InHandle,index;
    short   speed;
    long    rba;
    BOOL    bGotMemory = FALSE;
    char    szString[256];
    NEWOBJECT   *pObj;
    OBJSEQ      *pSeq;

// Create a temporary file to transfer from old resource to new
// resource (which could be the same name).
handle = _lcreat("ACKTEMP.ATF",0);
if (handle == HFILE_ERROR)
    {
    MessageBox(hWnd,"Error creating temporary resource!","ERROR",MB_OK);
    return(-1);
    }

rba = sizeof(long) * 500;
if (rbaTable == NULL)
    {
    rbaTable = (ULONG *)AckMalloc(rba);
    bGotMemory = TRUE;
    }

if (rbaTable == NULL)
    {
    MessageBox(hWnd,"Out of memory!","ERROR",MB_OK);
    return(-2);
    }

nOperation = OP_WAIT;
SetOperationCursor();
memset(rbaTable,0,rba);
rbaTable[0] = rba;
_lwrite(handle,(char *)rbaTable,rba);

rba = SaveInfoFile(handle,rba);

InHandle = _lopen(ResFileName,OF_READ);
if (InHandle == HFILE_ERROR)
    {
    MessageBox(hWnd,"No resource to transfer from!","Error",MB_OK);
    _lclose(handle);
    unlink("ACKTEMP.ATF");
    return(-3);
    }

_lread(InHandle,(char *)TempRBA,sizeof(long) * 500);

index = 1;
rbaTable[index++] = rba;    // Map file is at index 1

SaveMap(handle,&rba);

rbaTable[index++] = rba;    // Scrolling bkgnd at index 2

if (!BackGndFile[0])
    TransferFile(handle,InHandle,ResScrollBack,&rba);
else
    TransferFileName(handle,BackGndFile,&rba);

// Wall bitmaps start at index 3
for (i = 1; i < MAX_WALLBMPS; i++)
    {
    if (ae->bMaps[i] != NULL)
        {
        rbaTable[index++] = rba;

        if (bMapFiles[i] == NULL)
            {
            TransferFile(handle,InHandle,bMapIndex[i],&rba);
            }
        else
            {
            TransferFileName(handle,bMapFiles[i],&rba);
            }

        }
    }

for (i = 1; i < MAX_OBJBMPS; i++)
    {
    if (ae->oMaps[i] != NULL)
        {
        rbaTable[index++] = rba;

        if (oMapFiles[i] == NULL)
            {
            TransferFile(handle,InHandle,oMapIndex[i],&rba);
            }
        else
            {
            TransferFileName(handle,oMapFiles[i],&rba);
            }
        }
    }

rbaTable[index] = rba;      // Save last one for later length checking

_llseek(handle,0L,SEEK_SET);
_lwrite(handle,(char *)rbaTable,sizeof(long) * 500);
_lclose(handle);
_lclose(InHandle);

unlink(szName);
rename("ACKTEMP.ATF",szName);

if (bGotMemory)
    {
    AckFree(rbaTable);
    rbaTable = NULL;
    }

ClearAddinFiles();

// Now we need to adjust the saved resource indexes for the new file
// we just saved.
index = 3;
for (i = 1; i < MAX_WALLBMPS; i++)
    {
    if (ae->bMaps[i] != NULL)
        bMapIndex[i] = index++;
    }

for (i = 1; i < MAX_OBJBMPS; i++)
    {
    if (ae->oMaps[i] != NULL)
        oMapIndex[i] = index++;
    }

nOperation = OP_NORMAL;
SetOperationCursor();
bModified = FALSE;
return(0);
}


//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
short ACKFileOpen(HWND hWnd,char *szName,short nMaxLen,char *szFileList,
                    char *szTitle,char *szExt)
{
  OPENFILENAME ofnTemp;
  DWORD Errval; // Error value
  char buf[5];  // Error buffer
  char Errstr[50]="ACKFileOpen returned Error #";
  ofnTemp.lStructSize = sizeof( OPENFILENAME );
  ofnTemp.hwndOwner = hWnd; // An invalid hWnd causes non-modality
  ofnTemp.hInstance = 0;
  ofnTemp.lpstrFilter = (LPSTR)szFileList;  // See previous note concerning string
  ofnTemp.lpstrCustomFilter = NULL;
  ofnTemp.nMaxCustFilter = 0;
  ofnTemp.nFilterIndex = 1;
  ofnTemp.lpstrFile = (LPSTR)szName;  // Stores the result in this variable
  ofnTemp.nMaxFile = nMaxLen; //sizeof( szName );
  ofnTemp.lpstrFileTitle = NULL;
  ofnTemp.nMaxFileTitle = 0;
  ofnTemp.lpstrInitialDir = NULL;
  ofnTemp.lpstrTitle = szTitle;  // Title for dialog
  ofnTemp.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
  ofnTemp.nFileOffset = 0;
  ofnTemp.nFileExtension = 0;
  ofnTemp.lpstrDefExt = (LPSTR)szExt;
  ofnTemp.lCustData = NULL;
  ofnTemp.lpfnHook = NULL;
  ofnTemp.lpTemplateName = NULL;

  if(GetOpenFileName( &ofnTemp ) != TRUE)
      {
      Errval=CommDlgExtendedError();
      if (Errval != 0) // 0 value means user selected Cancel
          {
          wsprintf(buf,"%ld",Errval);
          strcat(Errstr,buf);
          MessageBox(hWnd,Errstr,"WARNING",MB_OK|MB_ICONSTOP);
          }

      return(-1);
      }

return(0);
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
short ACKFileSave(HWND hWnd,char *szName,short nMaxLen,char *szFileList,
                    char *szTitle)
{
  OPENFILENAME ofnTemp;
  DWORD Errval; // Error value
  char buf[5];  // Error buffer
  char Errstr[50]="ACKFileSave returned Error #";
  ofnTemp.lStructSize = sizeof( OPENFILENAME );
  ofnTemp.hwndOwner = hWnd; // An invalid hWnd causes non-modality
  ofnTemp.hInstance = 0;
  ofnTemp.lpstrFilter = (LPSTR)szFileList;  // See previous note concerning string
  ofnTemp.lpstrCustomFilter = NULL;
  ofnTemp.nMaxCustFilter = 0;
  ofnTemp.nFilterIndex = 1;
  ofnTemp.lpstrFile = (LPSTR)szName;  // Stores the result in this variable
  ofnTemp.nMaxFile = nMaxLen; //sizeof( szName );
  ofnTemp.lpstrFileTitle = NULL;
  ofnTemp.nMaxFileTitle = 0;
  ofnTemp.lpstrInitialDir = NULL;
  ofnTemp.lpstrTitle = szTitle;  // Title for dialog
  ofnTemp.Flags = OFN_SHOWHELP|OFN_OVERWRITEPROMPT;
  ofnTemp.nFileOffset = 0;
  ofnTemp.nFileExtension = 0;
  ofnTemp.lpstrDefExt = "dtf";
  ofnTemp.lCustData = NULL;
  ofnTemp.lpfnHook = NULL;
  ofnTemp.lpTemplateName = NULL;

  if (GetSaveFileName(&ofnTemp) != TRUE)
      {
      Errval=CommDlgExtendedError();
      if (Errval != 0) // 0 value means user selected Cancel
          {
          wsprintf(buf,"%ld",Errval);
          strcat(Errstr,buf);
          MessageBox(hWnd,Errstr,"WARNING",MB_OK|MB_ICONSTOP);
          }

      return(-1);
      }

return(0);
}



