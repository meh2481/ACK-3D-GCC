// Header file for Windows implementation of the ACK-3D engine
#ifndef ACKWIN_H
#define ACKWIN_H

#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

short InitACKEngine(void);
void AckPaint(); // Actual paint routine for 3D Window
void ProcessKeys(SDL_Event event);
//void AckDoSize(HWND hWnd,LPARAM lParam);
void DoCycle(void);

//extern void ClearSystemPalette(void);
//extern HPALETTE CreateIdentityPalette(RGBQUAD aRGB[], int nColors);
//extern void AppActivate(bool fActive);

#endif //Defined ACKWIN_H

