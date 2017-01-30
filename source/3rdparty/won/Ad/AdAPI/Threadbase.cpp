/******************************************************************************/
/*                                                                            */
/*     !  N O T I C E  !  N O T I C E  !  N O T I C E  !  N O T I C E  !      */
/*                                                                            */
/*             ©1998 Sierra On-Line, Inc.  All Rights Reserved.               */
/*                     U.S. and foreign patents pending.                      */
/*                                                                            */
/*                          THIS SOFTWARE BELONGS TO                          */
/*                            Sierra On-Line, Inc.                            */
/*     IT IS CONSIDERED A TRADE SECRET AND IS NOT TO BE DIVULGED OR USED      */
/*        BY PARTIES WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM         */
/*                            Sierra On-Line, Inc.                            */
/*                       3380 146th Place SE, Suite 300                       */
/*                          Bellevue, WA  98007-6472                          */
/*                                206 649-9800                                */
/*                                                                            */
/*              Federal law provides severe civil penalties for               */
/*                   misuse or violation of trade secrets.                    */
/*                                                                            */
/******************************************************************************/
// Threadbase.cpp - nice encapsulation for threads
//
//////////////////////////////////////////////////////////////////////
// 1/12/98 - Initial code from Dynamix, no changes have been made

#include "Threadbase.h"
#include <process.h>

// private namespace for constants and using
namespace {
	using WONCommon::ThreadBase;

	const char* THREADNAME_DEF = "Unnamed_Thread";
};

ThreadBase::ThreadBase(const char* theName) :
	hThread(0),
	threadId(0),
	mLastError(0),
	priority(THREAD_PRIORITY_NORMAL),
	hStop(0),
	hExceptionNotify(0),
	mName(theName ? theName : THREADNAME_DEF)
{
   hStop  = CreateEvent(NULL, true, false, NULL);
}


ThreadBase::~ThreadBase()
{
   stopThread();   
   if (hStop) CloseHandle( hStop ); hStop = NULL;
}


// If compiled to use single-threaded libraries,
// startThread() will fail to link.
#ifdef _MT
void ThreadBase::startThread()
{
   if (!hThread)
   {
      ResetEvent( hStop );                      //force to non-signaled
      hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadRoutine, this, CREATE_SUSPENDED, (unsigned int*)&threadId );

      if (hThread)
      {
         SetThreadPriority( hThread, priority );
         ResumeThread( hThread );   
      }
   }
}
#endif


void ThreadBase::stopThread()
{
   if (hThread)   
   {
      SetEvent( hStop );                        //signal hStop so threadProcess can return.

      WaitForSingleObject(hThread,INFINITE);    //wait for threadProcess to sucessfully exit
      CloseHandle(hThread);                     //close the thread
      hThread  = 0;
	  threadId = 0;
   }
}


unsigned int __stdcall ThreadBase::ThreadRoutine( void* param ) 
{
	ThreadBase *pTB = (ThreadBase *)param;

	// anything not caught in the derived thread will eventually end up here.
	// this is the bottom of our threads execution stack
	int aRet = 0;
	try
	{
		aRet = pTB->threadProcess();
	}

#ifndef _NO_TOPLEVEL_CATCH
	catch (...)
	{
		// Error code is unknown, set to (-1)
		if(! pTB->mLastError) pTB->mLastError = -1;

		// Set the exception event if needed
		if (pTB->hExceptionNotify) SetEvent(pTB->hExceptionNotify);
		aRet = pTB->mLastError;
	}
#endif

	return aRet;
}


int ThreadBase::threadProcess()
{
   while (1)
   {
      //default behavior: sleep until it's time to stop the thread
      switch (WaitForSingleObject( getStopEvent(), INFINITE ))
      {
         case WAIT_OBJECT_0:  return (1);
         case WAIT_TIMEOUT:   
         default:             return (0);
      }
   }
}
