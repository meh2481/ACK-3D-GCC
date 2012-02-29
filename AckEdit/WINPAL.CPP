///////////////////////////////////////////////////////////////////////////
// WINPAL.CPP
//
// Hacked from WinG sample PALANIM sample.c.  Only changes were replacing
// WinG calls with the WinG interface class, and removing the two routines
// at the end of sample.c, as they were of no use here.
//
// Basically pure Microsoft WinG sample code.
// 7/12/94 Chris Thomas (GCT)
///////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <windowsx.h>

//#include "wing.h"
//#include "wingdll.h"

//extern WinGdll WinG;

// *** Creating an identity palette code here

HPALETTE CreateIdentityPalette(RGBQUAD aRGB[], int nColors)
{
	int i;
	struct
	{
		WORD Version;
		WORD NumberOfEntries;
		PALETTEENTRY aEntries[256];
	} Palette =
	{
		0x300,
		256
	};
	HDC hdc = GetDC(NULL);

	// *** For SYSPAL_NOSTATIC, just copy the color table into
	// *** a PALETTEENTRY array and replace the first and last entries
	// *** with black and white
	if (GetSystemPaletteUse(hdc) == SYSPAL_NOSTATIC)
	{
		// *** Fill in the palette with the given values, marking each
		// *** as PC_RESERVED
		for(i = 0; i < nColors; i++)
		{
			Palette.aEntries[i].peRed = aRGB[i].rgbRed;
			Palette.aEntries[i].peGreen = aRGB[i].rgbGreen;
			Palette.aEntries[i].peBlue = aRGB[i].rgbBlue;
			Palette.aEntries[i].peFlags = PC_RESERVED;
		}

		// *** Mark any remaining entries PC_RESERVED
		for (; i < 256; ++i)
		{
			Palette.aEntries[i].peFlags = PC_RESERVED;
		}

		// *** Make sure the last entry is white
		// *** This may replace an entry in the array!
		Palette.aEntries[255].peRed = 255;
		Palette.aEntries[255].peGreen = 255;
		Palette.aEntries[255].peBlue = 255;
		Palette.aEntries[255].peFlags = 0;

		// *** And the first is black
		// *** This may replace an entry in the array!
		Palette.aEntries[0].peRed = 0;
		Palette.aEntries[0].peGreen = 0;
		Palette.aEntries[0].peBlue = 0;
		Palette.aEntries[0].peFlags = 0;
	}
	else
	// *** For SYSPAL_STATIC, get the twenty static colors into
	// *** the array, then fill in the empty spaces with the
	// *** given color table
	{
		int nStaticColors;
		int nUsableColors;

		// *** Get the static colors
		nStaticColors = GetDeviceCaps(hdc, NUMCOLORS);
		GetSystemPaletteEntries(hdc, 0, 256, Palette.aEntries);

		// *** Set the peFlags of the lower static colors to zero
		nStaticColors = nStaticColors / 2;
		for (i=0; i<nStaticColors; i++)
			Palette.aEntries[i].peFlags = 0;

		// *** Fill in the entries from the given color table
		nUsableColors = nColors - nStaticColors;
		for (; i<nUsableColors; i++)
		{
			Palette.aEntries[i].peRed = aRGB[i].rgbRed;
			Palette.aEntries[i].peGreen = aRGB[i].rgbGreen;
			Palette.aEntries[i].peBlue = aRGB[i].rgbBlue;
			Palette.aEntries[i].peFlags = PC_RESERVED;
		}

		// *** Mark any empty entries as PC_RESERVED
		for (; i<256 - nStaticColors; i++)
			Palette.aEntries[i].peFlags = PC_RESERVED;

		// *** Set the peFlags of the upper static colors to zero
		for (i = 256 - nStaticColors; i<256; i++)
			Palette.aEntries[i].peFlags = 0;
	}

	ReleaseDC(NULL, hdc);

	// *** Create the palette
	return CreatePalette((LOGPALETTE *)&Palette);
}

// *** Resetting the system palette code here

void ClearSystemPalette(void)
{
	// *** A dummy palette setup
	struct
	{
		WORD Version;
		WORD NumberOfEntries;
		PALETTEENTRY aEntries[256];
	} Palette =
	{
		0x300,
		256
	};

	HPALETTE ScreenPalette = 0;
	HDC ScreenDC;
	int Counter;

	// *** Reset everything in the system palette to black
	for(Counter = 0; Counter < 256; Counter++)
	{
		Palette.aEntries[Counter].peRed = 0;
		Palette.aEntries[Counter].peGreen = 0;
		Palette.aEntries[Counter].peBlue = 0;
		Palette.aEntries[Counter].peFlags = PC_NOCOLLAPSE;
	}

	// *** Create, select, realize, deselect, and delete the palette
	ScreenDC = GetDC(NULL);
	ScreenPalette = CreatePalette((LOGPALETTE *)&Palette);
	ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
	RealizePalette(ScreenDC);
	ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
	DeleteObject(ScreenPalette);
	ReleaseDC(NULL, ScreenDC);
}

// *** Setting up SYSPAL_NOSTATIC

#define NumSysColors (sizeof(SysPalIndex)/sizeof(SysPalIndex[1]))
#define rgbBlack RGB(0,0,0)
#define rgbWhite RGB(255,255,255)

// *** These are the GetSysColor display element identifiers
static int SysPalIndex[] = {
	COLOR_ACTIVEBORDER,
	COLOR_ACTIVECAPTION,
	COLOR_APPWORKSPACE,
	COLOR_BACKGROUND,
	COLOR_BTNFACE,
	COLOR_BTNSHADOW,
	COLOR_BTNTEXT,
	COLOR_CAPTIONTEXT,
	COLOR_GRAYTEXT,
	COLOR_HIGHLIGHT,
	COLOR_HIGHLIGHTTEXT,
	COLOR_INACTIVEBORDER,

	COLOR_INACTIVECAPTION,
	COLOR_MENU,
	COLOR_MENUTEXT,
	COLOR_SCROLLBAR,
	COLOR_WINDOW,
	COLOR_WINDOWFRAME,
	COLOR_WINDOWTEXT
};

// *** This array translates the display elements to black and white
static COLORREF MonoColors[] = {
	rgbBlack,
	rgbWhite,
	rgbWhite,
	rgbWhite,
	rgbWhite,
	rgbBlack,
	rgbBlack,
	rgbBlack,
	rgbBlack,
	rgbBlack,
	rgbWhite,
	rgbWhite,
	rgbWhite,
	rgbWhite,
	rgbBlack,
	rgbWhite,
	rgbWhite,
	rgbBlack,

	rgbBlack
};

// *** This array holds the old color mapping so we can restore them
static COLORREF OldColors[NumSysColors];

// *** AppActivate sets the system palette use and
// *** remaps the system colors accordingly.
void AppActivate(BOOL fActive)
{
	HDC hdc;
	int i;

	// *** Just use the screen DC
	hdc = GetDC(NULL);

	// *** If the app is activating, save the current color mapping
	// *** and switch to SYSPAL_NOSTATIC
	if (fActive && GetSystemPaletteUse(hdc) == SYSPAL_STATIC)

	{
		// *** Store the current mapping
		for (i=0; i<NumSysColors; i++)
			OldColors[i] = GetSysColor(SysPalIndex[i]);

		// *** Switch to SYSPAL_NOSTATIC and remap the colors
		SetSystemPaletteUse(hdc, SYSPAL_NOSTATIC);
		SetSysColors(NumSysColors, SysPalIndex, MonoColors);
	}
	else if (!fActive && GetSystemPaletteUse(hdc) == SYSPAL_NOSTATIC)
	{
		// *** Switch back to SYSPAL_STATIC and the old mapping
		SetSystemPaletteUse(hdc, SYSPAL_STATIC);

		SetSysColors(NumSysColors, SysPalIndex, OldColors);
	}

	// *** Be sure to release the DC!
	ReleaseDC(NULL,hdc);
}

