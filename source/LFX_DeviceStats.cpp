#include "LFX_DeviceStats.h"

#ifndef _WIN32
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

namespace LFX {

	DeviceStats DeviceStats::GetStats()
	{
		DeviceStats stats;

#ifdef _WIN32
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		stats.Processors = sysInfo.dwNumberOfProcessors;
#else
		int count = 1;
		size_t size = sizeof(int);
		sysctlbyname("hw.ncpu", &count, &size, nullptr, 0);
		stats.Processors = std::max(count, 1);
#endif

		return stats;
	}

}