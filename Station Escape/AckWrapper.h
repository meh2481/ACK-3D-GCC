//AckWrapper.h - nice C++ wrapper class for ACK-3D
// (c) 2012 Mark Hutcheson

#ifndef ACKWRAPPER_H
#define ACKWRAPPER_H

//-------Begin: Header file for ACK3D engine interface. Ripped from Station Escape example program.

/* USED TO RID OURSELVES OF THE MANY CASTING PROBLEMS */
#define CAST(t,f) (t)(f)
#include <cstdlib>  //MEH for size_t
#include <stdint.h>
typedef uint32_t	ULONG;
typedef unsigned short	USHORT;
typedef unsigned char	UCHAR;

/* Error codes returned from Ack... functions */
#define ERR_BADFILE	    100
#define ERR_BADCOMMAND	    101
#define ERR_BADOBJNUMBER    102
#define ERR_BADSYNTAX	    103
#define ERR_LOADINGBITMAP   104
#define ERR_BADDIRECTION    105
#define ERR_BADSTARTX	    106
#define ERR_BADSTARTY	    107
#define ERR_BADANGLE	    108
#define ERR_BADMAPFILE	    109
#define ERR_READINGMAP	    110
#define ERR_BADPICNAME	    111
#define ERR_INVALIDFORM	    112
#define ERR_NOPBM	    113
#define ERR_BADPICFILE	    114
#define ERR_NOMEMORY	    115
#define ERR_BADPALFILE	    116
#define ERR_BADWINDOWSIZE   117
#define ERR_TOMANYVIEWS	    118
#define ERR_BADOBJECTNUM    119
#define ERR_BADOBJTYPE	    120

#define INT_ANGLE_1	5
#define INT_ANGLE_2	10
#define INT_ANGLE_4	20
#define INT_ANGLE_6	30
#define INT_ANGLE_32	160
#define INT_ANGLE_45	225
#define INT_ANGLE_90	450
#define INT_ANGLE_135	675
#define INT_ANGLE_180	900
#define INT_ANGLE_225	1125
#define INT_ANGLE_270	1350
#define INT_ANGLE_315	1575
#define INT_ANGLE_360	1800

#define BITMAP_WIDTH	64
#define BITMAP_HEIGHT	64
#define BITMAP_SHIFT	6		    /* Bits to shift for bitmap width */
#define BITMAP_SIZE	(BITMAP_WIDTH * BITMAP_HEIGHT)

#define GRID_MASK	0xFFC0		    /* Use FF80 for 128 and FFC0 for 64 */

#define GRID_SIZE	64
#define GRID_WIDTH	64
#define GRID_HEIGHT	64
#define GRID_MAX	GRID_WIDTH * GRID_HEIGHT
#define GRID_XMAX	BITMAP_WIDTH * GRID_WIDTH
#define GRID_YMAX	BITMAP_WIDTH * GRID_HEIGHT

#define GRID_XMAXLONG	(GRID_XMAX * FP_MULT)
#define GRID_YMAXLONG	(GRID_YMAX * FP_MULT)

#define GRID_ARRAY	(GRID_WIDTH+2) * (GRID_HEIGHT+2)

#define MAX_WALLBMPS	256		    /* Total wall bitmaps allowed */
#define MAX_OBJBMPS	256		    /* Total obj bitmaps allowed */

#define MAX_OBJECTS	254		    /* Total objects in map */

#define MAX_ZONES	8		    /* Number of light zones */
#define PAL_SIZE	4096		    /* Shading palette ranges */

#define MAX_VIEWS	47		    /* Total sides to an object */

#define MAX_MULTI	3		    /* Max height for multi-ht walls */

#define DEFAULT_DOOR_SPEED  2
#define MAX_DOORS	    10		    /* Max opening, closing at one time */

#define DOOR_TYPE_SECRET    0x8000
#define DOOR_LOCKED	    0x4000
#define DOOR_TYPE_SLIDE	    0x2000
#define DOOR_TYPE_SPLIT	    0x1000
#define WALL_TYPE_TRANS	    0x0800	    /* Transparent wall */
#define WALL_TYPE_MULTI	    0x0400	    /* Wall is 1.5 times high */
#define WALL_TYPE_UPPER	    0x0200	    /* Wall is above floor level */
#define WALL_TYPE_PASS	    0x0100	    /* Wall can be walked through */

#define DOOR_XCODE	    60		    /* Map codes for the various doors */
#define DOOR_SIDECODE	    61
#define DOOR_YCODE	    62

#define TYPE_WALL	0
#define TYPE_OBJECT	1

/*
These defines used in object flags
*/
#define OF_PASSABLE	0x80		    /* Object can be walked thru */
#define OF_ANIMATE	0x40		    /* Object bitmaps are animated */
#define OF_MOVEABLE	0x20		    /* Object will move X,Y */
#define OF_MULTIVIEW	0x10		    /* Object has multiple sides */
#define OF_ANIMONCE	0x08		    /* Animate once then stop */
#define OF_ANIMDONE	0x04		    /* One shot animation is completed */

/*
    These defines are used in LightFlag
*/
#define SHADING_ON	1		    /* Set if distance shading is on */
#define SHADING_OFF	0

/*
  These defines are used in SysFlags
*/
#define SYS_SOLID_BACK	0x8000		    /* On if solid color bkgd vs picture */
#define SYS_SOLID_FLOOR 0x4000		    /* On if solid vs texture floor */
#define SYS_SOLID_CEIL	0x2000		    /* On if solid vs texture ceiling */
#define SYS_NO_WALLS	0x1000		    /* On if walls are NOT to display */
#define SYS_SINGLE_BMP	0x0800		    /* On if 1 bitmap for floor & ceiling */

#define OBJECT_ACTIVE	1		    /* Active and may be moveable */
#define OBJECT_INACTIVE 0		    /* Won't be considered moveable  */

/*
 These values are returned by AckMovePOV and AckMoveObjectPOV functions
*/
#define POV_NOTHING	0		    /* Nothing was hit */
#define POV_XWALL	1		    /* An X wall was hit */
#define POV_YWALL	2		    /* A Y wall was hit */
#define POV_OBJECT	3		    /* An object was hit */
#define POV_PLAYER	4		    /* The player was hit by object */
#define POV_SLIDEX	5		    /* Sliding on X wall */
#define POV_SLIDEY	6		    /* Sliding on Y wall */
#define POV_NODOOR	0		    /* No door was opened */
#define POV_XDOOR	1		    /* An X door was opened */
#define POV_YDOOR	2		    /* A Y door was opened */
#define POV_XSECRETDOOR 3		    /* An X secret door was opened */
#define POV_YSECRETDOOR 4		    /* A Y secret door was opened */

#define POV_DOORLOCKED	0x80		    /* Bit on if door is locked  */

#define RES_LOW	    1	    /* Resolution is low for walls and floor,ceil */
#define RES_MEDIUM  2	    /* Resolution is low only for floor,ceil */

#define BMLOAD_BBM  0	    /* Bitmaps will be loaded using BBM format */
#define BMLOAD_GIF  1	    /* Bitmaps will be loaded using GIF format */
#define BMLOAD_PCX  2	    /* Bitmaps will be loaded using PCX format */

//MEH defines
#define DEFAULT_PLAYER_HEIGHT   89

/*
Structure for new style objects
*/

#define MAX_OBJ_BITMAPS	    32

/*
These values are the current styles for an object
*/
#define NO_CREATE	    1
#define NO_DESTROY	    2
#define NO_WALK		    3
#define NO_ATTACK	    4
#define NO_INTERACT	    5

typedef struct {
	UCHAR	flags;			    /* Flags for this sequence */
	UCHAR	bitmaps[MAX_OBJ_BITMAPS];   /* Bitmap numbers in this sequence */
	short	bmSides;		    /* Number of views in sequence */
	short	bmBitmapsPerView;	    /* Number of bitmaps in each view */
	short	AngleFactor;
	UCHAR	MaxBitmaps;		    /* Max bitmaps in this sequence */
	} OBJSEQ;

typedef struct {
	char	Active;		    /*0=Inactive, 1=Active */
	UCHAR	Flags;		    /*Misc flags for this object */
	char	Speed;		    /* Speed of obj (used by appl)  */
	short	Dir;		    /* Direction of obj (used by appl) */
	short	x;		    /* Current location in grid               */
	short	y;
	short	mPos;		    /* Current map location in grid */
	UCHAR	id;		    /* Object id */
	short	CurrentType;	    /* Create,Destroy,etc is current */
	UCHAR	*CurrentBitmaps;    /* Current bitmap list */
	short	Sides;		    /* Number of views */
	short	aFactor;	    /* Angle factor */
	short	BitmapsPerView;	    /* Number of bitmaps in each view */
	UCHAR	CurrentBm;	    /* Current bitmap number */
	UCHAR	Maxbm;		    /* Max bitmap number for this view */
	OBJSEQ	Create;
	OBJSEQ	Destroy;
	OBJSEQ	Walk;
	OBJSEQ	Attack;
	OBJSEQ	Interact;
	} NEWOBJECT;


#define DOOR_OPENING	0x80		/* On if door is currently opening */
#define DOOR_CLOSING	0x40		/* On if door is currently closing */


typedef struct {
	    short   mPos;
	    short   mPos1;
	    short   mCode;
	    short   mCode1;
	    UCHAR   ColOffset;
	    char    Speed;
	    char    Type;
	    UCHAR   Flags;
} DOORS;
/*
  Main interface structure used between the application and the ACK engine
  This structure MUST be allocated or defined before any ACK calls are made.
*/
typedef struct {

	    USHORT	xGrid[GRID_ARRAY];  /* Map for X walls */
	    USHORT	yGrid[GRID_ARRAY];  /* Map for Y walls */

	    UCHAR	*mxGrid[GRID_ARRAY]; /* Wall data for multi-height walls */
	    UCHAR	*myGrid[GRID_ARRAY]; /* Wall data for multi-height walls */

	    UCHAR	*bMaps[MAX_WALLBMPS]; /* Pointers to wall bitmaps*/
	    UCHAR	*oMaps[MAX_OBJBMPS];  /* Pointers to object bitmaps  */

	    UCHAR	*ScreenBuffer;	    /* 64k buffer for screen */
	    UCHAR	*OverlayBuffer;	    /* Buffer for compiled overlay */
	    UCHAR	*BkgdBuffer;	    /* Buffer for ceiling,floor */

	    short	xPlayer;	    /* X value from 0 to 4095 */
	    short	yPlayer;	    /* Y value from 0 to 4095 */
	    short	PlayerAngle;	    /* Angle value from 0 to 1919 */

	    short	DoorSpeed;	    /* Door open/close speed */
	    short	NonSecretCode;	    /* Wall code for secret door */

	    UCHAR	TopColor;	    /* Base color of ceiling */
	    UCHAR	BottomColor;	    /* Base color of floor */
	    UCHAR	FloorBitmap;	    /* Bitmap number for single floor */
	    UCHAR	CeilBitmap;	    /* Bitmap number for single ceiling */

	    UCHAR	LightFlag;	    /* 0 = no light shading, 1 = ON */
	    UCHAR	PalTable[PAL_SIZE]; /* 16 solid colors, 32 zones */

	    short	WinStartX;	    /* Value of left side of viewport   */
	    short	WinStartY;	    /* Value of top side of viewport */
	    short	WinEndX;	    /* Value of right side              */
	    short	WinEndY;	    /* Value of bottom side */
	    short	CenterRow;	    /* Value of (WinEndY-WinStartY)/2 */
	    short	CenterOffset;	    /* Center row times bytes per row */
	    short	WinWidth;	    /* Value of WinEndX - WinStartX */
	    short	WinHeight;	    /* Value of WinEndY - WinStartY */
	    USHORT	WinLength;	    /* Number of dwords in window */
	    USHORT	WinStartOffset;	    /* Value of WinStartY * 320 */
	    USHORT	SysFlags;	    /* General system flags */
	    UCHAR	bmLoadType;	    /* Bitmap load flags (BBM,GIF,etc) */
	    short	MaxObjects;	    /* Total objects in map */
	    NEWOBJECT	*ObjList[MAX_OBJECTS+1]; /* Current objects in map */
	    DOORS	Door[MAX_DOORS];    /* Doors moving at one time */

	    //MEH
	    short   PlayerHeight;   //How far off the ground the player is
} ACKENG;

typedef struct {
    unsigned char start;
    unsigned char length;
} ColorRange;


short AckInitialize(ACKENG *ae);
/* Reads trig files, builds wall and object maps, general initialize */
/* Also sets up distance table */

short AckOpenResource(char *ResFileName);
/* Opens a resource file and prepares for reading */

void AckCloseResource(void);
/* Closes the currently open resource file */

short AckReadMapFile(ACKENG *ae,char *MapFileName);
/* Reads map file and setups map grids */

short AckLoadBitmap(ACKENG *ae,short BitmapNumber,short BitmapType,char *bmFileName);
/* Loads a bitmap and places in either bMaps or oMaps based on bmType */

UCHAR *AckReadBBM(char *FileName);
/* Reads in a .LBM or .BBM file and returns a buffer */

UCHAR *AckReadPCX(char *FileName);
/* Reads in a .pcx file and returns a buffer */

UCHAR *AckReadGIF(char *FileName);
/* Reads a 256 color GIF file and returns a buffer */

short AckLoadAndSetPalette(char *FileName);
/* Reads in a palette file and sets the screen palette */

void AckSetPalette(UCHAR *PalBuffer);
/* Sets a previously loaded palette */

short AckLoadWall(ACKENG *ae,short WallNumber,char *bmFileName);
/* Loads a wall bitmap and places it into the wall array */

short AckLoadObject(ACKENG *ae,short BmpNumber,char *bmFileName);
/* Loads an object bitmap and places it into the object array */

short AckCreateObject(ACKENG *ae,short ObjNumber);
/* Fills in ObjList structure with information passed */

short AckSetObjectType(ACKENG *ae,short oNum,short oType);
short AckSetupObject(ACKENG *ae,short oNum,short oType,OBJSEQ *os);

short AckCreateOverlay(ACKENG *ae, UCHAR *OverlayScreen);
/* Reads overlay file and compiles into OverlayBuffer */

short AckBuildBackground(ACKENG *ae);
/* Builds background buffer from TopColor, BottomColor, and LightFlag */

void AckSetVGAmode(void);
/* Places video in standard 320x200 mode 13h */

void AckSetTextmode(void);
/* Places video in 80x25 color text mode 3 */

void AckFadeIn(short Begin,short Count,UCHAR *Palette);
/* Fades the screen in from black to the supplied palette */

void AckFadeOut(short Begin,short Count);
/* Fades the screen from current palette to black */

void AckSetupPalRanges(ACKENG *ae,ColorRange *ranges);
/* Fills in the shading palette table */

void AckRegisterStructure(ACKENG *ae);
/* Assigns the current engine structure to various global variables */
/* This function MUST be called before AckBuildView!! */

void AckBuildView(void);
/* Draws the current view into ScreenBuffer */

void AckDrawOverlay(UCHAR *Screen,UCHAR *Overlay);
/* Places the overlay graphics on top of the screen */

short AckDisplayScreen(void);
/* Displays the contents of ScreenBuffer and OverlayBuffer if desired */

void AckSetPage(short PageNum);
/* Sets current displayed page to 0 or 1 */

short AckCheckHit(short xPlayer,short yPlayer,short ViewAngle);
/* Checks if a collision occurs */

short AckCheckDoorOpen(short xPlayer,short yPlayer,short PlayerAngle);
/* Check if a door in front to open */

short AckCheckObjPosn(short xPlayer,short yPlayer,short PlayerAngle,short oIndex);
/* Used by AckMovePOV() and AckMoveObjectPOV() to check collision with objects */

void AckCheckObjectMovement(void);
/* Runs the list of objects to check movement */

short AckMovePOV(short Angle,short Amount);
/* Moves the POV by the specified amount at the specified angle */

short AckMoveObjectPOV(short ObjIndex,short Angle,short Amount);
/* Moves the object POV by the specified amount at the specified angle */

short AckGetObjectHit(void);
/* Returns the object index number of the last object hit */

short AckGetWallHit(void);
/* Returns the map location of the last wall hit */

short AckGetObjectAngle(int32_t DeltaX,int32_t DeltaY);
/* Returns the angle (0-1919) between two objects */

short AckDeleteObject(ACKENG *ae,short ObjectIndex);
/* Removes the specified object from the map */

void AckReplaceBitmap(USHORT *xGrid,USHORT *yGrid,UCHAR OldBmp,UCHAR NewBmp);
/* Runs through the map and replaces old bitmaps with new bitmaps */

void AckOverlayBitmap(UCHAR *destination,UCHAR *source);
/* Copies non-transparent colors from source onto destination bitmap */

UCHAR *AckCopyNewBitmap(UCHAR *OldBitmap);
/* Creates a new bitmap buffer and copies OldBitmap into it */

short AckCreateBitmap(UCHAR *bmp,UCHAR **bmType);
/* Uses extended memory if available and returns pointer to new bitmap */
/* Frees bmp passed if XMS is allocated */

short AckSetNewBitmapPtr(short BitmapNumber,UCHAR **Maps,UCHAR *NewBitmap);
/* Sets a new or changes image into the bitmap array */

short AckFreeBitmap(UCHAR *bmType);
/* Frees up the memory used by the bitmap */

short AckWrapUp(ACKENG *ae);
/* Frees memory buffers */

void *AckMalloc(size_t mSize);
/* Internal memory allocation for development purposes */

void AckFree(void *m);
/* ALL memory allocated with AckMalloc must use AckFree to free memory */

//-----------------------End: ACK3D.H from Station Escape example program
#include <string>
using std::string;

//defines
#define ACK_VIEWWIDTH     96
#define ACK_VIEWHEIGHT    200       //Our screen width and height is set to 320x200
#define ACK_BYTES_PER_PIXEL 4       //32-bit offscreen buffer

#define ACK_LIGHTSHADING_OFF    0
#define ACK_LIGHTSHADING_ON     1   //Used to turn light shading off or on

extern ACKENG* ae; //Blah, this has to be global, or else nobody works. TODO: Fix later.

//Structure for holding RGB data
typedef struct {
    unsigned char r, g, b, a;
} PalletteSlot;

//Misc helper functions
short WrapAngle(short sAngle);                      //Makes sure an angle is in the 0-359 degree range
short GetMapPosn(short x, short y);                 //Returns the map position (grid square number) of x and y

//AckWrapper class definition
class AckWrapper
{
protected:
    unsigned char*  m_OffscreenBuffer;  //Used to hold the data for the screen.
    //MEH buffer for holding our palette data. Read in from "pallette.txt"
    PalletteSlot m_pallette[256]; //256 entries for 256 colors

    AckWrapper() {};    //No default constructor
    void ReadInPallette();      //Reads in the pallette from "pallette.txt"
    short InitACKEngine();      //Creates and initializes ACK-3D
    int AckProcessInfoFile();      //Processes the file that's been opened
    int LoadBackDrop();             //load the bg from the resource file
    void ProcessBackDrop(UCHAR *bPtr);  //and put it into the bg array
    //and functions used by ProcessInfoFile():
    int ReadLine();
    char* GetNextParm(char* s);
    int LoadWall();
    int LoadObject();
    char* SkipSpaces(char* s);
    int CreateObject();
    //------------------------------
    //Callback function for when an object hits something
    void	(*m_ObjectCollide)(NEWOBJECT* pObj, UCHAR nIndex, short nResult);

public:
    //Constructor/destructor
    AckWrapper(string sFilename);   //Creates an ACK-3D engine from the specified map file
    ~AckWrapper();  //Cleans up the ACK-3D engine and all memory

    //General methods
    const unsigned char*  RenderView();       //Builds the view, and returns a RGBA buffer to it.
    //DO NOT FREE OR DELETE the pointer once you're done using it!

    void            Update();           //Updates the engine- moves player, objects, etc

    //Accessor methods
    short GetPlayerAngle()              { return ae->PlayerAngle; };  //Get the player's current angle
    void  SetPlayerAngle(short sAngle)   { ae->PlayerAngle = sAngle; }; //Set the viewing angle of the player. No error checking.
    void  AddToPlayerAngle(short sVal);  //Add a positive or negative value to the player's angle. Has error checking.

    void  SetLightShading(int iOnOff);   //Turns light shading on or off. Pass ACK_LIGHTSHADING_ON/OFF to turn on or off
    int   GetLightShading();            //Returns ACK_LIGHTSHADING_ON or ACK_LIGHTSHADING_OFF

    void  OpenDoorForPlayer();          //If the player is near a door, open it
    short MovePlayer(short sAmount, short sAddAngle = 0); //Move the player by Amount, adding sAddAngle to the player's current angle
                                                          //if so desired
    //assigns the collision callback function
    void AssignObjectCallback(void (*ObjectCollide)(NEWOBJECT* pObj, UCHAR nIndex, short nResult)) {m_ObjectCollide = ObjectCollide;};

};








#endif  //Defined ACKWRAPPER_H
