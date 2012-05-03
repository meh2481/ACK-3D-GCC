// ACKEDIT.CPP - Map Editor for ACK-3D Windows

#define  STRICT
#include <windows.h>
#include <stdlib.h>
#include <string.h>
//#include "wing.h"
//#include "wingdll.h"
#include "ackwin.h"

//extern "C" {
#include "keys.h"
#include "ack3d.h"

//MEH Include this for reading in pallette
#include <fstream>

//MEH My function declarations
void ReadConfig();
void WriteConfig();
void HandleGenericKeypresses(WPARAM wParam);
void UpdateParentText(HWND hWnd);
void UpdateChildText(HWND hWnd);

//MEH so can call inside MainWindow::WndProc()
void ScrollBitmaps(HWND hWnd,int nScrollCode,int nPos);

extern              short   rsHandle;
extern              ULONG   *rbaTable;
extern              short   Resolution;
extern              UCHAR   colordat[];
extern              UCHAR   *BackArray[];
extern  unsigned    short   FloorMap[];
extern  unsigned    short   CeilMap[];
extern  unsigned    short   *ObjGrid;
extern              long    *SinTable;
extern              long    *CosTable;
extern              long    AckMemUsed;
//};

extern              char    LineBuffer[];

#include "ackedit.h"
#include "ackres.h"

// Some globals to use
    ACKENG      *ae = 0;        // Interface structure
    short       nMoveSpeed;
    short       nTurnSpeed;
    short       nOperation;     // Current operation being performed
    HINSTANCE   hInstGlobal;
    HWND        hWndMap;
    BOOL        bNoShow;        // Do not display if TRUE
    BOOL        bTrackPlayer;   // On if map tracks player position
    short       nViewType;      // Current view in grid window
    short       nEditType;      // Editing walls or objects
    WORD        nWallFlags;
    WORD        nObjFlags;
    UCHAR       **pBitmaps;     // Current bitmap array
    short       GridSize;
    short       GridWidth;
    short       GridHeight;
    short       nBackColor;
    short       xCurrent,yCurrent;
    short       CurrentTopBitmap;
    short       CurrentBitmap;
    short       BitmapSelectX1;
    short       BitmapSelectY1;
    short       BitmapSelectX2;
    short       BitmapSelectY2;
    HWND        hwndApp;
    COLORREF    rgbSelectColor;
    UCHAR       kbPressed[128];
    char        ResFileName[128];

//    WinGdll     WinG;

    header      BufferHeader;
    long        Orientation = 0;
    void        *pBuffer = 0;
    HDC         Buffer = 0;

    void        *pAckBuffer = 0;
    HDC         AckBuffer = 0;

    void        *pMapBuffer = 0;
    HDC         MapBuffer = 0;

    HPALETTE    hpalApp;

    BOOL        bMapBitmapInitialized = FALSE;
    BOOL        bShowRendering;
    BOOL        bWTinitialized = FALSE;
    RGBQUAD     ColorTable[256];

    pal         LogicalPalette = {0x300,256};

    HBITMAP     gbmOldMonoBitmap = 0;

    int         StretchFactor = 1;
    int         nBitmapW;
    int         nBitmapH;       //WinG bitmap dimensions.
    BOOL        bModified;
    BOOL        bShowCoords;
    char        MainWindow::szClassName[] = "Map Editor";
    char        szCurPath[130];
    char        *bMapFiles[MAX_WALLBMPS];
    char        *oMapFiles[MAX_OBJBMPS];
    short       bMapIndex[MAX_WALLBMPS];
    short       oMapIndex[MAX_OBJBMPS];
    short       bMapType[MAX_WALLBMPS];
    short       oMapType[MAX_OBJBMPS];

    char        BackGndFile[128];

    //MEH So we can globally access our main window...
    MainWindow* MainAppWnd;


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

    extern  int LineNumber; // In ACKSUP.CPP


    //MEH buffer for holding our pallette data. Read in from "pallette.txt"
  PalletteSlot pallette[256];
  //MEH function for reading in our pallette
void ReadInPallette(void);

//MEH 24-bit screen buffer, for Win32 reasons
   unsigned char OffscreenBuffer[4*320*200]; //4 chars per pixel (rgba), 320 pixels by 200 pixels

HDC hOffscreenDC = 0;
HBITMAP hOffscreenBitmap;

//MEH Buffer for holding a bitmaps' data before we draw it
unsigned char GenericUseBitmapBuffer[4*64*64];   //4 chars per pixel (rgba), 64x64 pixels
//MEH Offscreen buffer and HDC for child (bitmaps and objects) window
HDC hChildOffscreenDC = 0;
HBITMAP hChildOffscreenBitmap;

//MEH so we can pan the view with MMB
bool g_bPanning = false;
POINT g_ptStartPanLocation;

//MEH So we can copy/paste objects
unsigned short g_sClipboard = 0;

//MEH For setting the mouse cursor
HCURSOR g_cArrow, g_cPan;

//MEH For making it easier to click on edge of grid square
#define SQUARE_EDGE_SIZE    8

//MEH For ctrl and shift clicking not modifying the map
bool g_bNotModifyOnMouseUp = false;

//MEH For dealing with undo/redo
short g_sNumUndo = 0;
short g_sNumRedo = 0;
short g_sSaveUndo = 0;  //The g_sNumUndo location that we last saved at
bool g_bNoCreateUndoRedo = false;   //Whether we should create undo/redo locations or not

bool g_bCreatedObjectThisCycle = false; //Whether or not we created an object, in case user selects "cancel"
                                        // in the object attributes dialog right after they create it, we want
                                        // to delete the newly-created object.


//****************************************************************************
// Prototypes
//****************************************************************************
void AckReset(void);
void gkScaleBitmap(short bmWt,short bmHt,short DstWt,short DstHt,UCHAR *bm,
                   UCHAR *DstBuffer);
int DoOptionsDlg(HINSTANCE hInst,HWND hWnd);
//int DoGlobalDlg(HINSTANCE hInst,HWND hWnd);
int DoInfoDlg(HINSTANCE hInst,HWND hWnd);
short SaveResource(HWND hWnd,char *szName);
void EditInfoFile(HWND hWnd);

//****************************************************************************
//
//****************************************************************************
void MainWindow::Register( void )
{
    WNDCLASS wndclass;   // Structure used to register Windows class.

wndclass.style         = CS_HREDRAW | CS_VREDRAW;
wndclass.lpfnWndProc   = ::WndProc;
wndclass.cbClsExtra    = 0;
wndclass.cbWndExtra    = sizeof( MainWindow * );
wndclass.hInstance     = Main::hInstance;
wndclass.hIcon         = LoadIcon( Main::hInstance, "ackicon" );
wndclass.hCursor       = NULL; //LoadCursor( NULL, IDC_ARROW );
wndclass.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
wndclass.lpszMenuName  = "ACKMENU";
wndclass.lpszClassName = szClassName;

if ( ! RegisterClass( &wndclass ) )
    exit( FALSE );

wndclass.style         = CS_HREDRAW | CS_VREDRAW;
wndclass.lpfnWndProc   = ::ChildWndProc;
wndclass.cbClsExtra    = 0;
wndclass.cbWndExtra    = 0;
wndclass.hInstance     = Main::hInstance;
wndclass.hIcon         = LoadIcon( Main::hInstance, "ackicon" );
wndclass.hCursor       = NULL; //LoadCursor( NULL, IDC_ARROW );
wndclass.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
wndclass.lpszMenuName  = NULL;
wndclass.lpszClassName = "ACKBitmaps";

if ( ! RegisterClass( &wndclass ) )
    exit( FALSE );

wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
wndclass.lpfnWndProc   = ::AckWndProc;
wndclass.cbClsExtra    = 0;
wndclass.cbWndExtra    = 0;
wndclass.hInstance     = Main::hInstance;
wndclass.hIcon         = LoadIcon( Main::hInstance, "ackicon" );
wndclass.hCursor       = NULL; //LoadCursor( NULL, IDC_ARROW );
wndclass.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
wndclass.lpszMenuName  = NULL;
wndclass.lpszClassName = "ACK3DVIEW";

if ( ! RegisterClass( &wndclass ) )
    exit( FALSE );

}

//****************************************************************************
//
//****************************************************************************
MainWindow::MainWindow( void )
{
/*
if (!WinG.Load())
    {
    MessageBox(0,"Can't find WING32.DLL","ACK Error!",MB_OK);
    exit(FALSE);
    }*/

}

//****************************************************************************
//
//****************************************************************************
void MainWindow::CreateAll( void )
{
    RECT    rect;
    short   xPos;
    int     windx,windy;
    HBITMAP hbm;


GridSize = 32;
xCurrent = yCurrent = 0;
CurrentTopBitmap = 1;
CurrentBitmap = 1;
rgbSelectColor = RGB(255,0,0);

hWnd = CreateWindow( szClassName,
            "Map Editor - Walls",
            WS_OVERLAPPEDWINDOW|WS_VSCROLL|WS_HSCROLL,
            0,
            0,
            480,
            520,
            NULL,
            NULL,
            Main::hInstance,
            (LPSTR)this);

if (!hWnd)
    exit(FALSE);

hWndMap = hWnd;
Show( Main::nCmdShow );
Update();

GetWindowRect(hWnd,&rect);

char cBitmapTxt[32];
if(nEditType == EDIT_OBJECTS)
    sprintf(cBitmapTxt, "Obj: 1");
else
    sprintf(cBitmapTxt, "Wall: 1");

hWndChild = CreateWindow( "ACKBitmaps",
                cBitmapTxt,
                WS_POPUP|WS_BORDER|WS_CAPTION|WS_VISIBLE|WS_VSCROLL,
                rect.right,
                0,
                90,
                298,
                hWnd,
                NULL,
                Main::hInstance,
                (LPSTR)NULL);

if (!hWndChild)
    {
    MessageBox(NULL,"Unable to create bitmap window","ERROR",MB_OK);
    exit(FALSE);
    }

ShowWindow(hWndChild,SW_NORMAL);
xPos = (short)rect.right;

GetWindowRect(hWndChild,&rect);




windx = windy = GetSystemMetrics(SM_CXBORDER) * 2;
windx += nBitmapW;
windy += nBitmapH + GetSystemMetrics(SM_CYCAPTION) - 1;

hwndApp = CreateWindow ("ACK3DVIEW",    // Class name
                "3D View",              // Caption
                WS_POPUP|WS_BORDER|WS_CAPTION|WS_VISIBLE,
                xPos,                   // Position
                rect.bottom,
                windx,windy,            // Size
                (HWND)hWnd,             // Parent window
                (HMENU)NULL,            // use class menu
                Main::hInstance,        // handle to window instance
               (LPSTR)NULL );           // no params to pass on

//Set the window pos to 640*400 if stretched
if(StretchFactor == 2)
    SetWindowPos(hwndApp,NULL,0,0,640,400, SWP_NOMOVE|SWP_NOZORDER|SWP_NOCOPYBITS);

if (!hwndApp)
    {
    MessageBox(NULL,"Unable to create 3D View window","ERROR",MB_OK);
    exit(FALSE);
    }

ShowWindow(hwndApp,SW_NORMAL);

//if (WinG.pRecommendDIBFormat((BITMAPINFO *)&BufferHeader))
//    {
//    BufferHeader.Header.biBitCount = 8;
//    BufferHeader.Header.biCompression = BI_RGB;
//    }
//else
 //   {
   /* BufferHeader.Header.biSize = sizeof(BITMAPINFOHEADER);
    BufferHeader.Header.biPlanes = 1;
    BufferHeader.Header.biBitCount = 8;
    BufferHeader.Header.biCompression = BI_RGB;
    BufferHeader.Header.biSizeImage = 0;
    BufferHeader.Header.biClrUsed = 0;
    BufferHeader.Header.biClrImportant = 0;
  //  }

BufferHeader.Header.biWidth = 64;
BufferHeader.Header.biHeight = -350; // Use -height for a Top-down bitmap
*/
CreateAckPalette();
/*
for (Counter = 0;Counter < 256;Counter++)
    {
    BufferHeader.aColors[Counter].rgbRed   = ColorTable[Counter].rgbRed;
    BufferHeader.aColors[Counter].rgbGreen = ColorTable[Counter].rgbGreen;
    BufferHeader.aColors[Counter].rgbBlue  = ColorTable[Counter].rgbBlue;
    BufferHeader.aColors[Counter].rgbReserved = 0;
    }*/

Buffer = CreateCompatibleDC(GetDC(hwndApp));
hbm = CreateCompatibleBitmap(GetDC(hwndApp),nBitmapW,nBitmapH);

gbmOldMonoBitmap = (HBITMAP)SelectObject(Buffer, hbm);

PatBlt(Buffer, 0,0,nBitmapW,nBitmapH, BLACKNESS);
GetClientRect(hWnd,&rect);
GridWidth = (short)rect.right;
GridHeight = (short)rect.bottom;
//BufferHeader.Header.biWidth = (short)rect.right;
//BufferHeader.Header.biHeight = -(rect.bottom+GridSize); // Use -height for a Top-down bitmap
MapBuffer = CreateCompatibleDC(GetDC(hwndApp));
hbm = CreateCompatibleBitmap(GetDC(hwndApp),rect.right,rect.bottom);
SelectObject(MapBuffer,hbm);
PatBlt(MapBuffer, 0,0,rect.right,rect.bottom, BLACKNESS);
bMapBitmapInitialized = TRUE;
InvalidateRect(hWnd,NULL,TRUE);
InvalidateRect(hWndChild,NULL,TRUE);

//MEH Make sure windows have right text to start with
UpdateChildText(hWndChild);
//UpdateParentText(hWnd);
}

HINSTANCE Main::hInstance = 0;
HINSTANCE Main::hPrevInstance = 0;
int Main::nCmdShow = 0;


//****************************************************************************
//
//****************************************************************************
int Main::MessageLoop(void)
{
    MSG msg;
    unsigned int iCurTick = GetTickCount();
    unsigned int iTemp = 0;

    for (;;)
    {
        iTemp = GetTickCount();
        if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        //MEH Limit our framerate to something reasonable so you can actually MOVE AROUND in the 3D view
        //without getting totally sick.
        else if(iTemp - iCurTick >= 30)
        {
            if (nOperation == OP_NORMAL)
            {
                DoCycle();
                UpdateParentText(MainAppWnd->GetHandle());
            }
            iCurTick = iTemp;
        }
    }

    return msg.wParam;
}


//****************************************************************************
//
//****************************************************************************
void MainWindow::DrawGrid(HDC hdc,short wt,short ht)
{
    POINT   pt;
    HPEN    pen,oldpen;
    short   x,y;

/*//MEH first fill in bg color. Pretty standard graphics primitives stuff
PalletteSlot bgpalcol = pallette[nBackColor];
HBRUSH hBgColorBrush = CreateSolidBrush(RGB(bgpalcol.b, bgpalcol.g, bgpalcol.r));   //remember that R and B are reversed...
HBRUSH hOld = (HBRUSH)SelectObject(hdc, hBgColorBrush);
RECT rcArea = {0,0,wt*GridSize, ht*GridSize};
FillRect(hdc, &rcArea, hBgColorBrush);
SelectObject(hdc, hOld);
DeleteObject(hBgColorBrush);
//---MEH */

pen = CreatePen(PS_SOLID,1,RGB(128,128,128));
oldpen = (HPEN)SelectObject(hdc,pen);

for (y = 0; y < ht; y += GridSize)
    {
    MoveToEx(hdc,0,y,&pt);
    LineTo(hdc,wt,y);
    }

for (x = 0; x < wt; x += GridSize)
    {
    MoveToEx(hdc,x,0,&pt);
    LineTo(hdc,x,ht);
    }

SelectObject(hdc,oldpen);
DeleteObject(pen);
}



//****************************************************************************
//
//****************************************************************************
void DrawMap(HWND hWnd,HDC hdc,short wt,short ht)
{
    //wt++;
    //ht+=GridSize;

    short   MapPosn,x,y,mx,my,OrgPos,bx,by,x2,y2,x3,y3;
    int     len,offset;
    POINT   pts[4];
    WORD    nMapCode;
    HBRUSH  NewBrush,OldBrush;
    UCHAR   *pBmp,*pMap;
    NEWOBJECT   *pObj;



if (!bMapBitmapInitialized)
    return;

    UCHAR   *TempScreenBuffer = (UCHAR*)malloc(wt*ht);   //MEH Create a temp screen buffer
    UCHAR   *TempIconBuffer = (UCHAR*)malloc(wt*ht*4);    //MEH and another buffer to hold the icon
    memset(TempScreenBuffer,nBackColor,wt*ht);  //Fill in with bg color initially

NewBrush = CreateSolidBrush(RGB(0,0,192));
OldBrush = (HBRUSH)SelectObject(hdc,NewBrush);

if (bTrackPlayer)
    {
    bx = xCurrent;
    by = yCurrent;
    x = (short)(wt / GridSize);
    y = (short)(ht / GridSize);
    x2 = x >> 1;
    y2 = y >> 1;
    mx = ae->xPlayer >> 6;
    my = ae->yPlayer >> 6;

    x = 63 - x;
    y = 63 - y;

    xCurrent = mx - x2;
    if (xCurrent < 0)
        xCurrent = 0;

    if (xCurrent > x)
        xCurrent = x;

    yCurrent = my - y2;
    if (yCurrent < 0)
        yCurrent = 0;

    if (yCurrent > y)
        yCurrent = y;

    if (xCurrent != bx)
        SetScrollPos(hWnd,SB_HORZ,xCurrent,TRUE);

    if (yCurrent != by)
        SetScrollPos(hWnd,SB_VERT,yCurrent,TRUE);

    }

OrgPos = MapPosn = (short)((yCurrent * GRID_WIDTH) + xCurrent);
x2 = (short)((wt / GridSize) * GridSize);
y2 = (short)((ht / GridSize) * GridSize);

pMap = (UCHAR *)TempScreenBuffer;

len = (int)(wt * ht);
//memset(pMap,nBackColor,len);

switch (nViewType)
    {
    case VIEW_WALLS:
        for (y = 0; y < y2; y += GridSize)
            {
            mx = MapPosn;
            for (x = 0; x < x2; x += GridSize)
                {
                nMapCode = ObjGrid[mx];
                if (nMapCode)
                {
                    pObj = ae->ObjList[nMapCode & 0xFF];
                    if (pObj != NULL)
                    {
                        pBmp = ae->oMaps[pObj->CurrentBitmaps[0]];
                        if(pBmp != NULL)
                        {
                            offset = (y * wt) + x;
                            for (bx = 0; bx < GridSize; bx++)
                            {
                                pMap = (UCHAR *)TempScreenBuffer + offset;
                                for (by = 0; by < GridSize; by++)
                                {
                                    if(*pBmp != 0)
                                        *pMap = *pBmp;  //MEH: Don't draw black bg. It's distracting
                                    pMap += wt;
                                    pBmp += 2;
                                }
                                offset++;
                                pBmp += 64;
                            }
                        }
                    }
                }

                nMapCode = ae->xGrid[mx];
                if (nMapCode)
                    {
                    pMap = (UCHAR *)TempScreenBuffer + (y * wt) + x;
                    pBmp = ae->bMaps[nMapCode & 0x00FF];
                    for (my = 0; my < GridSize; my++)
                        {
                        *pMap = *pBmp;
                        pMap[1] = pBmp[(7 * 64)];
                        pMap[2] = pBmp[(15 * 64)];
                        pMap[3] = pBmp[(23 * 64)];
                        pMap[4] = pBmp[(31 * 64)];
                        pMap[5] = pBmp[(39 * 64)];
                        pMap[6] = pBmp[(47 * 64)];
                        pMap[7] = pBmp[(55 * 64)];
                        pMap += wt;
                        pBmp++;
                        }
                    }

                nMapCode = ae->yGrid[mx];
                if (nMapCode)
                    {
                    pMap = (UCHAR *)TempScreenBuffer + (y * wt) + x;
                    pBmp = ae->bMaps[nMapCode & 0x00FF];
                    for (my = 0; my < 8; my++)
                        {
                        memcpy(pMap,pBmp,GridSize);
                        pBmp += (7 * 64);
                        pMap += wt;
                        }
                    }

                mx++;
                }

            MapPosn += (short)64;
            offset = y * wt;
            pMap = (UCHAR *)TempScreenBuffer + offset;
            memset(pMap,0,wt);
            }
        break;

    case VIEW_FLOOR:
        for (y = 0; y < y2; y += GridSize)
            {
            mx = MapPosn;
            for (x = 0; x < x2; x += GridSize)
                {
                nMapCode = FloorMap[mx];
                if (nMapCode)
                    {
                    pBmp = ae->bMaps[nMapCode & 0xFF];
                    offset = (y * wt) + x;
                    for (bx = 0; bx < GridSize; bx++)
                        {
                        pMap = (UCHAR *)TempScreenBuffer + offset;
                        for (by = 0; by < GridSize; by++)
                            {
                            *pMap = *pBmp;
                            pMap += wt;
                            pBmp += 2;
                            }
                        offset++;
                        pBmp += 64;
                        }

                    }

                mx++;
                }
            MapPosn += (short)64;
            offset = y * wt;
            pMap = (UCHAR *)TempScreenBuffer + offset;
            memset(pMap,0,wt);
            }
        break;

    case VIEW_CEILING:
        for (y = 0; y < y2; y += GridSize)
            {
            mx = MapPosn;
            for (x = 0; x < x2; x += GridSize)
                {
                nMapCode = CeilMap[mx];
                if (nMapCode)
                    {
                    pBmp = ae->bMaps[nMapCode & 0xFF];
                    offset = (y * wt) + x;
                    for (bx = 0; bx < GridSize; bx++)
                        {
                        pMap = (UCHAR *)TempScreenBuffer + offset;
                        for (by = 0; by < GridSize; by++)
                            {
                            *pMap = *pBmp;
                            pMap += wt;
                            pBmp += 2;
                            }
                        offset++;
                        pBmp += 64;
                        }
                    }
                mx++;
                }
            MapPosn += (short)64;
            offset = y * wt;
            pMap = (UCHAR *)TempScreenBuffer + offset;
            memset(pMap,0,wt);
            }
        break;

    default:
        break;
    }

for (x = 0; x < x2; x += GridSize)
    {
    pMap = (UCHAR *)TempScreenBuffer + x;
    for (y = 0; y < ht; y++)
        {
        *pMap = 0;
        pMap += wt;
        }
    }

    //MEH Draw stuffz!
    unsigned char* pBufferData = TempIconBuffer;
    for(int r = 0; r < ht; r++)
    {
        for(int c = 0; c < wt; c++) //AHAHA JOKE
        {
            *pBufferData++ = pallette[TempScreenBuffer[c+wt*r]].r;
            *pBufferData++ = pallette[TempScreenBuffer[c+wt*r]].g;
            *pBufferData++ = pallette[TempScreenBuffer[c+wt*r]].b;
            *pBufferData++ = 255; //Alpha = 255 by default.
        }
    }
    //Create our offscreen HDC if we haven't yet
    /*if(!hOffscreenDC)
    {
        hOffscreenDC = CreateCompatibleDC(hdc);
        hOffscreenBitmap = CreateCompatibleBitmap(hdc, 320, 200);
        SelectObject(hOffscreenDC, hOffscreenBitmap);
    }*/
    //PatBlt(hOffscreenDC, 0,0,nBitmapW,nBitmapH, BLACKNESS);
    HICON ic = CreateIcon(Main::hInstance, wt, ht, 1, 32, NULL, (BYTE*)(TempIconBuffer));
    DrawIconEx(hdc, 0, 0, ic, wt, ht, NULL, NULL, DI_NORMAL);
    DestroyIcon(ic);
    //-------MEH

//BitBlt(hdc,0,0,wt,ht,MapBuffer,0,0, SRCCOPY);

x = (short)((ae->yPlayer & 0xFFC0) + (ae->xPlayer >> 6));

if (x >= OrgPos && x <= (MapPosn-1))
    {
    mx = (short)((ae->xPlayer >> 6) - xCurrent);
    my = (short)((ae->yPlayer >> 6) - yCurrent);
    x = ((mx * GridSize) + (GridSize >> 1));
    y = ((my * GridSize) + (GridSize >> 1));

    bx = (CosTable[ae->PlayerAngle] * 16) >> 16;
    by = (SinTable[ae->PlayerAngle] * 16) >> 16;
    bx += x;
    by += y;
    mx = ae->PlayerAngle - INT_ANGLE_90;
    if (mx < 0)
        mx += INT_ANGLE_360;
    x2 = (CosTable[mx] * 5) >> 16;
    y2 = (SinTable[mx] * 5) >> 16;
    x2 += x;
    y2 += y;
    mx = ae->PlayerAngle + INT_ANGLE_90;
    if (mx >= INT_ANGLE_360)
        mx -= INT_ANGLE_360;
    x3 = (CosTable[mx] * 5) >> 16;
    y3 = (SinTable[mx] * 5) >> 16;
    x3 += x;
    y3 += y;

    pts[0].x = bx;
    pts[0].y = by;
    pts[1].x = x2;
    pts[1].y = y2;
    pts[2].x = x3;
    pts[2].y = y3;
    pts[3].x = bx;
    pts[3].y = by;
    Polygon(hdc,(LPPOINT)&pts,4);
    }

SelectObject(hdc,OldBrush);
DeleteObject(NewBrush);

if (bShowCoords)
{
//    UpdateParentText(hWnd);
}

free(TempScreenBuffer); //MEH clean up memory
free(TempIconBuffer);

}


//****************************************************************************
//
//****************************************************************************
void MainWindow::Paint(void)
{
    PAINTSTRUCT ps;
    RECT        rect;

BeginPaint( hWnd, &ps );
GetClientRect(hWnd,&rect);
if (!bNoShow)
{
    DrawMap(hWnd,ps.hdc,(short)rect.right,(short)rect.bottom);
    DrawGrid(ps.hdc,(short)rect.right,(short)rect.bottom);
}
EndPaint( hWnd, &ps );
}

//****************************************************************************
//
//****************************************************************************
void SetOperationCursor(void)
{
if (nOperation == OP_NORMAL)
    SetCursor(LoadCursor(NULL,IDC_ARROW));

if (nOperation == OP_WAIT)
    SetCursor(LoadCursor(NULL,IDC_WAIT));

}


//****************************************************************************
//
//****************************************************************************
void HighlightBitmap(HDC hdc,COLORREF crColor)
{
    POINT   pt;
    HPEN    pen,oldpen;

if (BitmapSelectX1 < 0)
    return;

pen = CreatePen(PS_SOLID,2,crColor);
oldpen = (HPEN)SelectObject(hdc,pen);

MoveToEx(hdc,BitmapSelectX1,BitmapSelectY1,&pt);
LineTo(hdc,BitmapSelectX2,BitmapSelectY1);
LineTo(hdc,BitmapSelectX2,BitmapSelectY2);
LineTo(hdc,BitmapSelectX1,BitmapSelectY2);
LineTo(hdc,BitmapSelectX1,BitmapSelectY1-1);

SelectObject(hdc,oldpen);
DeleteObject(pen);
}


//****************************************************************************
// Draw the bitmaps and such in the bitmap/object select window
//****************************************************************************
void PaintChild(HWND hWnd)
{
    PAINTSTRUCT ps;
    RECT        rect;
    short       y,y1,yOffset,nIndex,OrgIndex,bx;
    UCHAR       *pPtr = NULL;//,*pBuf;
    NEWOBJECT   *pObj;
    UCHAR       cIsObjFlag = 0; //If this is an object, go ahead and draw transparent instead of black
    UCHAR       cIsObjActive = 1;   //Is the object active in the map? Draw black border if it isn't.

BeginPaint( hWnd, &ps );
if (bNoShow)
    {
    EndPaint( hWnd, &ps );
    return;
    }

//Make sure our offscreen buffer is initialized
/*if(!hChildOffscreenDC)
{
    hChildOffscreenDC = CreateCompatibleDC(ps.hdc);
    hChildOffscreenBitmap = CreateCompatibleBitmap(ps.hdc, 90, 298);
}*/


if (hpalApp)
    {
    UnrealizeObject(hpalApp);
    SelectPalette(ps.hdc, hpalApp, FALSE);
    RealizePalette(ps.hdc);
    }

GetClientRect(hWnd,&rect);

PatBlt(ps.hdc, 0,0,90, 298, WHITENESS);  //Clear the bitmap to white

OrgIndex = nIndex = CurrentTopBitmap;
BitmapSelectX1 = 0;
y1 = (short)(rect.bottom - 66);
//memset(pBuffer,255,(short)(rect.bottom * 66));
y = 2;
while (y < y1)
    {
    if (nEditType == EDIT_WALLS)
        pPtr = ae->bMaps[nIndex++];
    else
        {
        pPtr = NULL;
        pObj = ae->ObjList[nIndex++];
        if (pObj != NULL)
            {
            if (pObj->CurrentBitmaps != NULL)
               pPtr = ae->oMaps[pObj->CurrentBitmaps[0]];
            cIsObjFlag = 1;
            cIsObjActive = pObj->Active;
            }
        }

    if (pPtr == NULL)
        {
        Rectangle(ps.hdc,2,y,66,y+64);
        }
    else
        {
        yOffset = (short)(y * 64);

        //Draw the currently selected bitmap
        //First block-copy the data into a form Windows likes
        unsigned char* pBufferData = GenericUseBitmapBuffer;
        for(bx = 0; bx < 64; bx++)
        {
            pBufferData = &GenericUseBitmapBuffer[bx*4];
            for(int by = 0; by < 64; by++)
            {
                unsigned char red = pallette[*pPtr].r;
                unsigned char green = pallette[*pPtr].g;
                unsigned char blue = pallette[*pPtr].b;
                *pBufferData++ = red;
                *pBufferData++ = green;
                *pBufferData++ = blue;
                if(red == 0 && green == 0 && blue == 0 && cIsObjFlag && cIsObjActive)
                    *pBufferData++ = 0; //Set alpha to zero for black object pixels
                else
                    *pBufferData++ = 255; //Set alpha to 255 by default
                //Go to next col/row/whatever
                pBufferData += 63*4;    //I really don't understand how in the world, but 63 is the right number here
                pPtr++;
            }
        }

        HICON ic = CreateIcon(Main::hInstance, 64, 64, 1, 32, NULL, (BYTE*)(GenericUseBitmapBuffer));
        if(ic == NULL)
            printf("Error: Could not create icon to draw.\n");
        DrawIconEx(ps.hdc, 2, y, ic, 64, 64, NULL, NULL, DI_NORMAL);
        DestroyIcon(ic);

        /*for (bx = 0; bx < 64; bx++)
            {
            pBuf = (UCHAR *)pBuffer + bx + yOffset;
            for (by = 0; by < 64; by++)
                {
                *pBuf = *pPtr++;
                pBuf += 64;
                }
            }*/
        }

    if ((nIndex-1) == CurrentBitmap)
        {
        BitmapSelectX1 = 1;
        BitmapSelectY1 = (short)(y-1);
        BitmapSelectX2 = 66;
        BitmapSelectY2 = (short)(y+65);
        }

    if (nEditType == EDIT_WALLS)
      {
      if (nIndex >= MAX_WALLBMPS)
           nIndex = 1;

      }
   else
      {
      if (nIndex >= MAX_OBJECTS)
         nIndex = 1;
      }


    if (nIndex == OrgIndex)
        break;

    y += (short)67;
    }

//BitBlt(ps.hdc,0,0,90,298,hChildOffscreenDC,0,0, SRCCOPY);
if (BitmapSelectX1)
    HighlightBitmap(ps.hdc,rgbSelectColor);

EndPaint( hWnd, &ps );
}


//****************************************************************************
//
//****************************************************************************
void AckPaint(HWND hWnd,HDC hdc)
{

if (ae != NULL)
    {
    AckCheckObjectMovement();
    AckBuildView();
    }

//MEH
//Fill in our screenbuffer from ACK's buffer
unsigned char* pBufferData = OffscreenBuffer;
for(int r = 0; r < 320; r++)
{
    for(int c = 0; c < 200; c++) //AHAHA JOKE
    {
        *pBufferData++ = pallette[ae->ScreenBuffer[c+200*r]].r;
        *pBufferData++ = pallette[ae->ScreenBuffer[c+200*r]].g;
        *pBufferData++ = pallette[ae->ScreenBuffer[c+200*r]].b;
        *pBufferData++ = 255; //Alpha = 255 by default. You could probably do crazy stuff with this, but I'm not going to.
    }
}
//Create our offscreen HDC if we haven't yet
if(!hOffscreenDC)
{
    hOffscreenDC = CreateCompatibleDC(hdc);
    hOffscreenBitmap = CreateCompatibleBitmap(hdc, 320, 200);
    SelectObject(hOffscreenDC, hOffscreenBitmap);
}
PatBlt(hOffscreenDC, 0,0,nBitmapW,nBitmapH, BLACKNESS);
HICON ic = CreateIcon(hInstGlobal, 320, 200, 1, 32, NULL, (BYTE*)(OffscreenBuffer));
DrawIconEx(hOffscreenDC, 0, 0, ic, 320, 200, NULL, NULL, DI_NORMAL);
DestroyIcon(ic);


if (StretchFactor != 1)
    StretchBlt(hdc,0,0,StretchFactor*nBitmapW,StretchFactor*nBitmapH,
                     hOffscreenDC,0,0,nBitmapW,nBitmapH, SRCCOPY);
else
    BitBlt(hdc,0,0,nBitmapW,nBitmapH,hOffscreenDC,0,0, SRCCOPY);

}

//****************************************************************************
//
//****************************************************************************
void ScrollMap(HWND hWnd,int nScrollCode,int nPos,int nDir)
{
    int     pos,nMaxWt,nWt,nMaxHt,nHt;
    RECT    rect;


if (bTrackPlayer)
    return;

GetClientRect(hWnd,&rect);
nWt = rect.right / GridSize;
nMaxWt = 64 - nWt;
nHt = rect.bottom / GridSize;
nMaxHt = 64 - nHt;

if (nDir)
    pos = GetScrollPos(hWnd,SB_VERT);
else
    pos = GetScrollPos(hWnd,SB_HORZ);

switch (nScrollCode)
    {
    case SB_LINEDOWN:
        if (nDir)
            {
            if (pos < nMaxHt-1)
                pos++;
            }
        else
            {
            if (pos < nMaxWt-1)
                pos++;
            }
        break;

    case SB_LINEUP:
        if (pos)
            pos--;
        break;

    case SB_PAGEDOWN:
        if (nDir)
            {
            pos += nHt;
            if (pos >= nMaxHt)
                pos = nMaxHt - 1;
            }
        else
            {
            pos += nWt;
            if (pos >= nMaxWt)
                pos = nMaxWt - 1;
            }
        break;

    case SB_PAGEUP:
        if (nDir)
            pos -= nHt;
        else
            pos -= nWt;
        if (pos < 0)
            pos = 0;
        break;

    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        pos = nPos;
        if (pos < 0)
            pos = 0;
        if (nDir == 1 && pos >= nMaxHt)
            pos = nMaxHt - 1;
        if (nDir == 0 && pos >= nMaxWt)
            pos = nMaxWt - 1;
        break;


    default:
        break;
    }

if (nDir)
    {
    yCurrent = (short)pos;
    SetScrollPos(hWnd,SB_VERT,pos,TRUE);
    }
else
    {
    xCurrent = (short)pos;
    SetScrollPos(hWnd,SB_HORZ,pos,TRUE);
    }

InvalidateRect(hWnd,NULL,TRUE);

}

//****************************************************************************
//
//****************************************************************************
int GetGridPos(short mx,short my)
{
    int     row,col,pos;

row = my / GridSize;
col = mx / GridSize;
row += yCurrent;
col += xCurrent;
pos = (row * GRID_WIDTH) + col;
return(pos);
}

//****************************************************************************
//
//****************************************************************************
short ObjectInUse(short nCode)
{
    short   i;

for (i = 0; i < 4096; i++)
    {
    if (nCode == (ObjGrid[i] & 0xFF))
        return(1);
    }

return(0);
}


//****************************************************************************
//
//****************************************************************************
void SetBitmap(HWND hWnd,short xPos,short yPos,short nCode)
{
    short   gsLeft,gsRight;
    int     mPos;
    NEWOBJECT  *pObj;
    BOOL    bSetAll = FALSE;
    BOOL    bRedraw = FALSE;    //MEH If we haven't changed anything, don't redraw
    unsigned short nMask;

mPos = GetGridPos(xPos,yPos);

//MEH Quick test first, to be sure that this is an actual bitmap we can place!
if(nEditType != EDIT_OBJECTS && nCode)
{
    UCHAR* test = ae->bMaps[nCode];
    if(test == NULL)
        return;
}

switch (nViewType)
    {
    case VIEW_WALLS:
        if (nEditType == EDIT_OBJECTS)
            {
            if (nCode != 0 && ObjectInUse(nCode) != 0)
                {
                MessageBox(hWnd,"Object has already been used in map.","Warning",MB_OK);
                return;
                }

            //MEH Set object at this position to inactive, BEFORE WE REMOVE IT! Grr...
            //(if (!nCode), then we're removing an object, and if(nCode), then we're placing an object here,
            // and there could be one here already, so test whatever the case is)
            pObj = ae->ObjList[ObjGrid[mPos]];
            if(pObj != NULL)
                pObj->Active = 0;
            //--MEH

            ObjGrid[mPos] = nCode;
            bRedraw = TRUE;
            if (nCode)
                {
                pObj = ae->ObjList[nCode];
                if (pObj != NULL)
                    {
                    yPos = (short)(yCurrent + (yPos / GridSize));
                    xPos = (short)(xCurrent + (xPos / GridSize));
                    pObj->x = (short)((xPos * GRID_WIDTH) + (GRID_WIDTH / 2));
                    pObj->y = (short)((yPos * GRID_WIDTH) + (GRID_WIDTH / 2));
                    pObj->mPos = (short)mPos;
                    pObj->Active = 1;
                    AckSetObjectType(ae,nCode,NO_CREATE);
                    if (pObj->Flags & OF_MOVEABLE)
                        ObjGrid[mPos] = nCode;
                    }
                else
                    ObjGrid[mPos] = 0;
                }
            //MEH Redraw the bitmap list to reflect this object being active/not
            InvalidateRect(MainAppWnd->GetChildHandle(),NULL,FALSE);

            break;
            }

        xPos = (short)(xPos % GridSize);
        yPos = (short)(yPos % GridSize);
        gsLeft = SQUARE_EDGE_SIZE;
        gsRight = (short)(GridSize - SQUARE_EDGE_SIZE / 2);
        if (nCode)
            {
            nMask = 0xFF00;
            if (!nWallFlags)
                nMask = 0;
            else
                nCode |= nWallFlags;
            }
        else
            nMask = 0;


        if (yPos > gsLeft && yPos < gsRight)
            {
            if (xPos < gsLeft)
                {
                //MEH Only redraw if we have to
                if(ae->xGrid[mPos] != nCode)
                    bRedraw = TRUE;
                ae->xGrid[mPos] &= nMask;
                ae->xGrid[mPos] |= nCode;
                }
            else
                {
                if (xPos >= gsRight)
                    {
                    //MEH Only redraw if we have to
                    if(ae->xGrid[mPos+1] != nCode)
                        bRedraw = TRUE;
                    ae->xGrid[mPos+1] &= nMask;
                    ae->xGrid[mPos+1] |= nCode;
                    }
                else
                    {
                    bSetAll = TRUE;
                    }
                }
            }

        if (xPos > gsLeft && xPos < gsRight)
            {
            if (yPos < gsLeft)
                {
                //MEH Only redraw if we have to
                if(ae->yGrid[mPos] != nCode)
                    bRedraw = TRUE;
                ae->yGrid[mPos] &= nMask;
                ae->yGrid[mPos] |= nCode;
                }
            else
                {
                if (yPos >= gsRight)
                    {
                    //MEH Only redraw if we have to
                    if(ae->yGrid[mPos+GRID_WIDTH] != nCode)
                        bRedraw = TRUE;
                    ae->yGrid[mPos+GRID_WIDTH] &= nMask;
                    ae->yGrid[mPos+GRID_WIDTH] |= nCode;
                    }
                else
                    {
                    bSetAll = TRUE;
                    }
                }
            }


        if (bSetAll)
            {
            ae->xGrid[mPos] &= nMask;
            ae->xGrid[mPos] |= nCode;
            ae->xGrid[mPos+1] &= nMask;
            ae->xGrid[mPos+1] |= nCode;
            ae->yGrid[mPos] &= nMask;
            ae->yGrid[mPos] |= nCode;
            ae->yGrid[mPos+GRID_WIDTH] &= nMask;
            ae->yGrid[mPos+GRID_WIDTH] |= nCode;
            bRedraw = TRUE;
            }
        break;

    case VIEW_FLOOR:
        //MEH Only redraw if we have to
        if(FloorMap[mPos] != nCode)
            bRedraw = TRUE;
        FloorMap[mPos] = nCode;
        break;

    case VIEW_CEILING:
        if(CeilMap[mPos] != nCode)
            bRedraw = TRUE;
        CeilMap[mPos] = nCode;
        break;

    default:
        break;
    }

if(!bRedraw)
    return; //Don't redraw if we don't have to
InvalidateRect(hWnd,NULL,TRUE);
InvalidateRect(hwndApp,NULL,FALSE);
}


//****************************************************************************
//
//****************************************************************************
void GrabSquare(HWND hWndChild,short xPos,short yPos)
{
    short   gsLeft,gsRight;
    short   nCode = 0;
    int     mPos;

mPos = GetGridPos(xPos,yPos);

switch (nViewType)
    {
    case VIEW_WALLS:
        if (nEditType == EDIT_OBJECTS)
            {
            nCode = ObjGrid[mPos];
            break;
            }

        xPos = (short)(xPos % GridSize);
        yPos = (short)(yPos % GridSize);
        gsLeft = SQUARE_EDGE_SIZE;
        gsRight = (short)(GridSize - SQUARE_EDGE_SIZE / 2);
        if (yPos > gsLeft && yPos < gsRight)
            {
            if (xPos < gsLeft)
                {
                nCode = ae->xGrid[mPos];
                }
            else
                {
                if (xPos >= gsRight)
                    {
                    nCode = ae->xGrid[mPos+1];
                    }
                }
            }

        if (xPos > gsLeft && xPos < gsRight)
            {
            if (yPos < gsLeft)
                {
                nCode = ae->yGrid[mPos];
                }
            else
                {
                if (yPos >= gsRight)
                    {
                    nCode = ae->yGrid[mPos+GRID_WIDTH];
                    }
                }
            }
        break;

    case VIEW_FLOOR:
        nCode = FloorMap[mPos];
        break;

    case VIEW_CEILING:
        nCode = CeilMap[mPos];
        break;

    default:
        break;
    }

if (nCode)
    {
    CurrentTopBitmap = CurrentBitmap = nCode & 0xFF;
    SetScrollPos(hWndChild,SB_VERT,CurrentTopBitmap,TRUE);
    InvalidateRect(hWndChild,NULL,TRUE);
    }


}

//****************************************************************************
//
//****************************************************************************
void RemapBufferPalettes(void)
{
    HBITMAP     hbm;
    short       ht,wt,Counter;
    RECT        rect;

CreateAckPalette();

for (Counter = 0;Counter < 256;Counter++)
    {
    BufferHeader.aColors[Counter].rgbRed   = ColorTable[Counter].rgbRed;
    BufferHeader.aColors[Counter].rgbGreen = ColorTable[Counter].rgbGreen;
    BufferHeader.aColors[Counter].rgbBlue  = ColorTable[Counter].rgbBlue;
    BufferHeader.aColors[Counter].rgbReserved = 0;
    }

GetClientRect(hWndMap,&rect);
wt = rect.right;
ht = rect.bottom;

if (MapBuffer)
    {
    //BufferHeader.Header.biWidth = wt;
    //BufferHeader.Header.biHeight = -ht;

    hbm = CreateCompatibleBitmap(MapBuffer,wt,ht);

    hbm = (HBITMAP)SelectObject(MapBuffer,hbm);
    DeleteObject(hbm);

    PatBlt(MapBuffer,0,0,wt,ht,BLACKNESS);
    }
else
    {
    /*if (WinG.pRecommendDIBFormat((BITMAPINFO *)&BufferHeader))
        {
        BufferHeader.Header.biBitCount = 8;
        BufferHeader.Header.biCompression = BI_RGB;
        }
    else
        {
        BufferHeader.Header.biSize = sizeof(BITMAPINFOHEADER);
        BufferHeader.Header.biPlanes = 1;
        BufferHeader.Header.biBitCount = 8;
        BufferHeader.Header.biCompression = BI_RGB;
        BufferHeader.Header.biSizeImage = 0;
        BufferHeader.Header.biClrUsed = 0;
        BufferHeader.Header.biClrImportant = 0;
        }

    BufferHeader.Header.biWidth = wt;
    BufferHeader.Header.biHeight = -ht;*/

    MapBuffer = CreateCompatibleDC(GetDC(hWndMap));
    hbm = CreateCompatibleBitmap(GetDC(hWndMap),wt, ht);

    SelectObject(MapBuffer,hbm);
    PatBlt(MapBuffer, 0,0,wt,ht, BLACKNESS);
    }


if (Buffer)
    {
    //BufferHeader.Header.biWidth = 64;
    //BufferHeader.Header.biHeight = -350;

    hbm = CreateCompatibleBitmap(Buffer,64,350);

    hbm = (HBITMAP)SelectObject(Buffer,hbm);
    DeleteObject(hbm);

    PatBlt(Buffer,0,0,wt,ht,BLACKNESS);
    }
else
    {
    /*if (WinG.pRecommendDIBFormat((BITMAPINFO *)&BufferHeader))
        {
        BufferHeader.Header.biBitCount = 8;
        BufferHeader.Header.biCompression = BI_RGB;
        }
    else
        {
        BufferHeader.Header.biSize = sizeof(BITMAPINFOHEADER);
        BufferHeader.Header.biPlanes = 1;
        BufferHeader.Header.biBitCount = 8;
        BufferHeader.Header.biCompression = BI_RGB;
        BufferHeader.Header.biSizeImage = 0;
        BufferHeader.Header.biClrUsed = 0;
        BufferHeader.Header.biClrImportant = 0;
        }

    BufferHeader.Header.biWidth = 64;
    BufferHeader.Header.biHeight = -350;*/

    Buffer = CreateCompatibleDC(GetDC(hWndMap));
    hbm = CreateCompatibleBitmap(GetDC(hWndMap), 64, 350);

    SelectObject(Buffer,hbm);
    PatBlt(Buffer, 0,0,wt,ht, BLACKNESS);
    }


if (AckBuffer)
    {
    //BufferHeader.Header.biWidth = nBitmapW;
    //BufferHeader.Header.biHeight = -nBitmapH;   // -height for top-down display

    hbm = CreateCompatibleBitmap(AckBuffer,nBitmapW,nBitmapH);

    hbm = (HBITMAP)SelectObject(AckBuffer, hbm);
    DeleteObject(hbm);

    PatBlt(AckBuffer, 0,0,nBitmapW,nBitmapH, BLACKNESS);

    if (ae != NULL)
        {
//        ae->ScreenBuffer = (UCHAR *)pAckBuffer;
        ae->WinEndX = (short)nBitmapW;
        ae->WinEndY = (short)nBitmapH;
        if (ae->WinEndX > 319)
            ae->WinEndX = 319;
        if (ae->WinEndY > 199)
            ae->WinEndY = 199;
        AckRegisterStructure(ae);
        }
    }
else
    {
    /*if (WinG.pRecommendDIBFormat((BITMAPINFO *)&BufferHeader))
        {
        BufferHeader.Header.biBitCount = 8;
        BufferHeader.Header.biCompression = BI_RGB;
        }
    else
        {
        BufferHeader.Header.biSize = sizeof(BITMAPINFOHEADER);
        BufferHeader.Header.biPlanes = 1;
        BufferHeader.Header.biBitCount = 8;
        BufferHeader.Header.biCompression = BI_RGB;
        BufferHeader.Header.biSizeImage = 0;
        BufferHeader.Header.biClrUsed = 0;
        BufferHeader.Header.biClrImportant = 0;
        }

    BufferHeader.Header.biWidth = nBitmapW;
    BufferHeader.Header.biHeight = -nBitmapH;*/

    AckBuffer = CreateCompatibleDC(GetDC(hwndApp));
    hbm = CreateCompatibleBitmap(GetDC(hwndApp),nBitmapW,nBitmapH);

    SelectObject(AckBuffer, hbm);

    PatBlt(AckBuffer, 0,0,nBitmapW,nBitmapH, BLACKNESS);
    if (ae != NULL)
        {
//        ae->ScreenBuffer = (UCHAR *)pAckBuffer;
        AckRegisterStructure(ae);
        }
    }

}

//****************************************************************************
//
//****************************************************************************
void ResizeMapBuffer(HWND hWnd,LPARAM lParam)
{
    HBITMAP hbm;
    short   ht,wt;
    short   gWt,gHt;

wt = LOWORD(lParam);
ht = HIWORD(lParam);
wt = ((wt+3)/4)*4;
GridWidth = wt;
GridHeight = ht;
gHt = (short)(64 - (ht / GridSize));
if (gHt < 1) gHt = 1;
gWt = (short)(64 - (wt / GridSize));
if (gWt < 1) gWt = 1;
SetScrollRange(hWnd,SB_VERT,0,gHt,FALSE);
SetScrollRange(hWnd,SB_HORZ,0,gWt,FALSE);
SetScrollPos(hWnd,SB_VERT,0,TRUE);
SetScrollPos(hWnd,SB_HORZ,0,TRUE);
xCurrent = yCurrent = 0;

if (MapBuffer)
    {
    BufferHeader.Header.biWidth = wt;
    BufferHeader.Header.biHeight = -ht;

    hbm = CreateCompatibleBitmap(GetDC(hWndMap),wt,ht);

    hbm = (HBITMAP)SelectObject(MapBuffer, hbm);
    DeleteObject(hbm);

    PatBlt(MapBuffer, 0,0,wt,ht, BLACKNESS);
    }
else
    {
    /*if (WinG.pRecommendDIBFormat((BITMAPINFO *)&BufferHeader))
        {
        BufferHeader.Header.biBitCount = 8;
        BufferHeader.Header.biCompression = BI_RGB;
        }
    else
        {
        BufferHeader.Header.biSize = sizeof(BITMAPINFOHEADER);
        BufferHeader.Header.biPlanes = 1;
        BufferHeader.Header.biBitCount = 8;
        BufferHeader.Header.biCompression = BI_RGB;
        BufferHeader.Header.biSizeImage = 0;
        BufferHeader.Header.biClrUsed = 0;
        BufferHeader.Header.biClrImportant = 0;
        }

    BufferHeader.Header.biWidth = wt;
    BufferHeader.Header.biHeight = -ht;

    for (Counter = 0;Counter < 256;Counter++)
        {
        BufferHeader.aColors[Counter].rgbRed   = ColorTable[Counter].rgbRed;
        BufferHeader.aColors[Counter].rgbGreen = ColorTable[Counter].rgbGreen;
        BufferHeader.aColors[Counter].rgbBlue  = ColorTable[Counter].rgbBlue;
        BufferHeader.aColors[Counter].rgbReserved = 0;
        }*/

    MapBuffer = CreateCompatibleDC(GetDC(hWndMap));
    hbm = CreateCompatibleBitmap(GetDC(hWndMap),wt,ht);

    SelectObject(MapBuffer, hbm);
    PatBlt(MapBuffer, 0,0,wt,ht, BLACKNESS);
    }

}

//****************************************************************************
//
//****************************************************************************
void DeleteBitmap(HWND hWnd,HWND hwndChild)
{
    short   mPos;

if (nEditType == EDIT_WALLS)
    {
    if (ae->bMaps[CurrentBitmap] == NULL)
        return;

    for (mPos = 0; mPos < 4096; mPos++)
        {
        if (CurrentBitmap == (ae->xGrid[mPos] & 0xFF))
            ae->xGrid[mPos] = 0;
        if (CurrentBitmap == (ae->yGrid[mPos] & 0xFF))
            ae->yGrid[mPos] = 0;
        if (CurrentBitmap == (FloorMap[mPos] & 0xFF))
            FloorMap[mPos] = 0;
        if (CurrentBitmap == (CeilMap[mPos] & 0xFF))
            CeilMap[mPos] = 0;
        }
    AckFree(ae->bMaps[CurrentBitmap]);
    ae->bMaps[CurrentBitmap] = NULL;
    }
else
    {
    if (ae->ObjList[CurrentBitmap] == NULL)
        return;

    for (mPos = 0; mPos < 4096; mPos++)
        {
        if (CurrentBitmap == (ObjGrid[mPos] & 0xFF))
            ObjGrid[mPos] = 0;
        }

    AckFree(ae->ObjList[CurrentBitmap]);
    ae->ObjList[CurrentBitmap] = NULL;
    }


InvalidateRect(hWnd,NULL,TRUE);
InvalidateRect(hwndChild,NULL,TRUE);
}

//****************************************************************************
//
//****************************************************************************
void FillBorder(void)
{
    short   row,col,offset;

switch (nViewType)
    {
    case VIEW_WALLS:
        offset = 0;
        for (col = 0; col < 64; col++)
            {
            ae->yGrid[col] = CurrentBitmap;
            }
        for (row = 0; row < 64; row++)
            {
            ae->xGrid[offset] = CurrentBitmap;
            ae->xGrid[offset+63] = CurrentBitmap;
            offset += (short)64;
            }

        offset = 64 * 63;
        for (col = 0; col < 64; col++)
            {
            ae->yGrid[offset++] = CurrentBitmap;
            }
        break;

    case VIEW_FLOOR:
        offset = 64 * 63;
        for (col = 0; col < 64; col++)
            {
            FloorMap[col] = CurrentBitmap;
            FloorMap[offset++] = CurrentBitmap;
            }
        offset = 0;
        for (row = 0; row < 64; row++)
            {
            FloorMap[offset] = CurrentBitmap;
            FloorMap[offset+63] = CurrentBitmap;
            offset += (short)64;
            }
        break;

    case VIEW_CEILING:
        offset = 64 * 63;
        for (col = 0; col < 64; col++)
            {
            CeilMap[col] = CurrentBitmap;
            CeilMap[offset++] = CurrentBitmap;
            }
        offset = 0;
        for (row = 0; row < 64; row++)
            {
            CeilMap[offset] = CurrentBitmap;
            CeilMap[offset+63] = CurrentBitmap;
            offset += (short)64;
            }
        break;

    default:
        break;
    }

SetUndoPoint();
}


//****************************************************************************
//
//****************************************************************************
void FillEntireMap(short nCode)
{
    short   i;

switch (nViewType)
    {
    case VIEW_WALLS:
        //MEH Clear objects if we're editing them, rather than just blindly clearing walls
        if(nEditType == EDIT_OBJECTS)
        {
            //Loop through all grid positions
            for (i = 0; i < 4096; i++)
            {
                //Grab the object at this position
                unsigned short nCode = ObjGrid[i];
                if(nCode)   //If there's one here,
                {
                    //Grab the object at this position
                    NEWOBJECT* pObj = ae->ObjList[nCode];
                    if(pObj != NULL)
                    {
                        pObj->Active = 0;   //MAKE IT INACTIVE BEFORE YOU DELETE IT
                    }
                    ObjGrid[i] = 0; //Remove it
                }
            }
        }
        else    //Otherwise, clear the walls
        {
            for (i = 0; i < 4096; i++)
            {
                ae->xGrid[i] = nCode;
                ae->yGrid[i] = nCode;
            }
        }
        break;

    case VIEW_FLOOR:
        for (i = 0; i < 4096; i++)
            FloorMap[i] = nCode;
        break;

    case VIEW_CEILING:
        for (i = 0; i < 4096; i++)
            CeilMap[i] = nCode;
        break;

    default:
        break;
    }

SetUndoPoint();
}

//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
short GetLoadType(char *szName)
{
    short   nType = BMLOAD_BBM;

if (strstr(szName,".pcx") || strstr(szName,".PCX"))
    nType = BMLOAD_PCX;
else if (strstr(szName,".gif") || strstr(szName,".GIF"))
    nType = BMLOAD_GIF;

return(nType);
}


//****************************************************************************
//
//****************************************************************************
void AddFileName(short nType,char *szName)
{
if (nType == TYPE_WALL)
    {
    if (bMapFiles[CurrentBitmap] != NULL)
        AckFree(bMapFiles[CurrentBitmap]);
    bMapFiles[CurrentBitmap] = (char *)AckMalloc(strlen(szName)+1);
    lstrcpy(bMapFiles[CurrentBitmap],szName);
    bMapType[CurrentBitmap] = GetLoadType(szName);
    }
else
    {
    if (oMapFiles[CurrentBitmap] != NULL)
        AckFree(oMapFiles[CurrentBitmap]);
    oMapFiles[CurrentBitmap] = (char *)AckMalloc(strlen(szName)+1);
    lstrcpy(oMapFiles[CurrentBitmap],szName);
    oMapType[CurrentBitmap] = GetLoadType(szName);
    }

SetUndoPoint();
}

//****************************************************************************
//
//****************************************************************************
void RemoveFileName(short nType,short nIndex)
{
if (nType == TYPE_WALL)
    {
    if (bMapFiles[nIndex] != NULL)
        {
        AckFree(bMapFiles[nIndex]);
        bMapFiles[nIndex] = NULL;
        }
    }
else
    {
    if (oMapFiles[nIndex] != NULL)
        {
        AckFree(oMapFiles[nIndex]);
        oMapFiles[nIndex] = NULL;
        }
    }
}

//****************************************************************************
//
//****************************************************************************
void UpdateChildText(HWND hWnd)
{
    char    szBuf[50];

if (nEditType == EDIT_WALLS)
{
    wsprintf(szBuf,"Wall: %d",CurrentBitmap);
    //MEH Also update the scroll range
    SetScrollRange(hWnd,SB_VERT,0,252,TRUE);
}
else
{
    //Force scroll position to be in range
    if(CurrentTopBitmap > 250)
        CurrentTopBitmap = 250;
    if(CurrentBitmap > 253)
        CurrentBitmap = 253;
    wsprintf(szBuf,"Obj: %d",CurrentBitmap);
    SetScrollRange(hWnd,SB_VERT,0,250,TRUE);
}

SetWindowText(hWnd,szBuf);
}

//****************************************************************************
//  MEH Update the parent text
//****************************************************************************
void UpdateParentText(HWND hWnd)
{
    char buf[256];
    switch(nViewType)
    {
        case VIEW_WALLS:
            if (bShowCoords)
            {
                sprintf(buf,"Map Editor - Walls Row:%d,Col:%d",ae->yPlayer >> 6,ae->xPlayer >> 6);
                //SetWindowText(hWnd,buf);
            }
            else
            {
                sprintf(buf,"Map Editor - Walls");
            }
            break;
        case VIEW_FLOOR:
            if (bShowCoords)
            {
                sprintf(buf,"Map Editor - Floors Row:%d,Col:%d",ae->yPlayer >> 6,ae->xPlayer >> 6);
                //SetWindowText(hWnd,buf);
            }
            else
            {
                sprintf(buf,"Map Editor - Floors");
            }
            break;
        case VIEW_CEILING:
            if (bShowCoords)
            {
                sprintf(buf,"Map Editor - Ceilings Row:%d,Col:%d",ae->yPlayer >> 6,ae->xPlayer >> 6);
                //SetWindowText(hWnd,buf);
            }
            else
            {
                sprintf(buf,"Map Editor - Ceilings");
            }
            break;
    }

    //MEH Now see if map is modified and add asterisk at end if it is
    if(bModified)
    {
        char buf2[256];
        sprintf(buf2, "*");
        //GetWindowText(hWnd, buf, 256);
        SetWindowText(hWnd, strcat(buf2, buf));
    }
    else
        SetWindowText(hWnd, buf);
}


//****************************************************************************
//
//****************************************************************************
LRESULT MainWindow::WndProc(UINT iMessage, WPARAM wParam, LPARAM lParam )
{
    short           mx,my,nItem;
    HDC             hDC;
    LPMINMAXINFO    lpmmi;
    LPWINDOWPOS     lpwp;
    char            szName[128];


    switch (iMessage)
        {
        case WM_CREATE:
            SetScrollRange(hWnd,SB_VERT,0,64,TRUE);
            SetScrollRange(hWnd,SB_HORZ,0,64,TRUE);
            break;

        case WM_CLOSE:
            if (bModified)
            {
                int iResult = MessageBox(hWnd,"Map has been modified. Save changes before exit?", "Exit", MB_YESNOCANCEL);
                if(iResult == IDYES)
                {
                    SetCurrentDirectory(szCurPath);
                    SaveResource(hWnd,ResFileName);
                }
                else if(iResult == IDCANCEL)
                {
                    break;
                }
                else if(iResult == IDNO)
                {
                    char cFilename[256];
                    sprintf(cFilename, "pics u%d.dtf",g_sSaveUndo);
                    if(CopyFile(cFilename,"pics.dtf"))
                        CopyFile("pics r0.dtf", "pics.dtf");
                }
            }
            DestroyWindow(hWnd);
            return(0);

        case WM_ERASEBKGND:
            return(1L);

        case WM_SIZE:
            ResizeMapBuffer(hWnd,lParam);
            break;

        case WM_VSCROLL:
            ScrollMap(hWnd,LOWORD(wParam),HIWORD(wParam),1);
            break;

        //MEH Mousewheel scrolls up and down too
        case WM_MOUSEWHEEL:
            if((short)(HIWORD(wParam)) > 0)  //Rotated wheel up
            {
                //If the screen is set to track the player, scroll bitmaps instead
                if(bTrackPlayer)
                {
                    ScrollBitmaps(MainAppWnd->GetChildHandle(), SB_LINEUP, 0);
                    break;
                }
                //Otherwise, scroll the window
                if(GetAsyncKeyState(VK_SHIFT) < 0 || GetAsyncKeyState(VK_CONTROL) < 0)  //If pressing shift or ctrl, scroll horizontally
                {
                    ScrollMap(hWnd, SB_LINEUP, 0, 0);
                }
                else
                    ScrollMap(hWnd, SB_LINEUP, 0, 1);
            }
            else    //Rotated wheel down
            {
                //If the screen is set to track the player, scroll bitmaps instead
                if(bTrackPlayer)
                {
                    ScrollBitmaps(MainAppWnd->GetChildHandle(), SB_LINEDOWN, 0);
                    break;
                }
                if(GetAsyncKeyState(VK_SHIFT) < 0 || GetAsyncKeyState(VK_CONTROL) < 0)  //If pressing shift or ctrl, scroll horizontally
                {
                    ScrollMap(hWnd, SB_LINEDOWN, 0, 0);
                }
                else
                    ScrollMap(hWnd, SB_LINEDOWN, 0, 1);
            }
            break;

        case WM_HSCROLL:
            ScrollMap(hWnd,LOWORD(wParam),HIWORD(wParam),0);
            break;

        case WM_PAINT:
            Paint();
            break;


        case WM_SETFOCUS:
            InvalidateRect(hWnd,NULL,FALSE);
            return DefWindowProc( hWnd, iMessage, wParam, lParam );

        case WM_KEYDOWN:
            ProcessKeys(iMessage,wParam);
            HandleGenericKeypresses(wParam);
            switch(wParam)
            {
                case VK_F1:
                    wsprintf(LineBuffer,"ACK Mem used: %ld",AckMemUsed);
                    MessageBox(hWnd,LineBuffer,"Info",MB_OK);
                    break;
                case VK_SPACE:
                    if(!g_bPanning) //Allow panning using the spacebar also
                    {
                        POINT ptCursorPos;
                        GetCursorPos(&ptCursorPos);
                        g_ptStartPanLocation.x = ptCursorPos.x;
                        g_ptStartPanLocation.y = ptCursorPos.y;
                        g_bPanning = true;
                        SetCursor(g_cPan);
                    }
                    break;
            }
            break;

        case WM_KEYUP:
            ProcessKeys(iMessage,wParam);
            InvalidateRect(hWnd,NULL,FALSE);
            switch(wParam)
            {
                case VK_SPACE:
                    g_bPanning = false; //Stop panning view
                    SetCursor(g_cArrow);
                    break;
            }
            break;

        //MEH Pan view with MMB
        case WM_MBUTTONDOWN:
            if(!g_bPanning)
            {
                g_ptStartPanLocation.x = LOWORD(lParam);
                g_ptStartPanLocation.y = HIWORD(lParam);
                g_bPanning = true;
                SetCursor(g_cPan);
            }
            break;

        case WM_MBUTTONUP:
            g_bPanning = false; //Stop panning view
            SetCursor(g_cArrow);
            break;

        case WM_MOUSEMOVE:
            //Set the mouse cursor back to default
            SetCursor(g_cArrow);

            //Allow panning view if holding MMB or spacebar
            if((wParam & MK_MBUTTON || GetAsyncKeyState(VK_SPACE) < 0) && g_bPanning)
            {
                SetCursor(g_cPan);
                int xPos = LOWORD(lParam);
                int yPos = HIWORD(lParam);
                //Pulling left - scroll right
                if(g_ptStartPanLocation.x - xPos >= GridSize)
                {
                    g_ptStartPanLocation.x -= GridSize;
                    ScrollMap(hWnd, SB_LINEDOWN, 0, 0);
                }
                //Pulling right - scroll left
                if(xPos - g_ptStartPanLocation.x >= GridSize)
                {
                    g_ptStartPanLocation.x += GridSize;
                    ScrollMap(hWnd, SB_LINEUP, 0, 0);
                }
                //Pulling up - scroll down
                if(g_ptStartPanLocation.y - yPos >= GridSize)
                {
                    g_ptStartPanLocation.y -= GridSize;
                    ScrollMap(hWnd, SB_LINEDOWN, 0, 1);
                }
                //Pulling down - scroll up
                if(yPos - g_ptStartPanLocation.y >= GridSize)
                {
                    g_ptStartPanLocation.y += GridSize;
                    ScrollMap(hWnd, SB_LINEUP, 0, 1);
                }
                break;  //Stop here
            }

            //If they're not pressing left mouse button, don't keep placing bitmaps
            if(!(wParam & MK_LBUTTON))
            {
                //Also allow user to remove bitmaps by holding down RMB
                if(wParam & MK_RBUTTON)
                    SetBitmap(hWnd,LOWORD(lParam),HIWORD(lParam),0);
                break;
            }
            //Of course, not for objects, since you place these one at a time
            if(nEditType == EDIT_OBJECTS)
                break;
            //MEH Allow click & drag to keep placing bitmaps, by falling through here
        case WM_LBUTTONDOWN:
            mx = LOWORD(lParam);
            my = HIWORD(lParam);
            if (wParam & MK_CONTROL && wParam & MK_SHIFT) //MEH Grab map pos # if pressing Ctrl+Shift+LMB
            {
                ShowMapPosn(hWnd,mx,my);
            }

            else if (wParam & MK_CONTROL)
            {
                GrabSquare(hWndChild,mx,my);
                g_bNotModifyOnMouseUp = true;
                break;
            }

            else if (wParam & MK_SHIFT)
            {
                my = (short)(yCurrent + (my / GridSize));
                mx = (short)(xCurrent + (mx / GridSize));
                ae->xPlayer = (short)(mx * GRID_WIDTH);
                ae->yPlayer = (short)(my * GRID_WIDTH);
                InvalidateRect(hWnd,NULL,TRUE);
                SetUndoPoint();
                g_bNotModifyOnMouseUp = true;
            }
            else
                SetBitmap(hWnd,mx,my,CurrentBitmap);
            break;

        case WM_LBUTTONUP:
            if(!g_bNotModifyOnMouseUp)
                SetUndoPoint();
            g_bNotModifyOnMouseUp = false;
            break;

        case WM_RBUTTONDOWN:
            SetBitmap(hWnd,LOWORD(lParam),HIWORD(lParam),0);
            break;

        case WM_RBUTTONUP:
            SetUndoPoint();
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        case WM_INITMENU:
            switch (nViewType)
                {
                case VIEW_WALLS:
                    nItem = CM_VIEWWALLS;
                    break;

                case VIEW_FLOOR:
                    nItem = CM_VIEWFLOOR;
                    break;

                case VIEW_CEILING:
                    nItem = CM_VIEWCEILING;
                    break;

                default:
                    nItem = 0;
                    break;
                }
            CheckMenuItem((HMENU)wParam,CM_VIEWWALLS,MF_UNCHECKED|MF_BYCOMMAND);
            CheckMenuItem((HMENU)wParam,CM_VIEWFLOOR,MF_UNCHECKED|MF_BYCOMMAND);
            CheckMenuItem((HMENU)wParam,CM_VIEWCEILING,MF_UNCHECKED|MF_BYCOMMAND);
            CheckMenuItem((HMENU)wParam,nItem,MF_CHECKED|MF_BYCOMMAND);

            CheckMenuItem((HMENU)wParam,CM_VIEWTIMES_ONE,MF_UNCHECKED|MF_BYCOMMAND);
            CheckMenuItem((HMENU)wParam,CM_VIEWTIMES_TWO,MF_UNCHECKED|MF_BYCOMMAND);
            if (StretchFactor == 1)
                nItem = CM_VIEWTIMES_ONE;
            else
                nItem = CM_VIEWTIMES_TWO;
            CheckMenuItem((HMENU)wParam,nItem,MF_CHECKED|MF_BYCOMMAND);

            nItem = CM_EDITMAP;
            if (nEditType == EDIT_OBJECTS)
            {
                nItem = CM_EDITOBJECTS;
                //MEH Make menu say you can Tab into other edit mode
                ModifyMenu((HMENU)wParam,CM_EDITMAP,MF_STRING,CM_EDITMAP,"&Map\tTab");
                ModifyMenu((HMENU)wParam,CM_EDITOBJECTS,MF_STRING,CM_EDITOBJECTS,"&Objects");
                //MEH Also make copy/paste object selectable
                ModifyMenu((HMENU)wParam,CM_EDITCOPYOBJECT,MF_STRING,CM_EDITCOPYOBJECT,"&Copy Object\tCtrl+C");
                ModifyMenu((HMENU)wParam,CM_EDITPASTEOBJECT,MF_STRING,CM_EDITPASTEOBJECT,"&Paste Object\tCtrl+V");
            }
            else
            {
                ModifyMenu((HMENU)wParam,CM_EDITMAP,MF_STRING,CM_EDITMAP,"&Map");
                ModifyMenu((HMENU)wParam,CM_EDITOBJECTS,MF_STRING,CM_EDITOBJECTS,"&Objects\tTab");
                //MEH Make copy/paste object unselectable
                ModifyMenu((HMENU)wParam,CM_EDITCOPYOBJECT,MF_STRING | MF_GRAYED,CM_EDITCOPYOBJECT,"&Copy Object\tCtrl+C");
                ModifyMenu((HMENU)wParam,CM_EDITPASTEOBJECT,MF_STRING | MF_GRAYED,CM_EDITPASTEOBJECT,"&Paste Object\tCtrl+V");
            }

            //MEH Only allow undo/redo if it's possible
            if(g_sNumUndo)
                ModifyMenu((HMENU)wParam,CM_EDITUNDO,MF_STRING,CM_EDITUNDO,"&Undo\tCtrl+Z");
            else
                ModifyMenu((HMENU)wParam,CM_EDITUNDO,MF_STRING | MF_GRAYED,CM_EDITUNDO,"&Undo\tCtrl+Z");
            if(g_sNumRedo)
                ModifyMenu((HMENU)wParam,CM_EDITREDO,MF_STRING,CM_EDITREDO,"&Redo\tCtrl+Y");
            else
                ModifyMenu((HMENU)wParam,CM_EDITREDO,MF_STRING | MF_GRAYED,CM_EDITREDO,"&Redo\tCtrl+Y");

            CheckMenuItem((HMENU)wParam,CM_EDITMAP,MF_UNCHECKED|MF_BYCOMMAND);
            CheckMenuItem((HMENU)wParam,CM_EDITOBJECTS,MF_UNCHECKED|MF_BYCOMMAND);
            CheckMenuItem((HMENU)wParam,nItem,MF_CHECKED|MF_BYCOMMAND);

            nItem = CM_WALLS_NORMAL;
            if (nWallFlags & DOOR_TYPE_SPLIT)
               nItem = CM_WALLS_SPLIT;
            if (nWallFlags & DOOR_TYPE_SLIDE)
               nItem = CM_WALLS_SLIDING;
            if (nWallFlags & DOOR_TYPE_SECRET)
               nItem = CM_WALLS_SECRET;

            CheckMenuItem((HMENU)wParam,CM_WALLS_NORMAL,MF_UNCHECKED|MF_BYCOMMAND);
            CheckMenuItem((HMENU)wParam,CM_WALLS_SPLIT,MF_UNCHECKED|MF_BYCOMMAND);
            CheckMenuItem((HMENU)wParam,CM_WALLS_SLIDING,MF_UNCHECKED|MF_BYCOMMAND);
            CheckMenuItem((HMENU)wParam,CM_WALLS_SECRET,MF_UNCHECKED|MF_BYCOMMAND);
            CheckMenuItem((HMENU)wParam,nItem,MF_CHECKED|MF_BYCOMMAND);

            mx = MF_UNCHECKED;
            if (nWallFlags & WALL_TYPE_TRANS)
               mx = MF_CHECKED;

            mx |= MF_BYCOMMAND;
            CheckMenuItem((HMENU)wParam,CM_WALLS_TRANSPARENT,mx);

            mx = MF_UNCHECKED;
            if (nWallFlags & WALL_TYPE_PASS)
               mx = MF_CHECKED;

            mx |= MF_BYCOMMAND;
            CheckMenuItem((HMENU)wParam,CM_WALLS_PASSABLE,mx);

            if (ae->SysFlags & SYS_SOLID_FLOOR)
               mx = MF_CHECKED;
            else
               mx = MF_UNCHECKED;
            mx |= MF_BYCOMMAND;
//            CheckMenuItem((HMENU)wParam,CM_GOODIESSOLID_FLOOR,mx);

            if (ae->SysFlags & SYS_SOLID_CEIL)
               mx = MF_CHECKED;
            else
               mx = MF_UNCHECKED;
            mx |= MF_BYCOMMAND;
//            CheckMenuItem((HMENU)wParam,CM_GOODIESSOLID_CEILING,mx);

            if (ae->LightFlag == SHADING_OFF)
                mx = MF_UNCHECKED;
            else
                mx = MF_CHECKED;

            mx |= MF_BYCOMMAND;
            CheckMenuItem((HMENU)wParam,CM_GOODIESLIGHT_SHADING,mx);

            if (ae->SysFlags & SYS_SINGLE_BMP)
                mx = MF_CHECKED;
            else
                mx = MF_UNCHECKED;

            mx |= MF_BYCOMMAND;
//            CheckMenuItem((HMENU)wParam,CM_GOODIESSINGLE_BITMAP,mx);

            if (nEditType == EDIT_WALLS)
                ModifyMenu((HMENU)wParam,CM_GOODIESATTRIBUTES,MF_STRING,
                            CM_GOODIESATTRIBUTES,"&Wall Attributes ...");
            else
                ModifyMenu((HMENU)wParam,CM_GOODIESATTRIBUTES,MF_STRING,
                            CM_GOODIESATTRIBUTES,"&Object Attributes ...");


            if (bTrackPlayer)
                mx = MF_CHECKED;
            else
                mx = MF_UNCHECKED;
            mx |= MF_BYCOMMAND;
            CheckMenuItem((HMENU)wParam,CM_GOODIESTRACK_PLAYER,mx);

            if (bShowCoords)
                mx = MF_CHECKED;
            else
                mx = MF_UNCHECKED;
            mx |= MF_BYCOMMAND;
            CheckMenuItem((HMENU)wParam,CM_GOODIESSHOW_COORDINATES,mx);

            break;

        case WM_GETMINMAXINFO:
            lpmmi = (LPMINMAXINFO)lParam;
            mx = (short)(GridSize * 24);
            lpmmi->ptMaxSize.x = mx;
            lpmmi->ptMaxSize.y = mx;
            lpmmi->ptMaxTrackSize.x = mx;
            lpmmi->ptMaxTrackSize.y = mx;
            lpmmi->ptMinTrackSize.x = GridSize;
            lpmmi->ptMinTrackSize.y = GridSize;
            lpmmi->ptMaxPosition.x = 0;
            lpmmi->ptMaxPosition.y = 0;
            break;


        case WM_PALETTECHANGED:
            if (hWnd == (HWND)wParam)
                break;
            // Fall through here
        case WM_QUERYNEWPALETTE:
            hDC = GetDC(hWnd);
            if (hpalApp)
                SelectPalette(hDC,hpalApp,FALSE);
            RealizePalette(hDC);
            ReleaseDC(hWnd,hDC);
            return FALSE;

        case WM_WINDOWPOSCHANGING:
            lpwp = (LPWINDOWPOS)lParam;
            lpwp->cx = (lpwp->cx / GridSize) * GridSize;
            lpwp->cy = (lpwp->cy / GridSize) * GridSize;
            lpwp->flags |= SWP_NOCOPYBITS;
            return 0;

        case WM_COMMAND:
            switch (wParam)
                {
                case CM_FILEOPEN:
                    if (!ACKFileOpen(hWnd,ResFileName,sizeof(ResFileName),
                        "Resource Files (*.dtf)\0*.dtf\0All Files (*.*)\0*.*\0",
                        "Open Resource File","dtf"))
                        {
                        if (!AckOpenResource(ResFileName))
                            {
                            bNoShow = TRUE;
                            AckReset();
                            mx = (short)ProcessInfoFile(0);
                            xCurrent = yCurrent = 0;
                            CurrentTopBitmap = CurrentBitmap = 1;
                            if (mx)
                                {
                                wsprintf(szName,"Error: %d in DTF file at line %d",mx,LineNumber);
                                MessageBox(hWnd,szName,"ERROR",MB_OK);
                                }
                            else
                                {
                                LoadBackDrop();
                                AckRegisterStructure(ae);
                                bNoShow = FALSE;
                                }

                        //  AckDoSize(hWnd,0);
                        //  CreateAckPalette();
                            RemapBufferPalettes();
                            InvalidateRect(hWnd,NULL,TRUE);
                            InvalidateRect(hWndChild,NULL,TRUE);
                            }
                        }
                    break;

                //MEH Allow user to wipe out map easily, since it's kinda mundane atm
                case CM_FILENEWMAP:
                    CreateNewMap(hWnd, hWndChild);
                    break;

                case CM_FILESAVE:
                     g_sSaveUndo = g_sNumUndo;
                     bModified = FALSE;
                     break;

                case CM_FILESAVE_AS_:   //Disabled
                    /*strcpy(szName,ResFileName);
                    if (!ACKFileSave(hWnd,szName,sizeof(szName),
                        "Resource Files (*.dtf)\0*.dtf\0All Files (*.*)\0*.*\0",
                        "Save Resource File"))
                        {
                        SaveResource(hWnd,szName);
                        bModified = FALSE;
                        strcpy(ResFileName,szName);
                        GetCurrentDirectory(sizeof(szCurPath),szCurPath);
                        }*/
                    break;


                case CM_FILEEXIT:
                    if (bModified)
                    {
                        //MEH Standard yes/no/cancel message box for close w / w/o saving
                        int iResult = MessageBox(hWnd,"Map has been modified. Save changes before exit?", "Exit", MB_YESNOCANCEL);
                        if(iResult == IDYES)
                        {
                            SetCurrentDirectory(szCurPath);
                            SaveResource(hWnd,ResFileName);
                        }
                        else if(iResult == IDCANCEL)
                        {
                            break;
                        }
                        else if(iResult == IDNO)
                        {
                            char cFilename[256];
                            sprintf(cFilename, "pics u%d.dtf",g_sSaveUndo);
                            CopyFile(cFilename,"pics.dtf");
                        }
                    }
                    PostQuitMessage(0);
                    break;

                //MEH Undo/redo
                case CM_EDITUNDO:
                    if (g_sNumUndo)
                        Undo();
                    break;

                case CM_EDITREDO:
                    if (g_sNumRedo)
                        Redo();
                    break;

                //MEH Copy item to clipboard
                case CM_EDITCOPYOBJECT:
                    if(nEditType == EDIT_OBJECTS)
                        g_sClipboard = CurrentBitmap;
                    break;

                //MEH Paste item from clipboard
                case CM_EDITPASTEOBJECT:
                    if(CurrentBitmap != g_sClipboard && g_sClipboard)
                    {
                        if (ae->ObjList[CurrentBitmap] == NULL)
                        {
                            AckCreateObject(ae,CurrentBitmap);
                            AckSetObjectType(ae,CurrentBitmap,NO_CREATE);
                        }
                        //Hold onto data we want to not copy (Namely position and active/not)
                        short mPos = ae->ObjList[CurrentBitmap]->mPos;
                        short x = ae->ObjList[CurrentBitmap]->x;
                        short y = ae->ObjList[CurrentBitmap]->y;
                        char active = ae->ObjList[CurrentBitmap]->Active;
                        //Copy other object onto this one
                        CopyObject(ae->ObjList[g_sClipboard], ae->ObjList[CurrentBitmap]);
                        //Reset data we didn't want copied
                        ae->ObjList[CurrentBitmap]->Active = active;
                        ae->ObjList[CurrentBitmap]->mPos = mPos;
                        ae->ObjList[CurrentBitmap]->x = x;
                        ae->ObjList[CurrentBitmap]->y = y;
                        //Redraw the bitmap window
                        InvalidateRect(MainAppWnd->GetChildHandle(), NULL, FALSE);
                        //Map has now been modified
                        SetUndoPoint();
                    }
                    break;

                case CM_EDITMAP:
                    nEditType = EDIT_WALLS;
                    pBitmaps = ae->bMaps;
                    if (nViewType != VIEW_WALLS)
                        {
                        nViewType = VIEW_WALLS;
                        InvalidateRect(hWnd,NULL,TRUE);
                        }
                    CurrentTopBitmap = CurrentBitmap = 1;
                    UpdateChildText(hWndChild);
                    SetScrollPos(hWndChild,SB_VERT,1,TRUE);
                    InvalidateRect(hWndChild,NULL,TRUE);
                    //MEH Update the menu
                    //ModifyMenu((HMENU)wParam,CM_EDITMAP,MF_STRING,CM_EDITMAP,"&Map");
                    //ModifyMenu((HMENU)wParam,CM_EDITOBJECTS,MF_STRING,CM_EDITOBJECTS,"&Objects\tTab");
                    break;

                case CM_EDITOBJECTS:
                    nEditType = EDIT_OBJECTS;
                    pBitmaps = ae->oMaps;
                    if (nViewType != VIEW_WALLS)
                        {
                        nViewType = VIEW_WALLS;
                        InvalidateRect(hWnd,NULL,TRUE);
                        }
                    CurrentTopBitmap = CurrentBitmap = 1;
                    UpdateChildText(hWndChild);
                    SetScrollPos(hWndChild,SB_VERT,1,TRUE);
                    InvalidateRect(hWndChild,NULL,TRUE);
                    //MEH Update the menu
                    //ModifyMenu((HMENU)wParam,CM_EDITMAP,MF_STRING,CM_EDITMAP,"&Map\tTab");
                    //ModifyMenu((HMENU)wParam,CM_EDITOBJECTS,MF_STRING,CM_EDITOBJECTS,"&Objects");
                    break;

                case CM_BITMAPLOAD:
                    bNoShow = TRUE;
                    *szName = '\0';
                    if (nEditType == EDIT_WALLS)
                        {
                        if (ae->bMaps[CurrentBitmap] != NULL)
                            {
                            if (MessageBox(hWnd,"Replace Current bitmap?","Warning",
                                MB_OKCANCEL) != IDOK)
                                {
                                break;
                                }
                            }
                        if (!ACKFileOpen(hWnd,szName,sizeof(szName),
                            "GIF Images (*.gif)\0*.gif\0PCX files (*.pcx)\0*.pcx\0BBM Bitmaps (*.bbm)\0*.bbm\0All Files (*.*)\0*.*\0",
                            "Open Bitmap File","bbm"))
                            {
                            if (strstr(szName,".pcx") || strstr(szName,".PCX"))
                                ae->bmLoadType = BMLOAD_PCX;
                            else if (strstr(szName,".gif") || strstr(szName,".GIF"))
                                ae->bmLoadType = BMLOAD_GIF;
                            //MEH Free bitmap data AFTER we know the user wants to
                            if(ae->bMaps[CurrentBitmap] != NULL)
                            {
                                AckFree(ae->bMaps[CurrentBitmap]);
                                InvalidateRect(hWnd, NULL, FALSE);
                            }
                            ae->bMaps[CurrentBitmap] = NULL;
                            //----
                            AckLoadBitmap(ae,CurrentBitmap,TYPE_WALL,szName);
                            ae->bmLoadType = BMLOAD_BBM;
                            AddFileName(TYPE_WALL,szName);
                            InvalidateRect(hWndChild,NULL,TRUE);
                            SetUndoPoint();
                            }
                        }
                    else
                        {
                        MessageBox(hWnd,"Use Object Attributes to\nload new bitmaps.",
                                    "Information",MB_OK);
                        }
                    bNoShow = FALSE;
                    break;

                case CM_BITMAPDELETE:
                    if(nEditType == EDIT_OBJECTS)
                        sprintf(szName, "Delete current object?");
                    else
                        sprintf(szName, "Delete current bitmap?");
                    if (MessageBox(hWnd,szName,"Warning",MB_OKCANCEL) == IDOK)
                    {
                        DeleteBitmap(MainAppWnd->GetHandle(),hWnd);
                        InvalidateRect(MainAppWnd->GetChildHandle(), NULL, FALSE);
                        SetUndoPoint();
                    }
                    break;

                case CM_VIEWWALLS:
                    if (nEditType != EDIT_WALLS)    //MEH Make sure we aren't editing objects while viewing floors or cielings!!!
                    {
                        nEditType = EDIT_WALLS;
                        InvalidateRect(hWndChild,NULL,TRUE);
                    }
                    nViewType = VIEW_WALLS;
//                    UpdateParentText(hWnd);
                    InvalidateRect(hWnd,NULL,TRUE);
                    break;

                case CM_VIEWFLOOR:
                    if (nEditType != EDIT_WALLS)    //MEH Make sure we aren't editing objects while viewing floors or cielings!!!
                    {
                        nEditType = EDIT_WALLS;
                        InvalidateRect(hWndChild,NULL,TRUE);
                    }
                    nViewType = VIEW_FLOOR;
//                    UpdateParentText(hWnd);
                    InvalidateRect(hWnd,NULL,TRUE);
                    break;

                case CM_VIEWCEILING:
                    if (nEditType != EDIT_WALLS)    //MEH Make sure we aren't editing objects while viewing floors or cielings!!!
                    {
                        nEditType = EDIT_WALLS;
                        InvalidateRect(hWndChild,NULL,TRUE);
                    }
                    nViewType = VIEW_CEILING;
//                    UpdateParentText(hWnd);
                    InvalidateRect(hWnd,NULL,TRUE);
                    break;

                case CM_GOODIESATTRIBUTES:
                    g_bCreatedObjectThisCycle = false;
                    if (nEditType == EDIT_OBJECTS)
                        {
                        if (ae->ObjList[CurrentBitmap] == NULL)
                            {
                            if (MessageBox(hWnd,"Create NEW object?","ACK Editor",
                                MB_YESNO) != IDYES)
                                {
                                break;
                                }
                            AckCreateObject(ae,CurrentBitmap);
                            AckSetObjectType(ae,CurrentBitmap,NO_CREATE);
                            g_bCreatedObjectThisCycle = true;
                            }
                        }
                    DoOptionsDlg(hInstGlobal,hWnd);
                    SetCurrentDirectory(szCurPath);
                    InvalidateRect(hWndChild,NULL,TRUE);
                    break;

                case CM_VIEWINF_FILE:
                    DoInfoDlg(hInstGlobal,hWnd);
                    break;

                case CM_VIEWTIMES_ONE:
                    StretchFactor = 1;
                    SetWindowPos(hwndApp,NULL,0,0,320,200,
                                 SWP_NOMOVE|SWP_NOZORDER|SWP_NOCOPYBITS);
                    break;

                case CM_VIEWTIMES_TWO:
                    StretchFactor = 2;
                    SetWindowPos(hwndApp,NULL,0,0,640,400,
                                 SWP_NOMOVE|SWP_NOZORDER|SWP_NOCOPYBITS);
                    break;


                case CM_FILLBORDER:
                    if (MessageBox(hWnd,"Fill border with current bitmap?","Warning",
                        MB_OKCANCEL) == IDOK)
                        {
                        FillBorder();
                        InvalidateRect(hWnd,NULL,TRUE);
                        }
                    break;

                case CM_FILLENTIRE_MAP:
                    if (MessageBox(hWnd,"Fill ENTIRE map with current bitmap?",
                        "Warning",MB_OKCANCEL) == IDOK)
                        {
                        FillEntireMap(CurrentBitmap);
                        InvalidateRect(hWnd,NULL,TRUE);
                        }
                    break;

                case CM_WALLS_NORMAL:
                    nWallFlags = 0;
                    break;

                case CM_WALLS_SPLIT:
                    nWallFlags &= ~DOOR_TYPE_SLIDE;
                    nWallFlags &= ~DOOR_TYPE_SECRET;
                    nWallFlags |= DOOR_TYPE_SPLIT;
                    break;

                case CM_WALLS_SLIDING:
                    nWallFlags &= ~DOOR_TYPE_SPLIT;
                    nWallFlags &= ~DOOR_TYPE_SECRET;
                    nWallFlags |= DOOR_TYPE_SLIDE;
                    break;

                case CM_WALLS_SECRET:
                    nWallFlags &= ~DOOR_TYPE_SPLIT;
                    nWallFlags &= ~DOOR_TYPE_SLIDE;
                    nWallFlags |= DOOR_TYPE_SECRET;
                    break;

                case CM_WALLS_TRANSPARENT:
                    nWallFlags ^= WALL_TYPE_TRANS;
                    break;

                case CM_WALLS_PASSABLE:
                    nWallFlags ^= WALL_TYPE_PASS;
                    break;

                case CM_GOODIESCLEAR_MAP:
                    if (MessageBox(hWnd,"Clear ENTIRE map?",
                        "Warning",MB_OKCANCEL) == IDOK)
                        {
                        FillEntireMap(0);
                        InvalidateRect(hWnd,NULL,TRUE);
                        }
                    break;


                case CM_GOODIESLOAD_BACKDROP:
                    *szName = '\0';
                    if (!ACKFileOpen(hWnd,szName,sizeof(szName),
                        "PCX files (*.pcx)\0*.pcx\0All Files (*.*)\0*.*\0",
                        "Open Backdrop File","pcx"))
                        {
                        //if (strstr(szName,".PCX"))
                        //    ae->bmLoadType = BMLOAD_PCX;
                        LoadNewBackDrop(szName);
                        //ae->bmLoadType = BMLOAD_BBM;
                        lstrcpy(BackGndFile,szName);
                        InvalidateRect(hwndApp,NULL,FALSE);
                        SetUndoPoint();
                        }
                    break;

                case CM_GOODIESLIGHT_SHADING:
                    if (ae->LightFlag == SHADING_OFF)
                        ae->LightFlag = SHADING_ON;
                    else
                        ae->LightFlag = SHADING_OFF;
                    AckSetupPalRanges(ae,ranges);
                    AckRegisterStructure(ae);
                    SetUndoPoint();
                    break;


                /*case CM_GOODIESSOLID_FLOOR:
                  ae->SysFlags ^= SYS_SOLID_FLOOR;
                  AckRegisterStructure(ae);
                  break;*/

                /*case CM_GOODIESSOLID_CEILING:
                  ae->SysFlags ^= SYS_SOLID_CEIL;
                  AckRegisterStructure(ae);
                  break;*/

                /*case CM_GOODIESSINGLE_BITMAP:
                    if (ae->SysFlags & SYS_SINGLE_BMP)
                        ae->SysFlags &= ~SYS_SINGLE_BMP;
                    else
                        {
                        ae->SysFlags |= SYS_SINGLE_BMP;
                        ae->SysFlags &= ~SYS_SOLID_FLOOR;
                        ae->SysFlags &= ~SYS_SOLID_CEIL;
                        }
                    AckRegisterStructure(ae);
                    break;*/

                case CM_GOODIESTRACK_PLAYER:
                    bTrackPlayer ^= 1;
                    InvalidateRect(hWnd,NULL,TRUE);
                    break;

                case CM_GOODIESSHOW_COORDINATES:
                    bShowCoords ^= 1;
                    InvalidateRect(hWnd,NULL,TRUE);
                    break;

                /*case CM_ASSIGN_CEILING:
                    ae->CeilBitmap = CurrentBitmap;
                    AckRegisterStructure(ae);
                    break;*/

                /*case CM_ASSIGN_FLOOR:
                    ae->FloorBitmap = CurrentBitmap;
                    AckRegisterStructure(ae);
                    break;*/


                default:
                    break;
                }
            break;


        default:
            return DefWindowProc( hWnd, iMessage, wParam, lParam );
        }
return 0;
}


// If data pointers are near pointers
#if defined(__SMALL__) || defined(__MEDIUM__)
inline Window *GetPointer(HWND hWnd)
{
return (Window *)GetWindowWord(hWnd,0);
}

inline void SetPointer(HWND hWnd,Window *pWindow)
{
SetWindowWord(hWnd,0,(WORD)pWindow);
}

// else pointers are far
#else//if defined(__LARGE__) || defined(__COMPACT__) || defined(__FLAT__)
inline Window *GetPointer( HWND hWnd )
{
return (Window *)GetWindowLongPtr(hWnd,0);
}

inline void SetPointer( HWND hWnd, Window *pWindow )
{
SetWindowLongPtr(hWnd,0,(LONG) pWindow);
}

//#else
//    #error Choose another memory model!
#endif

//****************************************************************************
//
//****************************************************************************
LRESULT  CALLBACK _export WndProc(HWND hWnd,UINT iMessage,WPARAM wParam,
                                 LPARAM lParam)
{
    LPCREATESTRUCT lpcs;
    Window         *pWindow = GetPointer(hWnd);

if (pWindow == 0)
    {
    if ( iMessage == WM_CREATE )
        {
        lpcs = (LPCREATESTRUCT) lParam;
        pWindow = (Window *) lpcs->lpCreateParams;
        SetPointer(hWnd,pWindow);
        return pWindow->WndProc(iMessage,wParam,lParam);
        }
    else
        return DefWindowProc(hWnd,iMessage,wParam,lParam);
    }
else
    return pWindow->WndProc(iMessage,wParam,lParam);
}



//****************************************************************************
//
//****************************************************************************
LRESULT  CALLBACK _export AckWndProc( HWND hWnd, UINT iMessage, WPARAM wParam,
                                 LPARAM lParam )

{
//        HDC         hDC;
        PAINTSTRUCT ps;

switch (iMessage)
    {
    case WM_CREATE:
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        AckDoSize(hWnd,lParam);
        break;

    case WM_PAINT:
        BeginPaint(hWnd,&ps);
        AckPaint(hWnd,ps.hdc);
        EndPaint(hWnd,&ps);
        break;

    case WM_SETCURSOR:
        SetOperationCursor();
        return(TRUE);

    case WM_KEYDOWN:
        ProcessKeys(iMessage,wParam);
        HandleGenericKeypresses(wParam);
        break;

    case WM_KEYUP:
        ProcessKeys(iMessage,wParam);
        InvalidateRect(hWndMap,NULL,FALSE);
        break;

    case WM_RBUTTONDOWN:
        AckCheckDoorOpen(ae->xPlayer,ae->yPlayer,ae->PlayerAngle);
        break;

    case WM_LBUTTONDOWN:
        ProcessMouse(hWnd,1,LOWORD(lParam),HIWORD(lParam));
        SetCapture(hWnd);
        break;

    case WM_MOUSEMOVE:
        if (wParam & MK_LBUTTON)
            ProcessMouse(hWnd,1,LOWORD(lParam),HIWORD(lParam));
        break;

    case WM_LBUTTONUP:
        ProcessMouse(hWnd,0,LOWORD(lParam),HIWORD(lParam));
        ReleaseCapture();
        InvalidateRect(hWndMap,NULL,FALSE);
        break;

    //MEH Allow mousewheel to scroll bitmap/object list, even with the ACK-3D view window active
    case WM_MOUSEWHEEL:
        if((short)(HIWORD(wParam)) > 0)  //Rotated wheel up
        {
            ScrollBitmaps(MainAppWnd->GetChildHandle(), SB_LINEUP, 0);
        }
        else    //Rotated wheel down
        {
            ScrollBitmaps(MainAppWnd->GetChildHandle(), SB_LINEDOWN, 0);
        }
        break;

    default:
        return DefWindowProc(hWnd,iMessage,wParam,lParam);
    }

return 0;
}

//****************************************************************************
//
//****************************************************************************
void SelectBitmap(HWND hWnd,short yPos)
{
    HDC     hdc;
    short   oldCurrent = CurrentBitmap;

    g_bCreatedObjectThisCycle = false;

    CurrentBitmap = (short)(CurrentTopBitmap + (yPos / 66));
    if (CurrentBitmap != oldCurrent)
    {
        hdc = GetDC(hWnd);
        HighlightBitmap(hdc,RGB(255,255,255));
        ReleaseDC(hWnd,hdc);
        InvalidateRect(hWnd,NULL,FALSE);
        UpdateChildText(hWnd);
    }
    else    //MEH If we're clicking the same bitmap again, open wall options dialog
    {
        if (nEditType == EDIT_OBJECTS)
        {
            //But, if we're control-clicking it, remove it from the map wherever it is currently
            if(GetAsyncKeyState(VK_CONTROL) < 0)
            {
                if(ae->ObjList[CurrentBitmap] != NULL)
                {
                    for (int mPos = 0; mPos < 4096; mPos++)
                    {
                        if (CurrentBitmap == (ObjGrid[mPos] & 0xFF))
                            ObjGrid[mPos] = 0;
                    }
                    ae->ObjList[CurrentBitmap]->Active = 0; //set to inactive
                    InvalidateRect(MainAppWnd->GetChildHandle(),NULL,FALSE);
                    InvalidateRect(MainAppWnd->GetHandle(),NULL,FALSE); //Redraw windows
                    SetUndoPoint();
                    return;
                }
            }
            if (ae->ObjList[CurrentBitmap] == NULL)
            {
                if (MessageBox(MainAppWnd->GetHandle(),"Create NEW object?","ACK Editor", MB_YESNO) != IDYES)
                {
                    return;
                }
                AckCreateObject(ae,CurrentBitmap);
                AckSetObjectType(ae,CurrentBitmap,NO_CREATE);
                g_bCreatedObjectThisCycle = true;
            }
        }
        if (nEditType == EDIT_WALLS && ae->bMaps[CurrentBitmap] == NULL)    //If this bitmap is blank, load it by clicking
        {
            bNoShow = TRUE;
            char            szName[128];
            szName[0] = '\0';

            if (!ACKFileOpen(hWnd,szName,sizeof(szName),
                "GIF Images (*.gif)\0*.gif\0PCX files (*.pcx)\0*.pcx\0BBM Bitmaps (*.bbm)\0*.bbm\0All Files (*.*)\0*.*\0",
                "Open Bitmap File","bbm"))
            {
                if (strstr(szName,".pcx") || strstr(szName,".PCX"))
                    ae->bmLoadType = BMLOAD_PCX;
                else if (strstr(szName,".gif") || strstr(szName,".GIF"))
                    ae->bmLoadType = BMLOAD_GIF;
                //MEH Free bitmap data AFTER we know the user wants to
                if(ae->bMaps[CurrentBitmap] != NULL)
                    AckFree(ae->bMaps[CurrentBitmap]);
                ae->bMaps[CurrentBitmap] = NULL;
                //----
                AckLoadBitmap(ae,CurrentBitmap,TYPE_WALL,szName);
                ae->bmLoadType = BMLOAD_BBM;
                AddFileName(TYPE_WALL,szName);
                InvalidateRect(MainAppWnd->GetChildHandle(),NULL,TRUE);
            }
            bNoShow = FALSE;
            return; //stop here, so don't open dialog
        }
        DoOptionsDlg(hInstGlobal,MainAppWnd->GetHandle());
        SetCurrentDirectory(szCurPath);
        InvalidateRect(MainAppWnd->GetChildHandle(),NULL,TRUE);
    }
}


//****************************************************************************
//
//****************************************************************************
void ScrollBitmaps(HWND hWnd,int nScrollCode,int nPos)
{
    HDC     hdc;
    int     pos = GetScrollPos(hWnd,SB_VERT);

switch (nScrollCode)
    {
    case SB_LINEDOWN:
        if (pos < 252)
            pos++;
        break;

    case SB_LINEUP:
        if (pos > 1)
            pos--;
        break;

    case SB_PAGEDOWN:
        pos += 4;
        if (pos > 252)
            pos = 252;
        break;

    case SB_PAGEUP:
        pos -= 4;
        if (pos < 1)
            pos = 1;
        break;

    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        pos = nPos;
        if (pos < 1)
            pos = 1;
        break;

    default:
        break;
    }

//MEH Object list isn't as large for some reason. Whatever.
if(nEditType == EDIT_OBJECTS && pos > 250)
    pos = 250;

//Test to make sure we didn't go too far up
if(pos <= 0)
    pos = 1;

CurrentTopBitmap = (short)pos;

SetScrollPos(hWnd,SB_VERT,pos,TRUE);

hdc = GetDC(hWnd);
HighlightBitmap(hdc,RGB(255,255,255));
ReleaseDC(hWnd,hdc);
InvalidateRect(hWnd,NULL,FALSE);
}


//****************************************************************************
//
//****************************************************************************
LRESULT  CALLBACK _export ChildWndProc( HWND hWnd, UINT iMessage, WPARAM wParam,
                                 LPARAM lParam )
{
//    HDC     hDC;
    int     tempCurrent;

switch (iMessage)
    {
    case WM_CREATE:
        SetScrollRange(hWnd,SB_VERT,0,252,TRUE);
        break;

    case WM_PAINT:
        PaintChild(hWnd);
        break;

    case WM_SETCURSOR:
        SetOperationCursor();
        return(TRUE);

    case WM_LBUTTONDOWN:
        SelectBitmap(hWnd,HIWORD(lParam));
        break;

    //MEH Delete bitmaps by right-clicking them
    case WM_RBUTTONUP:
        tempCurrent = (short)(CurrentTopBitmap + (HIWORD(lParam) / 66));
        //If this is our currently selected bitmap/object, delete it
        if(tempCurrent == CurrentBitmap)
        {
            char cTempText[64];
            if(nEditType == EDIT_OBJECTS)
                sprintf(cTempText, "Delete current object?");
            else
                sprintf(cTempText, "Delete current bitmap?");
            if (MessageBox(hWnd,cTempText,"Warning",MB_OKCANCEL) == IDOK)
            {
                DeleteBitmap(MainAppWnd->GetHandle(),hWnd);
                SetUndoPoint();
            }
        }
        //Otherwise, select it
        else
        {
            CurrentBitmap = tempCurrent;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_MBUTTONDOWN:    //MEH: MMB removes the object from wherever it is on the map, when you click on it in the bitmap window
        tempCurrent = (short)(CurrentTopBitmap + (HIWORD(lParam) / 66));
        //But only if this is our currently selected bitmap/object
        if(tempCurrent == CurrentBitmap)
        {
            if(ae->ObjList[CurrentBitmap] != NULL)
            {
                for (int mPos = 0; mPos < 4096; mPos++)
                {
                    if (CurrentBitmap == (ObjGrid[mPos] & 0xFF))
                        ObjGrid[mPos] = 0;
                }
                ae->ObjList[CurrentBitmap]->Active = 0; //set to inactive
                InvalidateRect(MainAppWnd->GetChildHandle(),NULL,FALSE);
                InvalidateRect(MainAppWnd->GetHandle(),NULL,FALSE); //Redraw windows
                SetUndoPoint();
            }
        }
        else    //Otherwise, set this to current bitmap
        {
            CurrentBitmap = tempCurrent;
            InvalidateRect(MainAppWnd->GetChildHandle(),NULL,FALSE);    //Redraw bitmap window
        }
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

    //MEH Allow mousewheel to scroll bitmap/object list
    case WM_MOUSEWHEEL:
        if((short)(HIWORD(wParam)) > 0)  //Rotated wheel up
        {
            ScrollBitmaps(hWnd, SB_LINEUP, 0);
        }
        else    //Rotated wheel down
        {
            ScrollBitmaps(hWnd, SB_LINEDOWN, 0);
        }
        break;

    case WM_VSCROLL:
        ScrollBitmaps(hWnd,LOWORD(wParam),HIWORD(wParam));
        break;

    case WM_KEYUP:
        ProcessKeys(iMessage,wParam);
        InvalidateRect(hWndMap,NULL,FALSE);
        break;

    case WM_KEYDOWN:
        ProcessKeys(iMessage,wParam);
        HandleGenericKeypresses(wParam);
        break;

    default:
        return DefWindowProc(hWnd,iMessage,wParam,lParam);
    }
return 0;
}

//****************************************************************************
//
//****************************************************************************
int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine,
                    int nCmdShow)
{
    short   i;
    int     result;
    HBITMAP hbm;
    char szName[256];

//MEH Load our cursors
g_cArrow = LoadCursor(NULL, IDC_ARROW);
g_cPan = LoadCursor(NULL, IDC_SIZEALL);

nOperation = OP_NORMAL;
hInstGlobal = hInstance;
Main::hInstance = hInstance;
Main::hPrevInstance = hPrevInstance;
Main::nCmdShow = nCmdShow;

MainWindow MainWnd;
MainAppWnd = &MainWnd;

if ( ! Main::hPrevInstance )
    MainWnd.Register();

if (InitACKEngine())
    return(-1);

//MEH Read in our pallette also
ReadInPallette();

GetCurrentDirectory(sizeof(szCurPath),szCurPath);
strcpy(ResFileName,"pics.dtf");
AckOpenResource(ResFileName);
result = ProcessInfoFile(1);

if (result)
    {
    wsprintf(LineBuffer,"Error: %d reading resource\nfile %s",result,ResFileName);
    MessageBox(NULL,LineBuffer,"Error",MB_OK|MB_ICONSTOP);
    AckCloseResource();
    return(-2);
    }

LoadBackDrop();
AckCloseResource();

nViewType = VIEW_WALLS;
nEditType = EDIT_WALLS;
nWallFlags = 0;
nObjFlags = 0;
nBackColor = 9;
pBitmaps = ae->bMaps;
BackGndFile[0] = '\0';
nMoveSpeed = 0;
nTurnSpeed = 0;
bTrackPlayer = TRUE;
bModified = FALSE;
bShowCoords = FALSE;
nBitmapW = 319;         //default size (window client size to use).
nBitmapH = 199;
ReadConfig();


for (i = 0; i < MAX_OBJBMPS; i++)
    {
    oMapFiles[i] = NULL;
    }

for (i = 0; i < MAX_WALLBMPS; i++)
    {
    bMapFiles[i] = NULL;
    }

AckSetupPalRanges(ae,ranges);
MainWnd.CreateAll();
result = Main::MessageLoop();

if (Buffer)
    {
    hbm = (HBITMAP)SelectObject(Buffer,gbmOldMonoBitmap);
    DeleteObject(hbm);
    DeleteDC(Buffer);
    }

if(hpalApp)
    {
    DeleteObject(hpalApp);
    }

//WinG.Free();
WriteConfig();

//MEH Free cursors we created
DestroyCursor(g_cArrow);
DestroyCursor(g_cPan);

//MEH Destroy all undo/redo locations
if(g_sNumRedo > 0)
{
    for(;g_sNumRedo > 0;)
    {
        sprintf(szName, "pics r%d.dtf", g_sNumRedo);
        unlink(szName);
        g_sNumRedo--;
    }
    unlink("pics r0.dtf");
}
if(g_sNumUndo > 0)
{
    for(;g_sNumUndo > 0;)
    {
        sprintf(szName, "pics u%d.dtf", g_sNumUndo);
        unlink(szName);
        g_sNumUndo--;
    }
    unlink("pics u0.dtf");
}

return(result);
}


//MEH Read in the pallette from pallette.txt
void ReadInPallette()
{
    std::ifstream infile("pallette.txt");
    if(infile.fail())
        return;
//printf("reading in pallette\n");
    for(int i = 0; i < 256; i++)
    {
        unsigned int r, g, b;
        PalletteSlot p = {0, 0, 0};
        infile >> r >> g >> b;
        p.r = (unsigned char)b; //Flip blue and red, because Windows wants the icon data in BGRA format
        p.g = (unsigned char)g;
        p.b = (unsigned char)r;
        p.a = 255;
        //printf("%d, %d, %d\n", p.r, p.g, p.b);
        pallette[i] = p;
    }
    infile.close();
}


//MEH Handle key presses no matter what window they're in, if they're a keypress we want to respond
// to no matter the window. That kinda explains itself, doesn't it?
void HandleGenericKeypresses(WPARAM wParam)
{
    //char            szName[128];
    //short           mx;

    switch(wParam)
    {
        //MEH Pressing tab will alternate object editing and wall editing
        case VK_TAB:
            if(nEditType == EDIT_OBJECTS)
            {
                nEditType = EDIT_WALLS;
            }
            else
            {
                if(nViewType == VIEW_FLOOR || nViewType == VIEW_CEILING)
                {
                    nViewType = VIEW_WALLS;
                    InvalidateRect(MainAppWnd->GetHandle(), NULL, TRUE);
                }
                nEditType = EDIT_OBJECTS;
            }
            //Of course, redraw and set the child (bitmap) window text to be correct
            InvalidateRect(MainAppWnd->GetChildHandle(), NULL, TRUE);
            UpdateChildText(MainAppWnd->GetChildHandle());
//            UpdateParentText(MainAppWnd->GetHandle());
            break;

        //Pagedown/pageup causes background color to change
        case VK_PRIOR:
            nBackColor--;
            break;

        case VK_NEXT:
            nBackColor++;
            break;

        //MEH Pressing 1, 2, 3 will change view type
        case VK_1:
            //If they're pressing Control+1 or Control+2, though, change view size
            if(GetAsyncKeyState(VK_CONTROL) < 0)
            {
                StretchFactor = 1;
                SetWindowPos(hwndApp,NULL,0,0,320,200,SWP_NOMOVE|SWP_NOZORDER|SWP_NOCOPYBITS);
                break;
            }
            if(nViewType != VIEW_WALLS || nEditType == EDIT_OBJECTS)
            {
                //Redraw the bitmap window if we're toggling from objects
                if(nEditType == EDIT_OBJECTS)
                {
                    nEditType = EDIT_WALLS;
                    InvalidateRect(MainAppWnd->GetChildHandle(), NULL, TRUE);
                    UpdateChildText(MainAppWnd->GetChildHandle());
                }
                nViewType = VIEW_WALLS;
                InvalidateRect(MainAppWnd->GetHandle(), NULL, TRUE);
//                UpdateParentText(MainAppWnd->GetHandle());
            }
            break;

        case VK_2:
            //Change to view size = 2 if they're pressing Ctrl+2
            if(GetAsyncKeyState(VK_CONTROL) < 0)
            {
                StretchFactor = 2;
                SetWindowPos(hwndApp,NULL,0,0,640,400,SWP_NOMOVE|SWP_NOZORDER|SWP_NOCOPYBITS);
                break;
            }
            if(nViewType != VIEW_FLOOR)
            {
                //Redraw the bitmap window if we're toggling from objects
                if(nEditType == EDIT_OBJECTS)
                {
                    nEditType = EDIT_WALLS;
                    InvalidateRect(MainAppWnd->GetChildHandle(), NULL, TRUE);
                    UpdateChildText(MainAppWnd->GetChildHandle());
                }
                nViewType = VIEW_FLOOR;
                InvalidateRect(MainAppWnd->GetHandle(), NULL, TRUE);
//                UpdateParentText(MainAppWnd->GetHandle());
            }
            break;

        case VK_3:
            if(nViewType != VIEW_CEILING)
            {
                //Redraw the bitmap window if we're toggling from objects
                if(nEditType == EDIT_OBJECTS)
                {
                    nEditType = EDIT_WALLS;
                    InvalidateRect(MainAppWnd->GetChildHandle(), NULL, TRUE);
                    UpdateChildText(MainAppWnd->GetChildHandle());
                }
                nViewType = VIEW_CEILING;
                InvalidateRect(MainAppWnd->GetHandle(), NULL, TRUE);
//                UpdateParentText(MainAppWnd->GetHandle());
            }
            break;

        //Fill in border of map with Ctrl+B
        case VK_B:
            if(GetAsyncKeyState(VK_CONTROL) < 0 && MessageBox(MainAppWnd->GetHandle(),"Fill border with current bitmap?","Warning",MB_OKCANCEL) == IDOK)
            {
                FillBorder();
                InvalidateRect(MainAppWnd->GetHandle(),NULL,TRUE);
            }
            break;

        //MEH Copy item to clipboard with Ctrl+C
        case VK_C:
            if(GetAsyncKeyState(VK_CONTROL) < 0 && nEditType == EDIT_OBJECTS)
                g_sClipboard = CurrentBitmap;
            break;

        //Fill in whole map with Ctrl+M
        case VK_M:
            if(GetAsyncKeyState(VK_CONTROL) < 0 && MessageBox(MainAppWnd->GetHandle(),"Fill ENTIRE map with current bitmap?","Warning",MB_OKCANCEL) == IDOK)
            {
                FillEntireMap(CurrentBitmap);
                InvalidateRect(MainAppWnd->GetHandle(),NULL,TRUE);
            }
            break;

        //Create new map with Ctrl+N
        case VK_N:
            if(GetAsyncKeyState(VK_CONTROL) < 0)
                CreateNewMap(MainAppWnd->GetHandle(), MainAppWnd->GetChildHandle());
            break;

        case VK_O:
            //If pressing Ctrl+O, open file
            if(GetAsyncKeyState(VK_CONTROL) < 0)
            {
                //Currently broken

                /*if (!ACKFileOpen(MainAppWnd->GetHandle(),ResFileName,sizeof(ResFileName),
                        "Resource Files (*.dtf)\0*.dtf\0All Files (*.*)\0*.*\0",
                        "Open Resource File","dtf"))
                {
                if (!AckOpenResource(ResFileName))
                    {
                    bNoShow = TRUE;
                    AckReset();
                    mx = (short)ProcessInfoFile(0);
                    xCurrent = yCurrent = 0;
                    CurrentTopBitmap = CurrentBitmap = 1;
                    if (mx)
                        {
                        wsprintf(szName,"Error: %d in DTF file at line %d",mx,LineNumber);
                        MessageBox(MainAppWnd->GetHandle(),szName,"ERROR",MB_OK);
                        }
                    else
                        {
                        LoadBackDrop();
                        AckRegisterStructure(ae);
                        bNoShow = FALSE;
                        }
                    RemapBufferPalettes();
                    InvalidateRect(MainAppWnd->GetHandle(),NULL,TRUE);
                    InvalidateRect(MainAppWnd->GetChildHandle(),NULL,TRUE);
                    }
                }*/
            }
            break;

            case VK_S:
                //If pressing Ctrl+Shift+S, save as
                if(GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_SHIFT) < 0)
                {
                    //Currently broken

                    /*strcpy(szName,ResFileName);
                    if (!ACKFileSave(MainAppWnd->GetHandle(),szName,sizeof(szName),
                                     "Resource Files (*.dtf)\0*.dtf\0All Files (*.*)\0*.*\0",
                                     "Save Resource File"))
                    {
                        SaveResource(MainAppWnd->GetHandle(),szName);
                        strcpy(ResFileName,szName);
                        GetCurrentDirectory(sizeof(szCurPath),szCurPath);
                    }*/
                }
                //Otherwise, Ctrl+S = save
                else if (GetAsyncKeyState(VK_CONTROL) < 0)
                {
                    g_sSaveUndo = g_sNumUndo;
                    bModified = FALSE;
                }
                break;

            //MEH Paste item from clipboard with Ctrl+V
            case VK_V:
                if(GetAsyncKeyState(VK_CONTROL) < 0 && CurrentBitmap != g_sClipboard && g_sClipboard)
                {
                    if (ae->ObjList[CurrentBitmap] == NULL)
                    {
                        AckCreateObject(ae,CurrentBitmap);
                        AckSetObjectType(ae,CurrentBitmap,NO_CREATE);
                    }
                    //Hold onto data we want to not copy (Namely position and active/not)
                    short mPos = ae->ObjList[CurrentBitmap]->mPos;
                    short x = ae->ObjList[CurrentBitmap]->x;
                    short y = ae->ObjList[CurrentBitmap]->y;
                    char active = ae->ObjList[CurrentBitmap]->Active;
                    //Copy other object onto this one
                    CopyObject(ae->ObjList[g_sClipboard], ae->ObjList[CurrentBitmap]);
                    //Reset data we didn't want copied
                    ae->ObjList[CurrentBitmap]->Active = active;
                    ae->ObjList[CurrentBitmap]->mPos = mPos;
                    ae->ObjList[CurrentBitmap]->x = x;
                    ae->ObjList[CurrentBitmap]->y = y;
                    //Redraw the bitmap window
                    InvalidateRect(MainAppWnd->GetChildHandle(), NULL, FALSE);
                    //Map has now been modified
                    SetUndoPoint();
                }
                break;

            //Clear the map if they press Ctrl+X
            case VK_X:
                //OH LOOK I'M ACTUALLY USING C++'S METHOD OF PARSING STATEMENTS SEQUENTIALLY
                if (GetAsyncKeyState(VK_CONTROL) < 0 && MessageBox(MainAppWnd->GetHandle(),"Clear ENTIRE map?", "Warning",MB_OKCANCEL) == IDOK)
                {
                    FillEntireMap(0);
                    InvalidateRect(MainAppWnd->GetHandle(),NULL,TRUE);
                }
                break;

            //MEH: If pressing Ctrl+Y, redo
            case VK_Y:
                if (GetAsyncKeyState(VK_CONTROL) < 0 && g_sNumRedo)
                    Redo();
                break;

            //MEH: If pressing CTRL+Z, undo
            case VK_Z:
                if (GetAsyncKeyState(VK_CONTROL) < 0 && g_sNumUndo)
                    Undo();
                break;
    }
}

void ShowMapPosn(HWND hWndChild,short xPos,short yPos)
{
    int     row,col,pos;
    int     gsLeft, gsRight, mPos;
    bool    bLR = false;
    bool    bEdge = false;

    row = xPos / GridSize;
    col = yPos / GridSize;
    row += yCurrent;
    col += xCurrent;
    pos = (row * GRID_WIDTH) + col;

    switch (nViewType)
    {
    case VIEW_WALLS:
        if (nEditType != EDIT_WALLS)
        {
            break;
        }

        xPos = (short)(xPos % GridSize);
        yPos = (short)(yPos % GridSize);
        gsLeft = SQUARE_EDGE_SIZE;
        gsRight = (short)(GridSize - SQUARE_EDGE_SIZE / 2);
        if (yPos > gsLeft && yPos < gsRight)
            {
            if (xPos < gsLeft)
                {
                mPos = pos;
                bLR = true;
                bEdge = true;
                }
            else
                {
                if (xPos >= gsRight)
                    {
                    mPos = pos + 1;
                    bLR = true;
                    bEdge = true;
                    }
                }
            }

        if (xPos > gsLeft && xPos < gsRight)
            {
            if (yPos < gsLeft)
                {
                mPos = pos;
                bEdge = true;
                }
            else
                {
                if (yPos >= gsRight)
                    {
                    mPos = pos + GRID_WIDTH;
                    bEdge = true;
                    }
                }
            }
        break;
    }

    char ctext[128];
    if(bEdge)
    {
        if(bLR)
            sprintf(ctext, "Map Posn: %d row: %d col: %d\nX grid pos: %d", pos, row, col, mPos);
        else
            sprintf(ctext, "Map Posn: %d row: %d col: %d\nY grid pos: %d", pos, row, col, mPos);
    }
    else
        sprintf(ctext, "Map Posn: %d row: %d col: %d", pos, row, col);
    MessageBox(hWndChild, ctext, "ACK Editor", MB_OK);
}

//MEH Clear out this map, ready for a new one
void CreateNewMap(HWND hWnd, HWND hWndChild)
{
    if(MessageBox(hWnd,"Create new map?", "Ack Editor", MB_YESNO) == IDYES)
    {
        if(bModified)
        {
            int iResult = MessageBox(hWnd,"Current map has been modified. Save changes?", "New Map", MB_YESNOCANCEL);
            if(iResult == IDYES)
            {
                SetCurrentDirectory(szCurPath);
                SaveResource(hWnd,ResFileName);
                bModified = FALSE;
            }
            else if(iResult == IDCANCEL)
            {
                return;
            }
            g_sSaveUndo = g_sNumUndo;
        }
        g_bNoCreateUndoRedo = true;
        //-------------------Now clear everything out
        //First delete all the bitmaps, except the first
        nEditType = EDIT_WALLS;
        for(int i = 2; i < MAX_WALLBMPS; i++)
        {
            CurrentBitmap = i;
            DeleteBitmap(hWnd,hWndChild);
        }

        //Delete all objects
        nEditType = EDIT_OBJECTS;
        for(int i = 0; i < MAX_OBJECTS+1; i++)
        {
            CurrentBitmap = i;
            DeleteBitmap(hWnd,hWndChild);
        }

        //Delete all the object supply bitmaps
        for(int i = 0; i < MAX_OBJBMPS; i++)
        {
            if(ae->oMaps[i] == NULL)
                continue;
            AckFree(ae->oMaps[i]);
            ae->oMaps[i] = NULL;
            RemoveFileName(TYPE_OBJECT,i);
        }

        //Fill in the border with bitmap 1
        nEditType = EDIT_WALLS;
        nViewType = VIEW_WALLS;
        CurrentBitmap = 1;
        FillEntireMap(0);   //After clearing the map, since walls with bitmap 1 weren't deleted before as the other walls were
        FillBorder();

        //Clear the ceiling
        nViewType = VIEW_CEILING;
        FillEntireMap(0);

        //Fill the whole floor with bitmap 1
        nViewType = VIEW_FLOOR;
        FillEntireMap(1);

        //Set variables to good initial defaults
        CurrentBitmap = 1;
        CurrentTopBitmap = 1;
        nEditType = EDIT_WALLS;
        nViewType = VIEW_WALLS;

        g_bNoCreateUndoRedo = false;
        SetUndoPoint(); //Save this all out
    }
}


//MEH Functions for dealing with Undo/redo

//Copy one file to another - ripped from teh internets
bool CopyFile(const char* src, const char* dest)
{
    FILE *in, *out;
    char ch;

    if((in = fopen(src, "rb")) == NULL)
    {
        //printf("Cannot open input file.\n");
        return false;
    }
    if((out = fopen(dest, "wb")) == NULL)
    {
        printf("Cannot open output file.\n");
        return false;
    }

    while(!feof(in))
    {
        ch = getc(in);
        if(ferror(in))
        {
            printf("Read Error\n");
            clearerr(in);
            break;
        }
        else
        {
            if(!feof(in))
                putc(ch, out);
            if(ferror(out))
            {
                printf("Write Error\n");
                clearerr(out);
                break;
            }
        }
    }
    fclose(in);
    fclose(out);
    return true;
}

bool bLastUndoMoved = false;
//-----------------------------------------------------------------------
// Adds this point to the undo stack
//-----------------------------------------------------------------------
void SetUndoPoint(bool bMovement)
{
    if(g_bNoCreateUndoRedo)
        return;

    if(bLastUndoMoved && bMovement)
    {
        g_sNumUndo--;
    }

    bLastUndoMoved = bMovement;

    char szName[128];
    char PathBuffer[256];
    GetCurrentDirectory(256, PathBuffer);
    SetCurrentDirectory(szCurPath);

    bModified = TRUE;

    sprintf(szName, "pics u%d.dtf", g_sNumUndo);
    g_sNumUndo++;
    rename("pics.dtf",szName);
    CopyFile(szName, "pics.dtf");   //So SaveResource() doesn't totally mess up
    SaveResource(MainAppWnd->GetHandle(), "pics.dtf");

    SetCurrentDirectory(PathBuffer);

    if(g_sNumRedo > 0)
    {
        for(;g_sNumRedo > 0;)
        {
            sprintf(szName, "pics r%d.dtf", g_sNumRedo);
            unlink(szName);
            g_sNumRedo--;
        }
        unlink("pics r0.dtf");

        //Also make sure we don't get into a state where the editor thinks the map is saved, but it isn't.
        if(g_sSaveUndo > g_sNumUndo)
            g_sSaveUndo = -1;
    }
}

//-----------------------------------------------------------------------
// Undoes a step from the top of the undo stack
//-----------------------------------------------------------------------
void Undo(void)
{
    bLastUndoMoved = false;
    char LineBuffer[256];
    char cFilename[256];
    int result;
    char PathBuffer[256];
    GetCurrentDirectory(256, PathBuffer);
    SetCurrentDirectory(szCurPath);

    sprintf(cFilename, "pics r%d.dtf", g_sNumRedo);
    g_sNumRedo++;
    CopyFile("pics.dtf",cFilename);
    sprintf(cFilename, "pics u%d.dtf", g_sNumUndo - 1);
    g_sNumUndo--;
    CopyFile(cFilename, "pics.dtf");
    unlink(cFilename);
    sprintf(cFilename, "pics.dtf");
    AckOpenResource(cFilename);

    //Clear everything
    AckReset();

    result = ProcessInfoFile(1);
    if (result)
    {
        wsprintf(LineBuffer,"Error: %d reading resource file %s",result,cFilename);
        MessageBox(NULL,LineBuffer,"Error",MB_OK|MB_ICONSTOP);
        AckCloseResource();
        return;
    }

    LoadBackDrop();
    AckCloseResource();
    AckSetupPalRanges(ae,ranges);
    AckRegisterStructure(ae);

    SetCurrentDirectory(PathBuffer);

    //If we've undone everything, map isn't modified any more
    if(g_sNumUndo == g_sSaveUndo)
        bModified = FALSE;
    else
        bModified = TRUE;

    //Also redraw
    InvalidateRect(MainAppWnd->GetHandle(), NULL, FALSE);
    InvalidateRect(MainAppWnd->GetChildHandle(), NULL, FALSE);
}

//-----------------------------------------------------------------------
// Redoes an undo
//-----------------------------------------------------------------------
void Redo(void)
{
    bLastUndoMoved = false;
    char LineBuffer[256];
    char cFilename[256];
    int result;
    char PathBuffer[256];
    GetCurrentDirectory(256, PathBuffer);
    SetCurrentDirectory(szCurPath);

    sprintf(cFilename, "pics u%d.dtf", g_sNumUndo);
    g_sNumUndo++;
    CopyFile("pics.dtf",cFilename);
    sprintf(cFilename, "pics r%d.dtf", g_sNumRedo - 1);
    g_sNumRedo--;
    CopyFile(cFilename, "pics.dtf");
    unlink(cFilename);
    sprintf(cFilename, "pics.dtf");
    AckOpenResource(cFilename);

    //Clear everything
    AckReset();

    result = ProcessInfoFile(1);
    if (result)
    {
        wsprintf(LineBuffer,"Error: %d reading resource file %s",result,cFilename);
        MessageBox(NULL,LineBuffer,"Error",MB_OK|MB_ICONSTOP);
        AckCloseResource();
        return;
    }

    LoadBackDrop();
    AckCloseResource();
    AckSetupPalRanges(ae,ranges);
    AckRegisterStructure(ae);

    SetCurrentDirectory(PathBuffer);

    //If we've undone everything, map isn't modified any more
    if(g_sNumUndo == g_sSaveUndo)
        bModified = FALSE;
    else
        bModified = TRUE;

    //Also redraw
    InvalidateRect(MainAppWnd->GetHandle(), NULL, FALSE);
    InvalidateRect(MainAppWnd->GetChildHandle(), NULL, FALSE);
}




