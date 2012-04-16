//MEH For making all this file i/o stuff work and be more cross-platform (hopefully!)
//Needed for editor also, since it's so closely tied in with the engine
#ifndef FILEIO_H
#define FILEIO_H

#include <io.h>
#include <fcntl.h>
#define LSEEK   lseek
#define LREAD   read
#define LOPEN   open
#define LWRITE  write
#define LCLOSE  close
#define L_OF_READ O_RDONLY|O_BINARY
#define L_OF_READWRITE  O_RDWR|O_CREAT|O_TRUNC
#define L_HFILE_ERROR -1

#endif  //defined FILEIO_H
