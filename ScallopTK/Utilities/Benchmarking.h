
#ifndef SCALLOP_TK_TIMERS_H
#define SCALLOP_TK_TIMERS_H

// C/C++ Includes
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// For Windows
#ifdef WIN32 

#include <windows.h>
#include <stdio.h>

static LARGE_INTEGER frequency;
static LARGE_INTEGER t1, t2;
static double LAST_TIME;


inline void initializeTimer() {
	QueryPerformanceFrequency(&frequency);	
}

inline double getTimeElapsed() {
	QueryPerformanceCounter(&t2);
	return (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
}

inline void startTimer() {
	QueryPerformanceCounter(&t1);
	LAST_TIME = getTimeElapsed();
}

inline double getTimeSinceLastCall() {
	double newTime = getTimeElapsed();
	double passedSinceLastCall = newTime - LAST_TIME;
	LAST_TIME = newTime;
	return passedSinceLastCall;
}

// For Unix
#else 

#include <sys/time.h>
using namespace std;

static timeval t1, t2;
static double LAST_TIME;

inline void initializeTimer() {
		
}

inline double getTimeElapsed() {
	gettimeofday(&t2, NULL);
	double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
	return elapsedTime;
}

inline void startTimer() {
	gettimeofday(&t1, NULL);
	LAST_TIME = getTimeElapsed();
}

inline double getTimeSinceLastCall() {
	double newTime = getTimeElapsed();
	double passedSinceLastCall = newTime - LAST_TIME;
	LAST_TIME = newTime;
	return passedSinceLastCall;
}

#endif

#endif
