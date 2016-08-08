//------------------------------------------------------------------------------
// Title: Thread.h - Helper functions for threaded processing on Linux
//------------------------------------------------------------------------------

#ifndef SCALLOP_TK_THREAD_H
#define SCALLOP_TK_THREAD_H

// C/C++ Includes
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// OpenCV Includes
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

// Scallop Includes
#include "Definitions.h"

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
inline void thread_exit() { 
  pthread_exit( 0 ); 
}

inline void mark_thread_as_finished( const int& idx ) { 
  pthread_mutex_lock(&LockInfo[idx]);
  ThreadInfo[idx] = AVAILABLE; 
  pthread_mutex_unlock(&LockInfo[idx]);
}

inline void mark_thread_as_running( const int& idx ) { 
  pthread_mutex_lock(&LockInfo[idx]);
  ThreadInfo[idx] = RUNNING; 
  pthread_mutex_unlock(&LockInfo[idx]);
}

inline void set_threads_as_available() {
  for( int i=0; i < THREADS; i++ )
    ThreadInfo[i] = AVAILABLE;
}

inline bool is_thread_running( const int& idx ) {
  pthread_mutex_lock(&LockInfo[idx]);
  bool response = (ThreadInfo[idx] == RUNNING);
  pthread_mutex_unlock(&LockInfo[idx]);
  return response;
}

inline void get_display_lock() {
  pthread_mutex_lock(&OutputLock);
}

inline void unlock_display() {
  pthread_mutex_unlock(&OutputLock);
}

inline void initialize_threads() {
  pthread_mutex_init(&OutputLock, NULL);
  for( int i=0; i<THREADS; i++ )
    pthread_mutex_init(&LockInfo[i], NULL);
}

inline void kill_threads() {
  pthread_mutex_destroy(&OutputLock);
  for (int i=0; i<THREADS; i++ )
    pthread_mutex_destroy(&LockInfo[i]);
}

inline void get_ListLock() {
  pthread_mutex_lock(&ListLock);
}

inline void unlock_list() {
  pthread_mutex_unlock(&ListLock);
}

#else

//------------------------------------------------------------------------------
//                   No Threading Enabled Empty Prototypes
//------------------------------------------------------------------------------

inline void initialize_threads() {}
inline void thread_exit() {}
inline void mark_thread_as_finished( const int& idx ) {}
inline void mark_thread_as_running( const int& idx ) {}
inline void halt_until_finished() {}
inline void get_display_lock() {}
inline void unlock_display() {}
inline void get_ListLock() {}
inline void unlock_list() {}

#endif

#endif
