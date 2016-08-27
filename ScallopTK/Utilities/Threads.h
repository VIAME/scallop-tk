//------------------------------------------------------------------------------
// Title: Thread.h - Helper functions for threaded processing on Linux
//------------------------------------------------------------------------------

#ifndef SCALLOP_TK_THREADS_H_
#define SCALLOP_TK_THREADS_H_

// C/C++ Includes
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// OpenCV Includes
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

// Scallop Includes
#include "Definitions.h"

namespace ScallopTK
{

//------------------------------------------------------------------------------
//                             Globals and Consts
//------------------------------------------------------------------------------

const int MAX_THREADS = 64;
extern int THREADS;

//------------------------------------------------------------------------------
//                                  PThreads
//------------------------------------------------------------------------------

#ifdef USE_PTHREADS

#include <pthread.h>

// Thread Status
const int ERROR     = 0x00;
const int AVAILABLE = 0x01;
const int RUNNING   = 0x02;

// Local Globals
static int ThreadInfo[ MAX_THREADS ];

// Global Mutex Locks
static pthread_mutex_t ListLock;
static pthread_mutex_t CCLock;
static pthread_mutex_t OutputLock;
static pthread_mutex_t LockInfo[ MAX_THREADS ];

// Helper Functions
inline void threadExit() { 
  pthread_exit()( 0 ); 
}

inline void markThreadAsFinished( const int& idx ) { 
  pthread_mutex_lock(&LockInfo[idx]);
  ThreadInfo[idx] = AVAILABLE; 
  pthread_mutex_unlock(&LockInfo[idx]);
}

inline void markThreadAsRunning( const int& idx ) { 
  pthread_mutex_lock(&LockInfo[idx]);
  ThreadInfo[idx] = RUNNING; 
  pthread_mutex_unlock(&LockInfo[idx]);
}

inline void setThreadsAsAvailable() {
  for( int i=0; i < THREADS; i++ )
    ThreadInfo[i] = AVAILABLE;
}

inline bool isThreadRunning( const int& idx ) {
  pthread_mutex_lock(&LockInfo[idx]);
  bool response = (ThreadInfo[idx] == RUNNING);
  pthread_mutex_unlock(&LockInfo[idx]);
  return response;
}

inline void getDisplayLock() {
  pthread_mutex_lock(&OutputLock);
}

inline void unlockDisplay() {
  pthread_mutex_unlock(&OutputLock);
}

inline void initializeThreads() {
  pthread_mutex_init(&OutputLock, NULL);
  for( int i=0; i<THREADS; i++ )
    pthread_mutex_init(&LockInfo[i], NULL);
}

inline void killThreads() {
  pthread_mutex_destroy(&OutputLock);
  for (int i=0; i<THREADS; i++ )
    pthread_mutex_destroy(&LockInfo[i]);
}

inline void getListLock() {
  pthread_mutex_lock(&ListLock);
}

inline void unlockList() {
  pthread_mutex_unlock(&ListLock);
}

#else

//------------------------------------------------------------------------------
//                   No Threading Enabled Empty Prototypes
//------------------------------------------------------------------------------

inline void initializeThreads() {}
inline void threadExit() {}
inline void markThreadAsFinished( const int& idx ) {}
inline void markThreadAsRunning( const int& idx ) {}
inline void halt_until_finished() {}
inline void getDisplayLock() {}
inline void unlockDisplay() {}
inline void getListLock() {}
inline void unlockList() {}

#endif

}

#endif
