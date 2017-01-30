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

#include "won.h"
#include "Threadbase.h"
#include "WONException.h"
#include "EventLog.h"
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
	hRehup(0),
	hExceptionNotify(0),
	mName(theName ? theName : THREADNAME_DEF)
{
   hStop  = CreateEvent(NULL, true, false, NULL);
   hRehup = CreateEvent(NULL, false, false, NULL);
}


ThreadBase::~ThreadBase()
{
   stopThread();   
   if (hRehup) CloseHandle( hRehup );
   if (hStop) CloseHandle( hStop );
}


// If compiled to use single-threaded libraries,
// startThread() will fail to link.
#ifdef _MT
void ThreadBase::startThread()
{
   WTRACE("ThreadBase::startThread");
   if (!hThread)
   {
      WDBG_LM("ThreadBase::startThread Creating thread");
      ResetEvent( hStop );                      //force to non-signaled
	  ResetEvent( hRehup );
      hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadRoutine, this, CREATE_SUSPENDED, (unsigned int*)&threadId );

      if (hThread)
      {
         WDBG_LL("ThreadBase::startThread Set thread priority and resume thread");
         SetThreadPriority( hThread, priority );
         ResumeThread( hThread );   
      }
   }
}
#endif


void ThreadBase::stopThread()
{
   WTRACE("ThreadBase::stopThread");
   if (hThread)   
   {
      WDBG_LM("ThreadBase::stopThread Stopping the thread");
      SetEvent( hStop );                        //signal hStop so threadProcess can return.
	  SetEvent( hRehup );						//rehup the thread in case it's blocking for event signals

      WDBG_LL("ThreadBase::stopThread Waiting for thread to exit");
      WaitForSingleObject(hThread,INFINITE);    //wait for threadProcess to sucessfully exit
      CloseHandle(hThread);                     //close the thread
      hThread  = 0;
	  threadId = 0;
   }
}


unsigned int __stdcall ThreadBase::ThreadRoutine( void* param ) 
{
	ThreadBase *pTB = (ThreadBase *)param;

	WDBG_THREADSTART(pTB->mName.c_str());
	WTRACE("ThreadBase::ThreadRoutine");
   
	// anything not caught in the derived thread will eventually end up here.
	// this is the bottom of our threads execution stack
	int aRet = 0;
	try
	{
		aRet = pTB->threadProcess();
	}
	catch (WONCommon::WONException& ex)
	{
		WDBG_AH("WONException caught in ThreadRoutine!");

		// set error code
		// WONException may set this to zero if undefined, so reset to -1 in that case
		pTB->mLastError = (ex.GetCode() != 0 ? ex.GetCode() : -1); 
		ex.GetStream() << "Exception caught in Thread main.  Thread terminated!";

		// Set the exception event if needed
		if (pTB->hExceptionNotify) SetEvent(pTB->hExceptionNotify);
		aRet = pTB->mLastError;
	}

#ifndef _NO_TOPLEVEL_CATCH
	catch (...)
	{
		WDBG_AH("Unhandled exception caught in ThreadRoutine!");

		// Error code is unknown, set to (-1)
		if(! pTB->mLastError) pTB->mLastError = -1;

		// Log an error to event log
		WONCommon::EventLog aLog;
		aLog.Log("Unhandled Exception propogated to Thread main.  Thread terminated!");

		// Set the exception event if needed
		if (pTB->hExceptionNotify) SetEvent(pTB->hExceptionNotify);
		aRet = pTB->mLastError;
	}
#endif

	WDBG_THREADSTOP;
	return aRet;
}


int ThreadBase::threadProcess()
{
   WTRACE("ThreadBase::threadProcess");
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
