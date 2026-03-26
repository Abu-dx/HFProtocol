// XThread.h: interface for the CXThread class.


#pragma once
#include <afxmt.h>
class  CXThread  
{
public:

	CXThread();
	virtual ~CXThread();
	//Start the thread. Thread will begin it's 
	//exection in Run().
	BOOL Start(void* param=NULL);
	BOOL StartSave(void* param=NULL);
	//Stop the thread. Call this to make ShouldRun() return
	//false. Call Stop(true) to wait until the thread is stopeed.
	void Stop(bool waitThreadStop=false);
	void StopSave(bool waitThreadStop=false);

	//Check if the thread is running.
	bool IsRunning() const;
	bool IsSaveRunning() const;

	//Wait for the thread to stop.
	void WaitForStop();
	void WaitForSaveStop();
	//Check if the thread should continue it's exection. To stop
	//the thread, call Stop().
	bool ShouldRun() const;

	//Get the thread.
	CWinThread* GetThread() const;
	CWinThread* GetSaveThread() const;

protected:
	CEvent m_gSave;
	//This is the function where the thread executes.
	//Derive a class from CThread and implement this function.
	//param is the same that is used in Start
	virtual void Run(void* param)=0;
	virtual void RunSave(void* param) = 0;
	//Is true if the thread is running.
	bool m_runThread;
	bool m_runThreadSave;

private:

	//Set variables so no thread is running.
	void ThreadStopped();
	void ThreadSaveStopped();
	//Static function to start the thread.
	static UINT StartThread( LPVOID pParam );
	static UINT StartSaveThread( LPVOID pParam );
	//Set which thread that is using.
	void SetThread(CWinThread* thread);
	void SetSaveThread(CWinThread* thread);

	//The thread that is executing.
	CWinThread* m_thread;
	CWinThread* m_threadSave;

	struct ThreadStartInfo
	{
		CXThread* m_thread;
		void* m_param;
	} m_startUpInfo,m_startUpSaveInfo;

	//Handle and ID to thread.
	HANDLE m_threadHandle;
	HANDLE m_threadSaveHandle;
	DWORD m_threadId;
	DWORD m_threadSaveId;
};
