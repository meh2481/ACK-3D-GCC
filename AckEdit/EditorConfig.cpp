//EditorConfig.cpp
//Used to save/load the editor configuration, so some settings are kept from one editor launch to another
//Mark Hutcheson 2012

//#define  STRICT
#include <windows.h>
//#pragma hdrstop
//#include <stdlib.h>
//#include <string.h>
//#include "wing.h"
//#include "wingdll.h"
//#include "ackwin.h"

//extern "C" {
//#include "keys.h"
//#include "ack3d.h"

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

//--------------------------------------------------
// ReadConfig() - read in our configuration settings
//--------------------------------------------------
void ReadConfig()
{
    std::ifstream infile(sConfigFile);
    if(infile.fail())
        return; //if it isn't there, we don't care

    infile >> bTrackPlayer;
    infile >> nViewType;
    infile >> nEditType;
    infile >> bShowCoords;
    infile >> StretchFactor;
    infile >> nBackColor;

    infile.close();
}

//------------------------------------------------------
// WriteConfig() - write these settings back to the file
//------------------------------------------------------
void WriteConfig()
{
    std::ofstream ofile(sConfigFile);
    if(ofile.fail())
        return; //Can't write this file

    ofile << bTrackPlayer << endl;
    ofile << nViewType << endl;
    ofile << nEditType << endl;
    ofile << bShowCoords << endl;
    ofile << StretchFactor << endl;
    ofile << nBackColor << endl;

    ofile.close();
}
