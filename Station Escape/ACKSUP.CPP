// Support routines for ACK applications
// (c) 1995 ACK Software (Lary Myers)
#define  STRICT
//#include <windows.h>
#pragma hdrstop
#include <stdlib.h>
#include <stdio.h>
//#include <dos.h>
//#include <mem.h>
//#include <io.h>
//#include <fcntl.h>
//#include <sys\stat.h>
#include <string.h>
#include "ackse.h"
#include "ackres.h"
#include "ack3d.h"

extern "C" {
#include "keys.h"
extern  UCHAR   *BackArray[];
};


extern  ACKENG  *ae;
extern  short   nInfoMode;

//bool CALLBACK _export FAR PASCAL InfoDlgProc(HWND hDlg, UINT iMessage,
//        WPARAM wParam, LPARAM lParam);

// Globals
    extern  char    LineBuffer[];
    extern  short   LastObjectIndex;
    extern  int     MapResource;
    extern  int     PalResource;
    extern  int     ResScreenBack;
    extern  int     ResScrollBack;


//**************************************************************************
//
//**************************************************************************
/*int DoInfoDlg(HINSTANCE hInst, HWND hWnd)
{
    DLGPROC   lpfnInfoDlgProc;
    int       iReturn;

//lpfnInfoDlgProc = (DLGPROC)MakeProcInstance((FARPROC)InfoDlgProc, hInst);

iReturn = DialogBox(hInst,MAKEINTRESOURCE(DLGINFO),hWnd,lpfnInfoDlgProc);

FreeProcInstance((FARPROC)lpfnInfoDlgProc);

if (iReturn < 0)
    MessageBox(hWnd,"Unable to open dialog box","Error",MB_OK);


return(iReturn);
}*/



//***************************************************************************
// Creates the correct arrays for displaying a rotating background
//***************************************************************************
void ProcessBackDrop(UCHAR *bPtr)
{
        int     i,j,pos;
        UCHAR   *aPtr;
for (i = 0; i < 320; i++)
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
        pos += 320;
        }
    }

for (i = 320; i < 640; i++)
    {
    pos = (i - 320) + 32004;
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
        pos += 320;
        }
    }

}

//***************************************************************************
// Loads a background image (mountains) and processes the image into the
// separate slices for use at display time. Currently a background image
// can be 640 columns wide. This number can be made dynamic if needbe and would
// need to be changed in the routine below and in the DrawBackground routine
// in the ACK engine.
//***************************************************************************
int LoadBackDrop(void)
{
        UCHAR   *bPtr;
if (ResScrollBack)
    {
    bPtr = AckReadPCX((char *)ResScrollBack);

    if (bPtr == NULL)
    {
        printf("bPtr = NULL\n");
        return(8);
    }

    ProcessBackDrop(bPtr);
    AckFree(bPtr);
    }

return(0);
}

//**************************************************************************
//
//**************************************************************************
/*short ReadTextFile(HWND hDlg,char *szName)
{
    short   handle;
    FILE    *fp;
    char    *s;
    char    szBuf[256];

fp = fopen(szName,"rt");
if (fp == NULL)
    {
    MessageBox(hDlg,"Unable to read text file","Error",MB_OK);
    return(-1);
    }

while (1)
    {
    if (feof(fp))
        break;

    *szBuf = '\0';
    fgets(szBuf,255,fp);
    if (*szBuf)
        {
        s = strchr(szBuf,13);
        if (s != NULL)
            *s = '\0';
        s = strchr(szBuf,10);
        if (s != NULL)
            *s = '\0';
        SendDlgItemMessage(hDlg,IDC_LISTBOX1,LB_ADDSTRING,0,(LPARAM)szBuf);
        }
    }

fclose(fp);

return(0);
}
*/
//**************************************************************************
//
//**************************************************************************
/*void SetupHelpDlg(HWND hDlg)
{
    HWND    hDlgItem;

hDlgItem = GetDlgItem(hDlg,IDC_ST_HEADER);
SetWindowText(hDlgItem,"Help for Station Escape");

//ReadTextFile(hDlg,"SEHELP.TXT");

}*/

//**************************************************************************
// General purpose information dialog box. Here we will fill in the header
// and footer static controls as well as a listbox with various information.
//**************************************************************************
/*bool CALLBACK _export FAR PASCAL InfoDlgProc(HWND hDlg, UINT iMessage,
        WPARAM wParam, LPARAM lParam)
{

switch (iMessage)
    {
    case WM_INITDIALOG:
        switch (nInfoMode)
            {
            case IM_HELP:
//                SetupHelpDlg(hDlg);
                break;

            case IM_BACK:
//                ReadTextFile(hDlg,"SEBACK.TXT");
                break;

            case IM_TIMEOUT:
//                ReadTextFile(hDlg,"SETIME.TXT");
                break;

            case IM_WON:
//                ReadTextFile(hDlg,"SEWON.TXT");
                break;

            default:
                break;
            }
        return(true);

    case WM_COMMAND:
        switch (wParam)
            {
            case IDOK:
                EndDialog(hDlg, true);
            break;

        case IDCANCEL:
            EndDialog(hDlg, false);
            break;

            default:
                return(false);
            }
        break;

    default:
        return(false);
    }

return(true);
}*/

//**** End of Source ****

