
// XThread.cpp: implementation of the CXThread class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "stdafx.h"
#include "XThread.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXThread::CXThread()
{
	m_startUpInfo.m_param = NULL;
	m_startUpInfo.m_thread = NULL;
	m_startUpSaveInfo.m_param = NULL;
	m_startUpSaveInfo.m_thread = NULL;


	ThreadStopped();
	ThreadSaveStopped();
}

CXThread::~CXThread()
{
	if(IsRunning())
	{
		TRACE(_T("CThread: Warning: Stopping thread in CThread destructor\n"));
		Stop(true);
	}
	if(IsSaveRunning())
	{
		TRACE(_T("CThread: Warning: Stopping thread in CThread destructor\n"));
		Stop(true);
	}
}

BOOL CXThread::Start(void* param)
{
	//Make sure that no thread is currently running
	//ASSERT(IsRunning() == false);
	if(IsRunning() == true)
		return false;

	//Setup thread startup info
	m_startUpInfo.m_thread = this;
	m_startUpInfo.m_param = param;

	//Start the thread. It must starts it execution in
	//a static function.

	SetThread( AfxBeginThread(StartThread, (LPVOID) &m_startUpInfo));

	return true;
}

BOOL CXThread::StartSave(void* param)
{
	//Make sure that no thread is currently running
	//ASSERT(IsRunning() == false);
	if(IsSaveRunning() == true)
		return false;

	//Setup thread startup info
	m_startUpSaveInfo.m_thread = this;
	m_startUpSaveInfo.m_param = param;

	//Start the thread. It must starts it execution in
	//a static function.

	SetThread( AfxBeginThread(StartSaveThread, (LPVOID) &m_startUpSaveInfo));
	//SetEvent()
	return true;
}
UINT CXThread::StartSaveThread( LPVOID pParam )
{
	ThreadStartInfo* info = (ThreadStartInfo*) pParam;

	//The start parameter is a pointer to a CThread object.
	CXThread* thread = info->m_thread;
	thread->m_runThreadSave = true;

	//Copy handle and ID. Necessary if the thread crash,
	//and WaitForStop is called.
	CWinThread* winthread = AfxGetThread();
	thread->m_threadSaveHandle = winthread->m_hThread;
	thread->m_threadSaveId = winthread->m_nThreadID;


	//Run the thread, and stop it.
	TRACE(_T("CXThread: Starting thread 0x%X\n"), winthread->m_nThreadID);
	try
	{
		thread->RunSave(info->m_param);
	}
	catch(...)
	{
		TRACE(_T("CXThread: Warning. Run() thrown an exception. Thread will terminate.\n"));
	}
	thread->ThreadSaveStopped();
	//	TRACE(_T("CXThread: Exiting thread 0x%X\n"), winthread->m_nThreadID);

	return 0;

}
UINT CXThread::StartThread( LPVOID pParam )
{
	ThreadStartInfo* info = (ThreadStartInfo*) pParam;

	//The start parameter is a pointer to a CThread object.
	CXThread* thread = info->m_thread;
	thread->m_runThread = true;

	//Copy handle and ID. Necessary if the thread crash,
	//and WaitForStop is called.
	CWinThread* winthread = AfxGetThread();
	thread->m_threadHandle = winthread->m_hThread;
	thread->m_threadId = winthread->m_nThreadID;


	//Run the thread, and stop it.
	TRACE(_T("CXThread: Starting thread 0x%X\n"), winthread->m_nThreadID);
	try
	{
		thread->Run(info->m_param);
	}
	catch(...)
	{
		TRACE(_T("CXThread: Warning. Run() thrown an exception. Thread will terminate.\n"));
	}
	thread->ThreadStopped();
	//	TRACE(_T("CXThread: Exiting thread 0x%X\n"), winthread->m_nThreadID);

	return 0;
}

void CXThread::ThreadStopped()
{
	//Reset variables.
	m_runThread = false;

	m_threadHandle = NULL;
	m_threadId = 0;
	SetThread(NULL);
}
void CXThread::ThreadSaveStopped()
{
	//Reset variables.
	m_runThreadSave = false;

	m_threadSaveHandle = NULL;
	m_threadSaveId = 0;
	SetSaveThread(NULL);
}

void CXThread::Stop(bool waitThreadStop)
{	
	if(IsRunning())
	{
		TRACE(_T("CXThread: Stopping thread 0x%X...\n"), GetThread()->m_nThreadID);

		//Stop the thread. The exection is NOT stopped immiedatly,
		//instead m_runThread is set to false. You should call
		//ShouldRun() in you Run() to check this varaible.
		m_runThread=false;

		//Should we wait for the thread to stop?
		if(waitThreadStop)
			WaitForStop();
	}

}
void CXThread::StopSave(bool waitThreadStop)
{	
	if(IsSaveRunning())
	{
		TRACE(_T("CXThread: Stopping thread 0x%X...\n"), GetThread()->m_nThreadID);

		//Stop the thread. The exection is NOT stopped immiedatly,
		//instead m_runThread is set to false. You should call
		//ShouldRun() in you Run() to check this varaible.
		m_runThreadSave=false;

		//Should we wait for the thread to stop?
		if(waitThreadStop)
			WaitForSaveStop();
	}

}

bool CXThread::ShouldRun() const
{
	//Check if the thread should run.
	return m_runThread;
}

void CXThread::WaitForStop()
{
	//Don't call WaitForStop from the thread that running in Run
	//(that will make the thread waiting on it self to exit - not good).
	ASSERT( AfxGetThread() != GetThread() );

	//Wait for the thread to stop if the thread is running.
	if(m_threadHandle != NULL)
	{
		TRACE(_T("CThread: Waiting on thread 0x%X to exit...\n"), m_threadId);

		WaitForSingleObject(m_threadHandle, INFINITE);
	}

}
void CXThread::WaitForSaveStop()
{
	//Don't call WaitForStop from the thread that running in Run
	//(that will make the thread waiting on it self to exit - not good).
	ASSERT( AfxGetThread() != GetThread() );

	//Wait for the thread to stop if the thread is running.
	if(m_threadSaveHandle != NULL)
	{
		TRACE(_T("CThread: Waiting on thread 0x%X to exit...\n"), m_threadId);

		WaitForSingleObject(m_threadSaveHandle, INFINITE);
	}

}

bool CXThread::IsRunning() const
{
	//Check if the thread is running.
	return m_thread != NULL;
}
bool CXThread::IsSaveRunning() const
{
	//Check if the thread is running.
	return m_threadSave != NULL;
}

CWinThread* CXThread::GetThread() const
{
	//Get the thread. It's necessary to make sure that m_thread
	//isn't changed while doing this.
	CCriticalSection cs;
	cs.Lock();

	CWinThread* ret = m_thread;

	cs.Unlock();

	return ret;
}

CWinThread* CXThread::GetSaveThread() const
{
	//Get the thread. It's necessary to make sure that m_thread
	//isn't changed while doing this.
	CCriticalSection cs;
	cs.Lock();

	CWinThread* ret = m_threadSave;

	cs.Unlock();

	return ret;
}

void CXThread::SetThread(CWinThread* thread)
{
	//Get the thread. It's necessary to make sure that m_thread
	//isn't changed while doing this.	
	CCriticalSection cs;
	cs.Lock();

	m_thread = thread;

	cs.Unlock();
}
void CXThread::SetSaveThread(CWinThread* thread)
{
	//Get the thread. It's necessary to make sure that m_thread
	//isn't changed while doing this.	
	CCriticalSection cs;
	cs.Lock();

	m_threadSave = thread;

	cs.Unlock();
}


