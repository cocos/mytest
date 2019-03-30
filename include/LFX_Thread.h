#pragma once

#include "LFX_Types.h"
#ifndef _WIN32
#include <pthread.h>
#endif

namespace LFX {

	class Mutex
	{
#ifdef _WIN32
		CRITICAL_SECTION mSection;

	public:
		Mutex()
		{
			InitializeCriticalSection(&mSection);
		}

		~Mutex()
		{
			DeleteCriticalSection(&mSection);
		}

		void Lock()
		{
			EnterCriticalSection(&mSection);
		}

		void Unlock()
		{
			LeaveCriticalSection(&mSection);
		}

#else
		pthread_mutex_t mSection;

	public:
		Mutex()
		{
			pthread_mutex_init(&mSection, NULL);
		}

		~Mutex()
		{
			pthread_mutex_destroy(&mSection);
		}

		void Lock()
		{
			pthread_mutex_lock(&mSection);
		}

		void Unlock()
		{
			pthread_mutex_unlock(&mSection);
		}

#endif
	};

	//
	class LFX_ENTRY Thread
	{
	public:
#ifdef _WIN32
		typedef HANDLE THANDLE;
#else
		typedef pthread_t THANDLE;
#endif

		enum STATUS {
			RUN,
			SUSPEND,
			STOP,
		};

	public:
		static void Sleep(float second);

	public:
		Thread();
		virtual ~Thread();

		STATUS GetStatus() { return mStatus; }

		virtual void Start();
		virtual void Stop();

		virtual void Run() = 0;

	protected:
		THANDLE mHandle;
		STATUS mStatus;
	};

}
