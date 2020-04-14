#include "LFX_Thread.h"
#ifndef _WIN32
#include <unistd.h>
#endif

namespace LFX {

	void Thread::Sleep(float second)
	{
		if (second > 0)
		{
#ifdef _WIN32
			::Sleep((uint32_t)(second * 1000));
#else
			usleep((unsigned long)(second * 1000) * 1000);
#endif
		}
	}

	//
#ifdef _WIN32

	DWORD WINAPI Thread_Shared_Proc(LPVOID param)
	{
		Thread * p = (Thread *)param;

		p->Run();

		return 0;
	}

#else

	void * Thread_Shared_Proc(void * param)
	{
		Thread * p = (Thread *)param;

		p->Run();

		pthread_exit((void *)0);
	}

#endif

	Thread::Thread()
	{
		mHandle = 0;
		mStatus = STOP;
	}

	Thread::~Thread()
	{
		Stop();
	}

	void Thread::Start()
	{
		assert(mHandle == NULL);

		mStatus = RUNNING;
#ifdef _WIN32
		mHandle = CreateThread(NULL, 0, Thread_Shared_Proc, (void *)this, 0, NULL);
#else
		pthread_create(&mHandle, NULL, Thread_Shared_Proc, (void *)this);
#endif
	}

	void Thread::Stop()
	{
		if (mHandle != 0)
		{
			mStatus = STOP;

#ifdef _WIN32
			WaitForSingleObject(mHandle, INFINITE);

			TerminateThread(mHandle, 0);
			CloseHandle(mHandle);
#else
			pthread_join(mHandle, NULL);
#endif
			mHandle = 0;
		}
	}

}