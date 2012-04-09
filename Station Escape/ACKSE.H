// Header file for Station Escape Demo Program
// (c) 1995 Ack Software (Lary Myers)

BOOL Register(HINSTANCE hInst);
BOOL CreateAll(HINSTANCE hInst);
int MessageLoop(void);
LRESULT  CALLBACK _export AckWndProc( HWND hWnd, UINT iMessage, WPARAM wParam,
                                 LPARAM lParam );

int LoadBackDrop(void);
int AckProcessInfoFile(void);

//#define IM_TIMEOUT  3       // Ran out of time
//#define IM_WON      4       // Won the game!

#define REPAIR_OBJECTS_NEEDED 3    // Number of parts needed to win
#define REPAIR_POSITION       3769 // Location on map to win

#define BOMB_START_INDEX    32      // Starting object number
#define BOMB_END_INDEX      40      // Ending object number

//MEH structure for holding RGB data
typedef struct {
    unsigned char r, g, b, a;
} PalletteSlot;

//MEH read in pallette data from "pallette.txt"
void ReadInPallette(void);

//MEH Stretch an image. But it's too slow.
//void StretchImage(unsigned char* src, unsigned char* dest,
//                  unsigned int srcW, unsigned int srcH, unsigned int destW, unsigned int destH);
