//EditorConfig.cpp
//Used to save/load the editor configuration, so some settings are kept from one editor launch to another
//Mark Hutcheson 2012

#include <windows.h>

//For reading and writing the config file
#include <iostream>
#include <fstream>
using std::endl;

//Default name for text file we'll open
const char sConfigFile[] = "config.txt";

//Variables we want to keep in the config
extern BOOL        bTrackPlayer;   // On if map tracks player position
extern short       nViewType;      // Current view in grid window
extern short       nEditType;      // Editing walls or objects
extern BOOL        bShowCoords;    // Show the coordinates or not
extern int         StretchFactor;
extern short       nBackColor;
extern short       CurrentBitmap;
extern short       CurrentTopBitmap;

//--------------------------------------------------
// ReadConfig() - read in our configuration settings
//--------------------------------------------------
void ReadConfig()
{
    //Make sure we're in the right directory for our config heres. Don't want to save config files all over the place.
    CHAR PathBuffer[256];
    GetCurrentDirectory(256, PathBuffer);
    SetCurrentDirectory("./");

    std::ifstream infile(sConfigFile);
    if(infile.fail())
    {
        SetCurrentDirectory(PathBuffer);
        infile.close();
        return; //if it isn't there, we don't care
    }

    infile >> bTrackPlayer;
    infile >> nViewType;
    infile >> nEditType;
    infile >> bShowCoords;
    infile >> StretchFactor;
    infile >> nBackColor;
    infile >> CurrentBitmap;
    infile >> CurrentTopBitmap;

    infile.close();

    SetCurrentDirectory(PathBuffer);
}

//------------------------------------------------------
// WriteConfig() - write these settings back to the file
//------------------------------------------------------
void WriteConfig()
{
    CHAR PathBuffer[256];
    GetCurrentDirectory(256, PathBuffer);
    SetCurrentDirectory("./");

    std::ofstream ofile(sConfigFile);
    if(ofile.fail())
    {
        ofile.close();
        SetCurrentDirectory(PathBuffer);
        return; //Can't write this file
    }

    ofile << bTrackPlayer << endl;
    ofile << nViewType << endl;
    ofile << nEditType << endl;
    ofile << bShowCoords << endl;
    ofile << StretchFactor << endl;
    ofile << nBackColor << endl;
    ofile << CurrentBitmap << endl;
    ofile << CurrentTopBitmap << endl;

    ofile.close();
    SetCurrentDirectory(PathBuffer);
}
