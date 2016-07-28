
#ifndef FS_WIN32
#define FS_WIN32

#ifdef WIN32

// C++ Includes
#include <string>
#include <vector>
#include <iostream>
#include <stack>
#include <fstream>

// Windows Includes
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

// Namespaces
using namespace std;

// Function Prototypes
bool ListAllFiles(string path, vector<string>& files, vector<string>& dirlist);
bool CopyDirTree( vector<string>& dirlist, const string& input_dir, const string& output_dir);
void FormatOutputNames( vector<string>& in, vector<string>& out, const string& in_dir, const string& out_dir );
void CullNonImages( vector<string>& fn_list );
string GetExecutablePath();

// Function Declarations
inline bool ListAllFiles(string path, vector<string>& files, vector<string>& dirlist) {
  string mask = "*.*";
  HANDLE hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA ffd;
  string spec;
  stack<string> directories;

  directories.push(path);
  files.clear();
  dirlist.clear();

    while (!directories.empty()) {
        path = directories.top();
        dirlist.push_back(path);
        spec = path + "\\" + mask;
        directories.pop();

        hFind = FindFirstFile(spec.c_str(), &ffd);
        if (hFind == INVALID_HANDLE_VALUE) {
            return false;
        } 

        do {
            if (strcmp(ffd.cFileName, ".") != 0 && 
                strcmp(ffd.cFileName, "..") != 0) {
                if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    directories.push(path + "\\" + ffd.cFileName);
                }
                else {
                    files.push_back(path + "\\" + ffd.cFileName);
                }
            }
        } while (FindNextFile(hFind, &ffd) != 0);

        if (GetLastError() != ERROR_NO_MORE_FILES) {
            FindClose(hFind);
            return false;
        }

        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    return true;
}

inline bool CopyDirTree( vector<string>& dirlist, const string& input_dir, const string& output_dir) {
  for( unsigned int i=0; i<dirlist.size(); i++ ) {
    string toMake = dirlist[i];
    toMake.erase( 0, input_dir.size() ); 
    toMake = output_dir + toMake;
    LPSECURITY_ATTRIBUTES attr = NULL;
    CreateDirectory( toMake.c_str(), attr );
  }
  return true;
}

inline void CullNonImages( vector<string>& fn_list ) {

  for( int i=fn_list.size()-1; i>=0; i-- ) {
    string ext = fn_list[i].substr(fn_list[i].find_last_of(".") + 1);
    if( ext != "jpg" && ext != "JPG" && ext != "tif" && ext != "TIF" ) {
      fn_list.erase( fn_list.begin()+i );
    }
  }
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
  TCHAR szEXEPath[ 2048 ];
  DWORD nChars = GetModuleFileName( NULL, szEXEPath, 2048 );
  string output = szEXEPath;
  return output;
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
