#ifndef WINDOWSREDEF_H
#define WINDOWSREDEF_H

#include <fcntl.h>

#define BUILD_FOR_LINUX 0
#if BUILD_FOR_LINUX
#include <sys/types.h>
#include <unistd.h>
#include <sys/io.h>
#define O_BINARY 0
#else
#include <io.h>
#endif

#define _llseek lseek
#define _lread  read
#define _lopen  open
#define _lwrite write
#define _lclose close
#define OF_READ O_RDONLY|O_BINARY
#define HFILE_ERROR -1

#endif


