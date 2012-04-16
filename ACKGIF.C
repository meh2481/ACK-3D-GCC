// This source file contains the functions needed to read in PNG files.
// (c) 2012 ACK Software (Mark Hutcheson)
//#include <windows.h>
//#include <stdio.h>
//#include <conio.h>
//#include <process.h>
//#include <bios.h>
//#include <fcntl.h>
//#include <malloc.h>
//#include <string.h>
/*
#include "ack3d.h"
#include "ackeng.h"
#include "ackext.h"


//------------------------------------------------------------------------
// Read a PNG format file and return a buffer containing the uncompressed
// image.
//------------------------------------------------------------------------
//#pragma argsused
unsigned char *AckReadGIF (char *picname)
{
// This is a stub routine used only as a place holder for the actual
// PNG read routine.
return(NULL);
}*/
// **** End of Source ****

//MEH All the patents have expired! YES! GIF support ftw!

#include "windowsredef.h"
#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
//#include <process.h>
//#include <bios.h>
//#include <fcntl.h>
//#include <malloc.h>
#include <string.h>
#include "ack3d.h"
#include "ackeng.h"
#include "ackext.h"

#define MAX_CODES     4096

extern unsigned char colordat[];/* Palette buffer in AckReadiff module */


static short fp;
static short curr_size;
static short clear;
static short ending;
static short newcodes;
static short top_slot;
static short slot;
static short navail_bytes = 0;
static short nbits_left = 0;
static unsigned char b1;
static unsigned char byte_buff[257];
static unsigned char *pbytes;
static unsigned char *stack;
static unsigned char *suffix;
static unsigned short *prefix;
static unsigned char cTemp;  //MEH General use temp variable

static unsigned long code_mask[13] =
{
  0L,
  0x0001L, 0x0003L,
  0x0007L, 0x000FL,
  0x001FL, 0x003FL,
  0x007FL, 0x00FFL,
  0x01FFL, 0x03FFL,
  0x07FFL, 0x0FFFL
};

static short get_next_code (void)
{
  short i;
  static unsigned long ret;

  if (!nbits_left)
    {
      if (navail_bytes <= 0)
	{
	  pbytes = byte_buff;
	  _lread(fp,&cTemp,1);
	  navail_bytes = cTemp;

	  if (navail_bytes)
	    {
	      for (i = 0; i < navail_bytes; ++i)
		  {
		      _lread(fp,&cTemp,1);
		      *(byte_buff + i) = cTemp;
		  }
	    }
	}
      b1 = *pbytes++;
      nbits_left = 8;
      --navail_bytes;
    }

  ret = b1 >> (8 - nbits_left);
  while (curr_size > nbits_left)
    {
      if (navail_bytes <= 0)
	{
	  pbytes = byte_buff;
	  _lread(fp,&cTemp,1);
	  navail_bytes = cTemp;
	  if (navail_bytes)
	    {
	      for (i = 0; i < navail_bytes; ++i)
          {
              _lread(fp,&cTemp,1);
              *(byte_buff + i) = cTemp;
          }
	    }
	}
      b1 = *pbytes++;
      ret |= b1 << nbits_left;
      nbits_left += 8;
      --navail_bytes;
    }

  nbits_left -= curr_size;

  return ((short) (ret & *(code_mask + curr_size)));
}


//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// Read and process a GIF format image and return the resulting buffer to
// the caller. The global array colordat contains the palette information
// for the image just read.
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
unsigned char *AckReadGIF (char *picname)
{
  unsigned char *sp;
  unsigned char *buffer, *OrgBuffer;
  short code, fc, oc;
  short i;
  unsigned char size;
  short c;
  unsigned short wt, ht;
  int bSize;
  unsigned char buf[1028];
  unsigned char red;
  unsigned char grn;
  unsigned char blu;

  if (!rsHandle)
        fp = _lopen(picname,OF_READ);
    else
        {
        fp = rsHandle;
        _llseek(rsHandle,rbaTable[(ULONG)picname],SEEK_SET);
        }

  _lread (fp,buf,6);
  if (strncmp ((const char*)(buf), "GIF", 3))
    {
      if (!rsHandle)
	_lclose (fp);

      ErrorCode = ERR_INVALIDFORM;
      return (NULL);
    }

  _lread (fp,buf,7);
  for (i = 0; i < 768;)
    {
      _lread(fp,&red,1);
      _lread(fp,&grn,1);
      _lread(fp,&blu,1);
      colordat[i++] = red >> 2;
      colordat[i++] = grn >> 2;
      colordat[i++] = blu >> 2;
    }

  _lread (fp,buf,5);
  _lread (fp,buf,2);
  wt = (*(short *) buf);
  _lread (fp,buf,2);
  ht = (*(short *) buf);

  bSize = (ht * wt) + (sizeof (short) * 2);
  buffer = (UCHAR *) AckMalloc (bSize);
  if (buffer == NULL)
    {
      if (!rsHandle)
	_lclose (fp);

      ErrorCode = ERR_NOMEMORY;
      return (NULL);
    }

  OrgBuffer = buffer;

  (*(short *) buffer) = wt;
  buffer += sizeof (short);
  (*(short *) buffer) = ht;
  buffer += sizeof (short);

  _lread (fp,buf,1);
  _lread(fp,&size,1);

  if (size < 2 || 9 < size)
    {
      if (!rsHandle)
	_lclose (fp);
      AckFree (OrgBuffer);
      ErrorCode = ERR_INVALIDFORM;
      return (NULL);
    }

  stack = (unsigned char *) AckMalloc (MAX_CODES + 1);
  suffix = (unsigned char *) AckMalloc (MAX_CODES + 1);
  prefix = (unsigned short *) AckMalloc (sizeof (short) * (MAX_CODES + 1));

  if (stack == NULL || suffix == NULL || prefix == NULL)
    {
      if (!rsHandle)
	_lclose (fp);
      AckFree (OrgBuffer);
      ErrorCode = ERR_NOMEMORY;
      return (NULL);
    }

  curr_size = size + 1;
  top_slot = 1 << curr_size;
  clear = 1 << size;
  ending = clear + 1;
  slot = newcodes = ending + 1;
  navail_bytes = nbits_left = 0;
  oc = fc = 0;
  sp = stack;

  while ((c = get_next_code ()) != ending)
    {
      if (c == clear)
	{
	  curr_size = size + 1;
	  slot = newcodes;
	  top_slot = 1 << curr_size;
	  while ((c = get_next_code ()) == clear) ;

	  if (c == ending)
	    break;

	  if (c >= slot)
	    c = 0;

	  oc = fc = c;
	  *buffer++ = c;
	}
      else
	{
	  code = c;
	  if (code >= slot)
	    {
	      code = oc;
	      *sp++ = fc;
	    }

	  while (code >= newcodes)
	    {
	      *sp++ = *(suffix + code);
	      code = *(prefix + code);
	    }

	  *sp++ = code;
	  if (slot < top_slot)
	    {
	      *(suffix + slot) = fc = code;
	      *(prefix + slot++) = oc;
	      oc = c;
	    }

	  if (slot >= top_slot && curr_size < 12)
	    {
	      top_slot <<= 1;
	      ++curr_size;
	    }

	  while (sp > stack)
	    {
	      --sp;
	      *buffer++ = *sp;
	    }
	}
    }

  AckFree (stack);
  AckFree (suffix);
  AckFree (prefix);

  if (!rsHandle)
    _lclose (fp);

  return (OrgBuffer);
}


