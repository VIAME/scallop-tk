//------------------------------------------------------------------------------
// Title: thread.h
// Author: Matthew Dawkins
//------------------------------------------------------------------------------

#ifndef THREAD_STUFF_H
#define THREAD_STUFF_H

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
//                                  PTHREADS
//------------------------------------------------------------------------------

#ifdef USE_PTHREADS

#include <pthread.h>

// Thread Status
const int ERROR = 0x00;
const int AVAILABLE = 0x01;
const int RUNNING = 0x02;

// Local Globals (Bad practice, but quick)
static int thread_info[ MAX_THREADS ];

// Mutex Locks
static pthread_mutex_t list_lock;
static pthread_mutex_t cc_lock;
static pthread_mutex_t out_lock;
static pthread_mutex_t info_locks[ MAX_THREADS ];

// Functions
inline void thread_exit() { 
	pthread_exit( 0 ); 
}

inline void mark_thread_as_finished( const int& idx ) { 
	pthread_mutex_lock(&info_locks[idx]);
	thread_info[idx] = AVAILABLE; 
	pthread_mutex_unlock(&info_locks[idx]);
}

inline void mark_thread_as_running( const int& idx ) { 
	pthread_mutex_lock(&info_locks[idx]);
	thread_info[idx] = RUNNING; 
	pthread_mutex_unlock(&info_locks[idx]);
}

inline void set_threads_as_available() {
	for( int i=0; i < THREADS; i++ )
		thread_info[i] = AVAILABLE;
}

inline bool is_thread_running( const int& idx ) {
	pthread_mutex_lock(&info_locks[idx]);
	bool response = (thread_info[idx] == RUNNING);
	pthread_mutex_unlock(&info_locks[idx]);
	return response;
}

inline void get_display_lock() {
	pthread_mutex_lock(&out_lock);
}

inline void unlock_display() {
	pthread_mutex_unlock(&out_lock);
}

inline void initialize_threads() {
	pthread_mutex_init(&out_lock, NULL);
	for( int i=0; i<THREADS; i++ )
		pthread_mutex_init(&info_locks[i], NULL);
}

inline void kill_threads() {
	pthread_mutex_destroy(&out_lock);
	for (int i=0; i<THREADS; i++ )
		pthread_mutex_destroy(&info_locks[i]);
}

inline void get_list_lock() {
	pthread_mutex_lock(&list_lock);
}

inline void unlock_list() {
	pthread_mutex_unlock(&list_lock);
}

#else

//------------------------------------------------------------------------------
//                           No Threading Enabled
//------------------------------------------------------------------------------

inline void initialize_threads() {}
inline void thread_exit() {}
inline void mark_thread_as_finished( const int& idx ) {}
inline void mark_thread_as_running( const int& idx ) {}
inline void halt_until_finished() {}
inline void get_display_lock() {}
inline void unlock_display() {}
inline void get_list_lock() {}
inline void unlock_list() {}

#endif

#endif
