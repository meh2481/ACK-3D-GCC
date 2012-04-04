// Header file for Windows implementation of the ACK-3D engine
#ifndef ACKWIN_H
#define ACKWIN_H

short InitACKEngine(void);
void AckPaint(HWND hWnd,HDC hdc); // Actual paint routine for 3D Window
void ProcessKeys(UINT iMessage,WPARAM wParam);
void AckDoSize(HWND hWnd,LPARAM lParam);
void DoCycle(void);

//extern void ClearSystemPalette(void);
//extern HPALETTE CreateIdentityPalette(RGBQUAD aRGB[], int nColors);
//extern void AppActivate(BOOL fActive);

#endif //Defined ACKWIN_H

