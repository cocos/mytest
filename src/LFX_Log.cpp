#include "LFX_Log.h"
#include <time.h>

namespace LFX {

	ImplementSingleton(Log);

	Log::Log(const char * filename, const char * mode)
		: mFile(NULL)
	{
		if (filename != NULL) {
			mFile = fopen(filename, mode);
			if (mFile == NULL) {
				char msg[256];
				sprintf(msg, "?: log file '%s' open failed", filename);

				Print(LOGC_ERROR, msg);
			}
		}
	}

	Log::~Log()
	{
		if (mFile) {
			fclose(mFile);
		}
	}

	int Log::Format(char* str, const char* format, va_list args)
	{
		return vsprintf(str, format, args);
	}

	void Log::Print(int channel, const char * text)
	{
		if (mFile) {
			PrintTime(mFile, true);
			fprintf(mFile, text);
			fprintf(mFile, "\n");
#ifdef _DEBUG
			fflush(mFile);
#endif
		}
	}

	void Log::Printf(int channel, const char* format, ...)
	{
		char str[64 * 1024];

		va_list arglist;
		va_start(arglist, format);
		Format(str, format, arglist);
		va_end(arglist);

		Print(channel, str);
	}

	void Log::PrintTime(FILE* fp, bool date)
	{
		//       YYYY   year
		//       MM     month (2 digits 01-12)
		//       DD     day (2 digits 01-31)
		//       HH     hour (2 digits 00-23)
		//       MM     minutes (2 digits 00-59)
		//       SS     seconds (2 digits 00-59)
		time_t t = time(NULL);
		tm* aTm = localtime(&t);

		if (fp) {
			if (date) {
				fprintf(fp, "%-4d-%02d-%02d %02d:%02d:%02d ", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
			}
			else {
				fprintf(fp, "%02d:%02d:%02d ", aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
			}
		}
		else {
			if (date) {
				printf("%-4d-%02d-%02d %02d:%02d:%02d ", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
			}
			else {
				printf("%02d:%02d:%02d ", aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
			}
		}
	}

}
