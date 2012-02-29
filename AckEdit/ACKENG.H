/* Header file for ACK3D engine routines */

#define TRANS_WALLS	0
#define FLOOR_ACTIVE	1
#define USE_XMS		0	    /* Set to 0 if XMS not desired */

#define MAX_RBA		500	    /* Number of rba's in resource header */


#define FP_SHIFT	16
#define FP_MULT		65536
#define FP_HALF		32768

#define VIEW_WIDTH	320
#define MAX_DISTANCE	2048

#define TYPE_WALL	0
#define TYPE_OBJECT	1
#define TYPE_PALETTE	2

#define MAX_HEIGHT	960
#define MIN_HEIGHT	8


#define MAX_UPDOWN	    30	    /* Max up or down spots for each level */

#define MAP_STARTCODE	    0xFC    /* Force player to this square  */
#define MAP_UPCODE	    0xFD    /* Go up to previous level	    */
#define MAP_DOWNCODE	    0xFE    /* Go down to next level	    */
#define MAP_GOALCODE	    0xFF    /* Finish line!		    */

typedef struct _WARRAY {
	short	      Number;
	short	      Column;
	short	      Distance;
	short	      LightAdj;
struct	_WARRAY	    *Prev;
struct	_WARRAY	    *Next;
    } WALLARRAY;

typedef struct {
	    short     mPos;
	    USHORT    mCode;
} SPECIALCODE;

typedef struct _slicer {
		UCHAR	  **bMap;
		UCHAR	  *mPtr;
		short	  bNumber;
    unsigned	short	  bColumn;
		short	  Distance;
		short	  mPos;
    unsigned	char	Type;
		void	(*Fnc)(void);
    unsigned	char	Active;	    /* Keep these last 3 fields in this order */
    struct	_slicer *Prev;	    /* a memmove is done on sizeof(SLICE)-9 */
    struct	_slicer *Next;	    /* to move everything above these fields. */
    } SLICE;

typedef struct _saslicer {
		UCHAR	  **sabMap;
		UCHAR	  *samPtr;
		short	  sabNumber;
    unsigned	short	  sabColumn;
		short	  saDistance;
		short	  samPos;
    unsigned	char	saType;
		void	(*saFnc)(void);
    unsigned	char	saActive;     /* Keep these last 3 fields in this order */
    struct	_slicer *saPrev;      /* a memmove is done on sizeof(SLICE)-9 */
    struct	_slicer *saNext;      /* to move everything above these fields. */
    } SASLICE;

typedef struct {
    UCHAR   *Vid;
    UCHAR   *Wall;
    UCHAR   *Pal;
    short   dst;
    short   wht;
    short   multi;
    short   topht;
    short   botht;
    short   savwht;
    short   mulcnt;
    UCHAR   *savVid;
    USHORT    bNum;
    } SCOL;

typedef struct {
    short   WinWidth;
    short   WinStartY;
    short   WinEndY;
    short   CenterRow;
    short   CenterOffset;
    short   Color;
    short   WinStartX;
    } SOLIDS;

#define ST_WALL	    1
#define ST_OBJECT   2

#define COLS_PER_BYTE	1	/* Use 1 for normal mode 13h, 4 for modeX */
#define BYTES_PER_ROW	320	/* Use 320 for normal mode 13h, 80 for modeX */
#define DWORDS_PER_ROW	(BYTES_PER_ROW / 4)
#define SCREEN_SIZE	64000

