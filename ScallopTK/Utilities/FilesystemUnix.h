
#ifndef SCALLOP_TK_FILESYSTEM_UNIX_H_
#define SCALLOP_TK_FILESYSTEM_UNIX_H_

#ifndef SCALLOP_TK_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

// Namespaces
using namespace std;

// Constants
const int MAX_DIR_SIZE = 400;
const int DIR_VAL      = 0x4;
const int FILE_VAL     = 0x8;

// Recursively list all files and subdirs in dir_name
inline bool ListAllFiles( string dir_name, vector<string>& fileList, vector<string>& subdirs  ) {
  // Declare necessary variables
  char filepath[MAX_DIR_SIZE];
  char subdir[MAX_DIR_SIZE];
  struct dirent *entry;
  struct stat fileprop;
  // Open dir
  DIR *dir = opendir( dir_name.c_str() );
  // Validity check
  if( dir == NULL )
    return false;
  /* Cycle through every entry printing out files in this dir first.
   * Makes 2 cycles to print out all files first, before venturing
   * into subdirs. Slightly inefficient but easier than making a 
   * data structure to remember folders. */ 
  while( (entry = readdir(dir)) != NULL ) {
    // Skip top and current dir
    if( strcmp(entry->d_name,".") == 0 )
      continue;
    if( strcmp(entry->d_name,"..") == 0 )
      continue;
    // Skip sub-directories on first pass
    if( entry->d_type != FILE_VAL )
      continue;
    // Read file properties (includes size)
    strcpy(filepath,dir_name.c_str());
    strcat(filepath,"/");
    strcat(filepath,entry->d_name);
    fileList.push_back( filepath );  
  }
  rewinddir(dir);
  // Cycle through subdirs and call listAll recursively
  while( (entry = readdir(dir)) != NULL ) {
    // Skip top and current dir
    if( strcmp(entry->d_name,".") == 0 )
      continue;
    if( strcmp(entry->d_name,"..") == 0 )
      continue;
    // Skip files we've already outputted
    if( entry->d_type != DIR_VAL )
      continue;
    // Call listAllFile recursively
    strcpy(subdir,dir_name.c_str());
    strcat(subdir,"/");
    strcat(subdir,entry->d_name);
    subdirs.push_back(subdir);
    ListAllFiles(subdir,fileList,subdirs);
  }
  // Close stream and dealloc mem
  closedir(dir);
  // Sort file list based on filename
  sort( fileList.begin(), fileList.end() );
  return true;
}

inline int doesDirectoryExist( const char* filename ) {
  struct stat fileprop;
  int ret = stat(filename, &fileprop);
  if( ret == 0 ) {
    if( S_ISDIR(fileprop.st_mode) )
      return 1;
  }
  return 0;
}

inline bool CopyDirTree( vector<string>& dirlist, const string& input_dir, const string& output_dir) {
  if( !doesDirectoryExist( output_dir.c_str() ) ) {
    mkdir( output_dir.c_str(), 0xFFF );
  }
  for( unsigned int i=0; i<dirlist.size(); i++ ) {
    string toMake = dirlist[i];
    toMake.erase( 0, input_dir.size() ); 
    toMake = output_dir + toMake;
    if( !doesDirectoryExist( toMake.c_str() ) ) {
      mkdir( toMake.c_str(), 0xFFF );
    }
  }
  return true;
}

inline void FormatOutputNames( vector<string>& in, vector<string>& out, 
                             const string& in_dir, const string& out_dir ) {

  for( unsigned int i=0; i<in.size(); i++ ) {
    string ofile = in[i];
    ofile.erase( 0, in_dir.size() ); 
    ofile = out_dir + ofile;
    out.push_back( ofile );
  }
}

inline string GetExectuablePath()
{
  char procname[2024];
  int len = readlink("/proc/self/exe", procname, 2023);
  if (len <= 0) {
    // I guess we're not running on the right version of unix
    cerr << "ERROR: Unable to get path to executable" << endl;
    return "";
  }
  procname[len] = '\0';
  string output = procname;
  
  return procname;
}

inline bool DoesFileExist( string path )
{
  ifstream test( path.c_str() );
  if( !test )
  {
    test.close();
    return false;
  }
  test.close();
  return true;
}


#endif

#endif
