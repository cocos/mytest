#include "LFX_File.h"

#include "dirent.h"
#include <io.h>

namespace LFX {

	String FileUtil::GetDirectory(const String & file)
	{
		const char * str = file.c_str();
		int len = file.length();

		while (len > 0 && (str[len - 1] != '\\' && str[len - 1] != '/'))
			--len;

		char dir[256];

		strcpy(dir, file.c_str());
		dir[len] = 0;
		if (len > 0)
		{
			dir[len - 1] = 0;
		}

		return dir;
	}

	bool FileUtil::Exist(const String & file)
	{
		return access(file.c_str(), 0) != -1;
	}

	void FileUtil::MakeDir(const String & filedir)
	{
		String dir = filedir;

		if (dir == "")
			return;

		if (!FileUtil::Exist(dir))
		{
			String _dir = FileUtil::GetDirectory(dir);

			MakeDir(_dir.c_str());

			FileUtil::CreateDir(dir);
		}
	}

	bool FileUtil::CreateDir(const String & dir)
	{
#ifdef _WIN32
		SECURITY_ATTRIBUTES attribute;
		attribute.nLength = sizeof(attribute);
		attribute.lpSecurityDescriptor = NULL;
		attribute.bInheritHandle = FALSE;

		return CreateDirectory(dir.c_str(), &attribute) == TRUE;
#else
		return mkdir(dir.c_str(), 777) != -1;
#endif
	}

	bool FileUtil::DeleteDir(const String & dir)
	{
		const char * dirName = dir.c_str();
		DIR* dp = NULL;
		DIR* dpin = NULL;
		char pathname[256];
		struct dirent* dirp;

		dp = opendir(dirName);
		if (dp == NULL)
			return false;

		while ((dirp = readdir(dp)) != NULL)
		{
			if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0)
				continue;

			strcpy(pathname, dirName);
			strcat(pathname, "/");
			strcat(pathname, dirp->d_name);

			dpin = opendir(pathname);
			if (dpin != NULL)
			{
				closedir(dpin);
				dpin = NULL;
				DeleteDir(pathname);
			}
			else
			{
				remove(pathname);
			}
		}

#ifdef _WIN32
		RemoveDirectory(dirName);
#else
		rmdir(dirName);
#endif

		closedir(dp);
		dirp = NULL;

		return true;
	}


}