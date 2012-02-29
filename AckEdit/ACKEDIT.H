// Header file for ACKEDIT program

//from winpal.cpp (really a hacked PALANIM sample.c)
extern void ClearSystemPalette(void);
extern HPALETTE CreateIdentityPalette(RGBQUAD aRGB[], int nColors);
extern void AppActivate(BOOL fActive);

void CreateAckPalette(void);				//at end of this module.
void DoCycle(void);
int LoadBackDrop(void);
int LoadNewBackDrop(char *szName);
int ProcessInfoFile(short qFlag);
void SetOperationCursor(void);
short ACKFileOpen(HWND hWnd,char *szName,short nMaxLen,
        char *szFileList,char *szTitle,char *szExt);
short ACKFileSave(HWND hWnd,char *szName,short nMaxLen,
        char *szFileList,char *szTitle);

// Comment next line out to use ACK palette
//#define _USE_SYSTEM_PALETTE_

LRESULT CALLBACK _export WndProc( HWND hWnd, UINT iMessage,
                                 WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK _export ChildWndProc( HWND hWnd, UINT iMessage,
                                 WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK _export AckWndProc( HWND hWnd, UINT iMessage,
                                 WPARAM wParam, LPARAM lParam);



// Various operations for cursor control
#define OP_NORMAL       0   // No special operation
#define OP_WAIT         1   // Display hourglass


// Current view being displayed in grid window
#define VIEW_WALLS      1
#define VIEW_FLOOR      2
#define VIEW_CEILING    3

// Current edit settings
#define EDIT_WALLS      1
#define EDIT_OBJECTS    2

#define SPEED_AMOUNT    5

#ifndef STRUCT_HEADER
#define STRUCT_HEADER
typedef struct header
{
	BITMAPINFOHEADER Header;
	RGBQUAD aColors[256];
} header;
#endif

#ifndef STRUCT_PAL
#define STRUCT_PAL
typedef struct pal
{
	WORD Version;
	WORD NumberOfEntries;
	PALETTEENTRY aEntries[256];
} pal;
#endif

#ifndef STRUCT_PALSLOT
#define STRUCT_PALSLOT
//MEH structure for holding RGB data
typedef struct {
    unsigned char r, g, b, a;
} PalletteSlot;
#endif




//*****************************************************************************
//
//*****************************************************************************
class Main
{
public:
  static HINSTANCE hInstance;
  static HINSTANCE hPrevInstance;
  static int nCmdShow;
  static int MessageLoop( void );
};

//*****************************************************************************
//
//*****************************************************************************
class Window
{
protected:
    HWND hWnd;
    HWND hWndChild;

public:
    // Provide (read) access to the window's handle in case it is needed
    // elsewhere.
    HWND    GetHandle( void ) {return hWnd;};
    HWND    GetChildHandle(void) {return hWndChild;};

    BOOL    Show( int nCmdShow ) { return ShowWindow( hWnd, nCmdShow ); }
    void    Update( void ) { UpdateWindow( hWnd ); }

    // Pure virtual function makes Window an abstract class.
    virtual LRESULT WndProc(UINT iMessage, WPARAM wParam, LPARAM lParam ) = 0;
};


//*****************************************************************************
//
//*****************************************************************************
class MainWindow : public Window
{
private:
    static char szClassName[14];
    // Helper function used by Paint function; it is used as a
    // callback function by LineDDA.

// Constructor
public:
    MainWindow(void);

public:
    static void Register(void);
    void CreateAll(void);

    LRESULT WndProc( UINT iMessage, WPARAM wParam, LPARAM lParam );

    void DrawGrid(HDC hdc,short wt,short ht);
    void Paint( void );
};


