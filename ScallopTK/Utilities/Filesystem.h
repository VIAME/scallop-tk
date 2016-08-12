
#ifndef SCALLOP_TK_FILESYSTEM_H_
#define SCALLOP_TK_FILESYSTEM_H_

#ifdef WIN32
  #include "FilesystemWin32.h"
#else
  #include "FilesystemUnix.h"
#endif

#endif
