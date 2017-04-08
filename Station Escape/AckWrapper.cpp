// AckWrapper.cpp - nice C++ class wrapper for ACK-3D.
// (c) 2012 Mark Hutcheson

#include "AckWrapper.h"
#include <fstream>

//This is required for my stuff in ACKRTNRecode.c. And elsewhere.
ACKENG* ae;
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

//Default object collision function
void DoNothing(NEWOBJECT* pObj, UCHAR nIndex, short nResult)
{
    return;
}

void AckWrapper::ReadInPallette()
{
    std::ifstream infile("pallette.txt");
    if(infile.fail())
        return; //Uh-oh!
    for(int i = 0; i < 256; i++)
    {
        unsigned int r, g, b;
        PalletteSlot p = {0, 0, 0};
        infile >> r >> g >> b;
        p.r = (unsigned char)r;
        p.g = (unsigned char)g;
        p.b = (unsigned char)b;
        p.a = 255;
        m_pallette[i] = p;
    }
    infile.close();
}

//Opens sFilename as a resource file and sets up ACK-3D
AckWrapper::AckWrapper(string sFilename)
{
    // Prime the ACK-3D engine with our startup parameters
    if (InitACKEngine())
      return;

    // Open our default resource file and load bitmaps, etc.
    AckOpenResource((char*)sFilename.c_str());
    int result = AckProcessInfoFile();
    if (result)
    {
      printf("ACK-3D Error: %d in resource file",result);
      AckCloseResource();
      return;
    }
    LoadBackDrop();
    AckCloseResource();
    AckSetupPalRanges(ae, ranges);

    //Create memory for offscreen buffer
    m_OffscreenBuffer = (unsigned char*)malloc(ACK_VIEWWIDTH * ACK_VIEWHEIGHT * ACK_BYTES_PER_PIXEL);
    //Read in pallette data
    ReadInPallette();

    //Assign default object collision function
    m_ObjectCollide = DoNothing;
}

//Destructor for AckWrapper class: cleans up ACK-3D and frees up memory
AckWrapper::~AckWrapper()
{
    AckWrapUp(ae);
    free(m_OffscreenBuffer);
}

const unsigned char* AckWrapper::RenderView()
{
    AckBuildView();             // Render the 3D view
    unsigned char* pBufferData = m_OffscreenBuffer;
    for(int r = 0; r < ACK_VIEWWIDTH; r++)
    {
        for(int c = 0; c < ACK_VIEWHEIGHT; c++) //AHAHA JOKE
        {
            *pBufferData++ = m_pallette[ae->ScreenBuffer[c+ACK_VIEWHEIGHT*r]].r;
            *pBufferData++ = m_pallette[ae->ScreenBuffer[c+ACK_VIEWHEIGHT*r]].g;
            *pBufferData++ = m_pallette[ae->ScreenBuffer[c+ACK_VIEWHEIGHT*r]].b;
            *pBufferData++ = 255; //Alpha = 255 by default. You could probably do crazy stuff with this, but I'm not going to.
        }
    }
    return m_OffscreenBuffer;
}

void AckWrapper::Update()
{
    AckCheckObjectMovement();   // Check object animation

    //And move all the objects
    UCHAR   Index;
    short   nResult;
    NEWOBJECT   *pObj;

    for (Index = 0; Index < ae->MaxObjects; Index++)
    {
        pObj = ae->ObjList[Index];
        //If this object is movable, move it
        if (pObj != NULL && pObj->Active && pObj->Flags & OF_MOVEABLE)
        {
            nResult = AckMoveObjectPOV(Index,pObj->Dir,pObj->Speed);
            //And call our callback function, whether it hit something or not (so we can have AI in the application easier)
            m_ObjectCollide(pObj, Index, nResult);
        }
    }


    //MoveObjects();              // Application object movement
}

#include <string.h>
short AckWrapper::InitACKEngine()
{
    short   result;

ae = (ACKENG *)AckMalloc(sizeof(ACKENG));
if (ae == NULL)
    {
    printf("Unable to allocate memory for ACK-3D\n");
    return(-1);
    }
memset(ae,0,sizeof(ACKENG));

ae->WinStartX = 0;
ae->WinStartY = 0;
ae->WinEndX = ACK_VIEWWIDTH-1;
ae->WinEndY = ACK_VIEWHEIGHT-1;

ae->LightFlag = SHADING_OFF;
ae->xPlayer = 192;
ae->yPlayer = 640;
ae->PlayerAngle = 0;
ae->TopColor = 0;
ae->BottomColor = 24;
ae->DoorSpeed = 6;
ae->NonSecretCode = 1;
ae->FloorBitmap = 50;
ae->CeilBitmap = 47;

ae->PlayerHeight = DEFAULT_PLAYER_HEIGHT;

result = AckOpenResource((char*)"KIT.OVL");
if (result)
    {
    printf("ACK-3D Error: Cannot open KIT.OVL\n");
    return(-1);
    }

result = AckInitialize(ae);
if (result)
    {
    printf("ACK-3D Error: Unable to initialize ACK engine\n");
    return(-1);
    }

AckCloseResource();

AckRegisterStructure(ae);

return(0);
}

void  AckWrapper::SetLightShading(int iOnOff)
{
    bool    bSet = false;

    if (iOnOff == ACK_LIGHTSHADING_ON && ae->LightFlag == SHADING_OFF)
      {
      ae->LightFlag = SHADING_ON;
      bSet = true;
      }

    if (iOnOff == ACK_LIGHTSHADING_OFF && ae->LightFlag == SHADING_ON)
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

void AckWrapper::OpenDoorForPlayer()
{
    AckCheckDoorOpen(ae->xPlayer,ae->yPlayer,ae->PlayerAngle);
}

//Add a positive or negative value to the player's angle.
void AckWrapper::AddToPlayerAngle(short sVal)
{
    ae->PlayerAngle += sVal;
    //Check for wraparound
    if(ae->PlayerAngle >= INT_ANGLE_360)
        ae->PlayerAngle -= INT_ANGLE_360;
    if(ae->PlayerAngle < 0)
        ae->PlayerAngle += INT_ANGLE_360;
}

//Returns ACK_LIGHTSHADING_ON or ACK_LIGHTSHADING_OFF
int AckWrapper::GetLightShading()
{
    if(ae->LightFlag == SHADING_OFF)
        return ACK_LIGHTSHADING_OFF;
    return ACK_LIGHTSHADING_ON;
}

//Move the player by Amount, adding sAddAngle to the player's current angle if so desired
short AckWrapper::MovePlayer(short sAmount, short sAddAngle)
{
    short sPrevPlayerAngle = ae->PlayerAngle;
    AddToPlayerAngle(sAddAngle);
    short sResult = AckMovePOV(ae->PlayerAngle,sAmount);
    ae->PlayerAngle = sPrevPlayerAngle;
    return sResult;
}

//------------------------------------------------------
//----------ACKINFO.CPP
//------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#define BUILD_FOR_LINUX 0

#if BUILD_FOR_LINUX
    #include <sys/types.h>
    #include <unistd.h>
    #include <sys/io.h>
    #include <ctype.h>
    #define O_BINARY 0
    #define stricmp strcasecmp
    #define strnicmp strncasecmp
#else
    #include <io.h>
#endif

#define LSEEK lseek
#define LREAD read

extern "C" {
extern  short   rsHandle;
extern  ULONG   *rbaTable;
extern  short   Resolution;
};

static int     ResScrollBack;
static char    LineBuffer[ACK_VIEWHEIGHT];
static short   LastObjectIndex;
static int     MapResource;

//MEH Need to make function in place of strupr()
#if BUILD_FOR_LINUX
void strupr(char* cString)
{
    char *p = cString;
    while (*p)
    {
        *p = toupper(*p);
        ++p;
    }
}
#endif

int AckWrapper::AckProcessInfoFile()
{
    int     result;
    int     mode;
    int32_t    pos;

//LineNumber = 0;
// Position to start of info file within resource file
LSEEK(rsHandle,(int)(rbaTable[0]),SEEK_SET);

mode = result = 0;

while (!result)
    {
//    LineNumber++;
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
                ae->bmLoadType = (short)atoi(&LineBuffer[9]);   // Sets for GIF or BBM
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
                ae->bmLoadType = (short)atoi(&LineBuffer[9]);
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
                pos = LSEEK(rsHandle,0,SEEK_CUR);
                result = AckReadMapFile(ae,(char *)MapResource);
                LSEEK(rsHandle,(int)(pos),SEEK_SET);
                break;
                }

            if (!strnicmp(LineBuffer,"PALFILE:",8))
                {
//                PalResource = atoi(&LineBuffer[8]);
                break;
                }

            if (!strnicmp(LineBuffer,"XPLAYER:",8))
                {
                ae->xPlayer = (short)atoi(&LineBuffer[8]);
                break;
                }

            if (!strnicmp(LineBuffer,"YPLAYER:",8))
                {
                ae->yPlayer = (short)atoi(&LineBuffer[8]);
                break;
                }

            if (!strnicmp(LineBuffer,"PLAYERANGLE:",12))
                {
                ae->PlayerAngle = (short)atoi(&LineBuffer[12]);
                break;
                }

            if (!strnicmp(LineBuffer,"SCREENBACK:",11))
                {
//                ResScreenBack = atoi(&LineBuffer[11]);
                break;
                }

            if (!strnicmp(LineBuffer,"Scrollback:",11))
                {
                ResScrollBack = atoi(&LineBuffer[11]);
                break;
                }

            if (!strnicmp(LineBuffer,"TOPCOLOR:",9))
                {
                ae->TopColor = (short)atoi(&LineBuffer[9]);
                break;
                }

            if (!strnicmp(LineBuffer,"BOTTOMCOLOR:",12))
                {
                ae->BottomColor = (short)atoi(&LineBuffer[12]);
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
                ae->CeilBitmap = (short)atoi(&LineBuffer[11]);
                break;
                }

            if (!strnicmp(LineBuffer,"FLOORBITMAP:",12))
                {
                ae->FloorBitmap = (short)atoi(&LineBuffer[12]);
                break;
                }


            if (!strnicmp(LineBuffer,"RESOLUTION:",11))
                {
                Resolution = (short)atoi(&LineBuffer[11]);
                break;
                }

            if (!strnicmp(LineBuffer,"LOADTYPE:",9))
                {
                ae->bmLoadType = (short)atoi(&LineBuffer[9]);   // Sets for GIF or BBM
                break;
                }

            break;
        }
    }

return(result);
}

//=============================================================================
// Reads a text line from the resource file
//=============================================================================
int AckWrapper::ReadLine(void)
{
    int     len;
    char    ch;

len = 0;
while (len < ACK_VIEWHEIGHT)
    {
    if (LREAD(rsHandle,&LineBuffer[len],1) != 1)
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

//----------------------------------------------------------------------------
// Skips to the next parameter in a text line
//----------------------------------------------------------------------------
char *AckWrapper::GetNextParm(char *s)
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

//----------------------------------------------------------------------------
// Loads a wall bitmap specified in info file
//----------------------------------------------------------------------------
int AckWrapper::LoadWall(void)
{
    int     wnum,rnum,result;
    int32_t    pos;
    char    *lb;


lb = LineBuffer;            // Info file buffer
wnum = atoi(lb);            // Wall number to load into

lb = GetNextParm(lb);

if (lb == NULL)
    return(-1);

rnum = atoi(lb);            // Resource number

pos = LSEEK(rsHandle,0,SEEK_CUR);
result = AckLoadWall(ae,(short)wnum,(char *)rnum);
LSEEK(rsHandle,(int)(pos),SEEK_SET);

return(result);
}


//----------------------------------------------------------------------------
// Loads an object bitmap specified in info file
//----------------------------------------------------------------------------
int AckWrapper::LoadObject(void)
{
    int     onum,rnum,result;
    int32_t    pos;
    char    *lb;

lb = LineBuffer;
onum = atoi(lb);            // Object bitmap number

lb = GetNextParm(lb);

if (lb == NULL)
    return(-2);

rnum = atoi(lb);            // Resource number
pos = LSEEK(rsHandle,0,SEEK_CUR);
result = AckLoadObject(ae,(short)onum,(char *)rnum);
LSEEK(rsHandle,(int)(pos),SEEK_SET);

return(result);
}


//----------------------------------------------------------------------------
// Skip any leading spaces in the string
// NOTE: Actually modifies the string passed!
//----------------------------------------------------------------------------
char *AckWrapper::SkipSpaces(char *s)
{

while (*s == ' ' || *s == '\t' || *s == ',')
    s++;//strcpy(s,&s[1]);

return(s);
}


//----------------------------------------------------------------------------
// Creates and object of the desired style
//----------------------------------------------------------------------------
int AckWrapper::CreateObject(void)
{
    short   onum,vnum;
    short   result,oType;
    short   NumViews,bmPerView;
    USHORT  flags;
    char    *lb;
    OBJSEQ  os;

lb = LineBuffer;

if (!strnicmp(lb,"NUMBER:",7))
    {
    lb = &lb[7];
    onum = (short)atoi(lb);
    if (onum < 1 || onum > MAX_OBJECTS)
        return(-3);

    result = AckCreateObject(ae,(short)onum);
    if (result)
        return(result);

    LastObjectIndex = onum;
    lb = GetNextParm(lb);
    if (lb == NULL)
        return(-4);

    ae->ObjList[onum]->Speed = (short)atoi(lb);
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

NumViews = (short)atoi(lb);

if (NumViews < 1)
    return(-6);

lb = GetNextParm(lb);
if (lb == NULL)
    return(-7);

bmPerView = (short)atoi(lb);

if (bmPerView < 1)
    return(-7);

vnum = (short)(NumViews * bmPerView);

if (vnum > MAX_OBJ_BITMAPS)
    return(-8);

lb = GetNextParm(lb);
if (lb == NULL)
    return(-9);

vnum = 0;

while (lb != NULL && vnum < MAX_OBJ_BITMAPS)
    {
    os.bitmaps[vnum++] = (short)atoi(lb);
    lb = GetNextParm(lb);
    }

os.bmBitmapsPerView = bmPerView;
os.flags = flags;
os.MaxBitmaps = bmPerView;
os.bmSides = NumViews;

result = (short)AckSetupObject(ae,onum,oType,&os);

return(result);
}

int AckWrapper::LoadBackDrop()
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

extern  UCHAR   *BackArray[];

void AckWrapper::ProcessBackDrop(UCHAR *bPtr)
{
        int     i,j,pos;
        UCHAR   *aPtr;
for (i = 0; i < ACK_VIEWWIDTH; i++)
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
        pos += ACK_VIEWWIDTH;
        }
    }

for (i = ACK_VIEWWIDTH; i < 640; i++)
    {
    pos = (i - ACK_VIEWWIDTH) + 32004;
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
        pos += ACK_VIEWWIDTH;
        }
    }

}

//---------------------------------------------------------
//
// Misc helper functions by MEH
//
//---------------------------------------------------------

//Wrap an angle to stay within 0 - 359 degree range
short WrapAngle(short sAngle)
{
    if(sAngle >= INT_ANGLE_360)
        sAngle -= INT_ANGLE_360;
    if(sAngle < 0)
        sAngle += INT_ANGLE_360;
    return sAngle;
}

//Gets the map position of x and y, down to the grid square
short GetMapPosn(short x, short y)
{
    return((short)((y & GRID_MASK) + (x >> 6)));
}





