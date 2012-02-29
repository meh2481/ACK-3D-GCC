// Header file for Windows implementation of the ACK-3D engine

// Uncomment next line to use normal windows colors
//#define _USE_SYSTEM_PALETTE 1


short InitACKEngine(void);
void AckPaint(HWND hWnd,HDC hdc); // Actual paint routine for 3D Window
void ProcessKeys(UINT iMessage,WPARAM wParam);
void ProcessMouse(HWND hWnd,short nType,short xPos,short yPos);
void AckDoSize(HWND hWnd,LPARAM lParam);
void CreateAckPalette(void);
void DoCycle(void);

extern void ClearSystemPalette(void);
extern HPALETTE CreateIdentityPalette(RGBQUAD aRGB[], int nColors);
extern void AppActivate(BOOL fActive);

typedef struct header
{
	BITMAPINFOHEADER Header;
	RGBQUAD aColors[256];
} header;

typedef struct pal
{
	WORD Version;
	WORD NumberOfEntries;
	PALETTEENTRY aEntries[256];
} pal;

