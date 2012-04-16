#ifndef WINDOWSREDEF_H
#define WINDOWSREDEF_H

#include <io.h>
#include <fcntl.h>
#define _llseek lseek
#define _lread  read
#define _lopen  open
#define _lwrite write
#define _lclose close
#define OF_READ O_RDONLY|O_BINARY
#define HFILE_ERROR -1

#endif


