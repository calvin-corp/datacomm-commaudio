/**
 * contains implementations of functions declared in handlerHelper.h
 *
 * @sourceFile   handlerHelper.cpp
 *
 * @program      commaudio
 *
 * @function     fatalError
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
#include "handlerHelper.h"
#include "common.h"

#include <stdio.h>

/**
 * prints the error message, sleeps for a little while, then exits the program.
 *
 * @function     fatalError
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
 *
 * @signature    void fatalError(char* errstr)
 *
 * @param        errstr   string to print before exiting the program.
 */
void fatalError(char* errstr)
{
    printf("%s: %d\n",errstr,GetLastError());
    perror(errstr);
    Sleep(5000);
    exit(0);
}
