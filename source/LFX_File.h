#pragma once

#include "LFX_Types.h"

namespace LFX {

	class LFX_ENTRY FileUtil
	{
	public:
		static String GetDirectory(const String & file);
		static bool Exist(const String & file);
		static void MakeDir(const String & dir);
		static bool CreateDir(const String & dir);
		static bool DeleteDir(const String & dir);
	};

}