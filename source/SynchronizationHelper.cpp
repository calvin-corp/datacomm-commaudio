#include "SynchronizationHelper.h"

///////////////////////////
//  forward declarations //
///////////////////////////

static DWORD WINAPI delayedSetEventRoutine(LPVOID params);
static HANDLE delayedSetEventAccess = CreateMutex(NULL, FALSE, NULL);;

/**
 * structure passed to the thread as thread running delayedSetEventRoutine
 *   as parameters.
 */
struct DelayedSetEventParams
{
    HANDLE event;
    long milliseconds;
};

typedef struct DelayedSetEventParams DelayedSetEventParams;

//////////////////////////////
// function implementations //
//////////////////////////////

/**
 * sets an {event} milliseconds {milliseconds} in the future.
 *
 * @function     delayedSetEvent
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
 * @signature    void delayedSetEvent(HANDLE event, long milliseconds)
 *
 * @param        event   event to set in the future
 * @param        milliseconds   milliseconds to wait before setting the
 *   parameter.
 */
void delayedSetEvent(HANDLE event, long milliseconds)
{
    // obtain synchronization objects
    WaitForSingleObject(delayedSetEventAccess,INFINITE);

    // prepare thread parameters
    DelayedSetEventParams* params;
    params = (DelayedSetEventParams*) malloc(sizeof(DelayedSetEventParams));
    params->event        = event;
    params->milliseconds = milliseconds;

    // create the thread
    DWORD useless;
    CreateThread(0,0,delayedSetEventRoutine,params,0,&useless);

    // release synchronization objects
    ReleaseMutex(delayedSetEventAccess);
}

/**
 * thread that gets created from the delayedSetEvent function. it does the
 *   actual waiting, then setting of the passed event object.
 *
 * @function     delayedSetEventRoutine
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
 * @signature    static DWORD WINAPI delayedSetEventRoutine(LPVOID params)
 *
 * @param        params   pointer to a DelayedSetEventParams structure
 *
 * @return       exit code
 */
static DWORD WINAPI delayedSetEventRoutine(LPVOID params)
{
    // parse thread parameters
    DelayedSetEventParams* p = (DelayedSetEventParams*) params;

    // wait for the specified time, then set the event
    Sleep(p->milliseconds);
    SetEvent(p->event);

    // free the malloced parameters & return
    free(p);
    return 0;
}
