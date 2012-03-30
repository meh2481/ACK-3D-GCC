// ACKRTNRecode.c
// ACK-3D C source
// Mark H's reimplementation of needed routines from ACKRTN*.ASM
// Copyright (c) 2012, except functions based off C or ASM routines in book

#include "ACKRTNRecode.h"

//BuildUpView - our big workhorse to make sure the view and everything gets built properly.
void BuildUpView(void)
{
    //Clear out the screen buffer to start our next pass (Just so we avoid issues with uninitialized stuff when the map isn't
                                                       // built fully yet or something)
    int j,k; // j/k. Seewhatididthere?
    for(j = 0; j < 320; j++)
    {
        for(k = 0; k < 200; k++)
        {
            gScrnBuffer[j*200+k] = 0;
        }
    }


    CheckDoors();   //Check the doors to update them and such.

    //Clear our hit map
    int i;  //General-purpose increment variable (don't declare in for loops because C doesn't support it)
    for(i = 0; i < 4096; i++)
    {
        HitMap[i] = 0;
    }

    MaxDistance = 0; //Reset our maximum distance

    FoundObjectCount = MoveObjectCount; //Store the number of movable objects we've seen (reset to all of them for now)

    //Move the objects we've seen into MoveObjectList[]
    if (MoveObjectCount)
        memmove(ObjectsSeen,MoveObjectList,MoveObjectCount);
    /*for(i = 0; i < MAX_OBJECTS + 1; i++)
    {
        MoveObjectList[i] = ObjectsSeen[i];
    }*/

    //buv010
    //Get the right side of the viewport and make sure it's 320
    unsigned int iRightSide = gWinEndX;
    if(iRightSide < 320)
        iRightSide++;

    //buv020
    //Get the starting viewport angle
    unsigned int iStartAngle = PlayerAngle - INT_ANGLE_32;
    if(iStartAngle < 0) //If we've gone too far,
        iStartAngle += INT_ANGLE_360;   //Add 360 degrees to the angle

    //buv030
    unsigned int iLeftSide = gWinStartX;
    ViewColumn = iLeftSide;
    iStartAngle += iLeftSide;
    if(iStartAngle > INT_ANGLE_360)
        iStartAngle -= INT_ANGLE_360;

    //buv040
    ViewAngle = iStartAngle;

    //buv050: Main loop
    for(i = 0; i < VIEW_WIDTH; i++)
    {
        //I'm not sure what I did, but somehow I ended up with angles from 0 to -32 degrees. Not good. Brute force error handling FTW!
        if(ViewAngle < 0)
            ViewAngle += INT_ANGLE_360;

        //printf("Now in loop %d\n", i);
        WallDistTable[i] = 2048; //init to max dist by default
        sPtr = &(Slice[ViewColumn]);
        //movzx edi,[_ViewAngle]
        xRaySetup();
        //buv060
        yRaySetup();
        //buv070
        LastWallHeight = 0;
        BuildSlice();

        //multi-height wall stuff. We shouldn't really worry about this stuff, rite?
        /*if(gMultiWalls != 0 && LastWallHeight < 200 && sPtr->Next != NULL && sPtr->Distance > 96)
        {
            BuildSliceMulti();
        }*/

        //buv080
        //Increment view column and angle
        ViewColumn++;
        ViewAngle++;
        //I'm not sure about this. RES_LOW causes us to increment twice? Because the assembly makes it look vice versa
        if(Resolution == RES_LOW)
        {
            ViewColumn++;
            ViewAngle++;
            i++;
        }

        //buv090:
        if(ViewAngle >= INT_ANGLE_360)
            ViewAngle -= INT_ANGLE_360;

        //buv100: loop again

    }

    //buv_exit
    CheckHitMap();
    FindObject();
    FloorCeilRtn();
    DrawWalls();
}

//CheckHitMap() - checks if movable objects are in the view. I DON'T CARE! HAHAHA
void CheckHitMap(void)
{
    //For now, we'll say all the movable objects are in the view. Slower? Yes, but who cares?
}

//DrawWalls() - the workhorse for setting up all the global variables and stuff for each column of the view, so ShowCol() and
// variants can display the walls correctly. Does very little in my version of it.
void DrawWalls(void)
{
    //Store the floor height as the viewport height / 2
    Floorht = gWinHeight / 2;
    //and half-bitmap height
    sctopht = ViewHeight;
    scbotht = BITMAP_HEIGHT - sctopht;  //store height of remaining half of bitmap

    //Store screen buffer center row for easy access
    scVid = &(gScrnBuffer[gWinStartX + gCenterOff]);

    //Ignore low-res code, since the game has a REALLY low resolution anyway. :P
    //I mean, come on. Who wants to go low-res in a 320x200 game nowdays? Seriously?

    //stw010: Start main loop
    int i;
    for(i = 0; i < VIEW_WIDTH; i++)
    {
        SLICE* CurSlice = &Slice[i];
        //stw020:
        //See if there's a slice or slices behind this one. If so, go to the farthest one first
        while(1)
        {
            if(!CurSlice->Active)
                break;
            if(CurSlice->Next == NULL)
                break;
            CurSlice = CurSlice->Next;
        }

        //stw030:
        scbNum = CurSlice->bNumber;
        scmulcnt = scbNum & 0xFF;
        scmulti = 0;
        if(scbNum & WALL_TYPE_MULTI)    //Deal with a multi-height wall
        {
            scMulData = CurSlice->mPtr;
            if(scMulData)
                scmulti = 1;
        }

        bmDistance = CurSlice->Distance;

        ShadingRegion = bmDistance >> 6;
        if(ShadingRegion > 15)
            ShadingRegion = 15;

        //stw050: Light shading stuffz - Currently I have no idea what this does and I don't care
        //ShadingRegion >>= 8; //Multiply by 256 for pallette entry
        scColumn = CurSlice->bMap[CurSlice->bColumn];
        scWall = *(CurSlice->bMap);    //?????????
        scPal = &(gPalTable[ShadingRegion]);

        gCurSlice = CurSlice;
        g_iCurScreenSpot = i;

        CurSlice->Fnc();    //Draw this slice
        //Saving stuff after here? I hope it isn't important

        //stw075:
        if(CurSlice->Prev != NULL)
        {
            i--;
            CurSlice = CurSlice->Prev;
            CurSlice->Active = 0;  //MEH: don't get into infinite loop
            continue;   //keep going
        }
    }
}

//Draw a wall column without light shading
void ShowColNS(void)
{
    ShowColumn(0,0);

}

//Draw a wall column without light shading, replacing black with transparency
void ShowColMaskNS(void)
{
    ShowColumn(1,0);
}

//ShowCol - Draw a column of wall with light shading
void ShowCol(void)
{
    ShowColumn(0,1);
}

//ShowColMask - Draw a column of wall with light shading, replacing black color with transparency
void ShowColMask(void)
{
    ShowColumn(1,1);    //Draw masked, with light shading
}


//NOTE: Depends on the ranges[] array declared in ackse.cpp
//Get the correct lightshaded value of the given pixel. Slower than his old table lookups? Quite, but I don't care, and couldn't
//figure out what was going on with his old table lookups. "When in doubt, ignore his way of doing things" is my motto, since I'm
//lazy. I don't exactly need the engine running at 400fps, either, like his code does on modern machines.
unsigned char GetLightShadedPixel(unsigned char cPixel)
{
    //The LowerTable tells us what pixels from the bitmap to draw
    int iColRange;
    for(iColRange = 0; iColRange < 64; iColRange++)
    {
        if(ranges[iColRange].start > cPixel || ranges[iColRange].start == 0)
            break;
        if((ranges[iColRange].start + ranges[iColRange].length - 1) >= cPixel)
        {
            cPixel+=ShadingRegion;
            if(cPixel > (ranges[iColRange].start + ranges[iColRange].length-1))
                cPixel = ranges[iColRange].start + ranges[iColRange].length-1;
        }
    }
    return cPixel;
}

//Draw this column; all four options in one for code reuse
void ShowColumn(char bTrans, char bLightShaded)
{
    unsigned short bNumber = gCurSlice->bNumber & 0xFF;
    short* sTableSpot = LowerTable[gCurSlice->Distance];
    //Get the height for the wall that we're supposed to draw
    unsigned int iWallHeight = DistanceTable[gCurSlice->Distance] - 1;
    if(iWallHeight > 99)
    {
        iWallHeight = 99;   //Force to max height
    }


    // Draw bottom half of bitmap - from middle of screen down
    UCHAR* CurCol = &(gScrnBuffer[g_iCurScreenSpot]);
    CurCol=&CurCol[VIEW_WIDTH * gWinHeight/2];
    UCHAR* BmpCol = gCurSlice->bMap[bNumber];
    if(BmpCol == NULL)
        return;             //TODO: DERP! Sometimes this is true for doors, and would cause crashes if this wasn't here.
                            // WHY?!? (Example: exit maze door in cemetery level at an angle)
                            // Probably some kind of problem in ACKRAY.C, function x/yRayCast().
    BmpCol=&BmpCol[BITMAP_HEIGHT / 2];
    BmpCol = &BmpCol[gCurSlice->bColumn * BITMAP_WIDTH];
    //CurCol += ((200 - iWallHeight) / 2) * 320;
    int i;
    for(i = 0; i < iWallHeight; i++)
    {
        UCHAR cTemp = BmpCol[*sTableSpot];
        //Get lightshaded value if we should
        if(bLightShaded)
            cTemp = GetLightShadedPixel(cTemp);
        // If we're transparent and not invisible
        if(bTrans && cTemp)
            *CurCol = cTemp;
        //Not transparent
        else if(!bTrans)
            *CurCol = cTemp;
        CurCol=&CurCol[VIEW_WIDTH];
        sTableSpot++;
    }

    //Draw top half of bitmap - from middle of screen up
    iWallHeight++;
    sTableSpot = LowerTable[gCurSlice->Distance];
    CurCol = &(gScrnBuffer[g_iCurScreenSpot]);
    CurCol=&CurCol[VIEW_WIDTH * (gWinHeight/2 - 1)];
    BmpCol = gCurSlice->bMap[bNumber];
    BmpCol = &BmpCol[gCurSlice->bColumn * BITMAP_HEIGHT];
    BmpCol=&BmpCol[BITMAP_HEIGHT / 2 - 1];
    for(i = 0; i < iWallHeight; i++)
    {
        UCHAR cTemp = BmpCol[-(*sTableSpot)];
        //Get lightshaded value if we should
        if(bLightShaded)
            cTemp = GetLightShadedPixel(cTemp);
        // If we're transparent and not invisible
        if(bTrans && cTemp)
            *CurCol = cTemp;
        //Not transparent
        else if(!bTrans)
            *CurCol = cTemp;
        CurCol-=VIEW_WIDTH;
        sTableSpot++;
    }
}
