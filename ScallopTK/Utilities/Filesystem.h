
#ifndef SCALLOP_TK_FS
#define SCALLOP_TK_FS

#ifdef WIN32
  #include "FilesystemWin32.h"
#else
  #include "FilesystemUnix.h"
#endif

#endif
