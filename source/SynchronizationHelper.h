/**
 * contains function declarations that are implements in the
 *   SynchronizationHelper.cpp file.
 *
 * @sourceFile   SynchronizationHelper.h
 *
 * @program      commaudio.exe
 *
 * @function     void delayedSetEvent(HANDLE event, long milliseconds);
 *
 * @date         2015-04-09
 *
 * @revision     none
 *
 * @designer     Eric Tsang
 *
 * @programmer   Eric Tsang
 *
 * @note         none
 */
#ifndef _SYNCHRONIZATION_HELPER_H_
#define _SYNCHRONIZATION_HELPER_H_

#include "common.h"

void delayedSetEvent(HANDLE event, long milliseconds);

#endif
