#pragma once

#include "LFX_File.h"

namespace LFX {

#define LOGC_INFO 0
#define LOGC_DEBUG 1
#define LOGC_WARN 2
#define LOGC_ERROR 3

	class Log : public Singleton<Log>
	{
	public:
		Log(const char* filename, const char* mode = "w");
		~Log();

		int
			Format(char* str, const char* format, va_list args);
		void
			Print(int channel, const char * text);
		void
			Printf(int channel, const char* format, ...);
		static void
			PrintTime(FILE* fp, bool date);

	protected:
		FILE* mFile;
	};
}

