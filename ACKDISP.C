//#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
//#include <dos.h>
//#include <mem.h>
//#include <io.h>
//#include <fcntl.h>
#include <time.h>
#include <string.h>
//#include <sys\stat.h>

//typedef unsigned short USHORT;

#include "ACK3D.H"
#include "ACKENG.H"
#include "ACKEXT.H"


void AckDrawPage (void);

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// This function has been replaced by AckDisplayScreen in assembler
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
short xxxAckDisplayScreen (void)
{

/* Let the assembly routine do the hard work */
  AckDrawPage ();

  return (0);
}


