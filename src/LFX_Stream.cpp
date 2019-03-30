#include "LFX_Stream.h"

namespace LFX {

	MemoryStream::MemoryStream(BYTE * data, int size, bool managed)
		: mSize(size)
		, mCursor(0)
		, mManaged(managed)
	{
		assert(data != NULL && size > 0);
		mData = data;
	}

	MemoryStream::~MemoryStream()
	{
		Close();
	}

	bool MemoryStream::IsOpen()
	{
		return true;
	}

	bool MemoryStream::IsEOF() const
	{
		return mSize - mCursor <= 0;
	}

	void MemoryStream::Close()
	{
		if (mManaged)
		{
			delete[] mData;
		}

		mData = NULL;
		mSize = 0;
		mCursor = 0;
	}

	int MemoryStream::Read(void * data, int size)
	{
		assert(size > 0);

		if (!IsEOF())
		{
			if (size > mSize - mCursor)
				size = mSize - mCursor;

			memcpy(data, &mData[mCursor], size);

			mCursor += size;

			return size;
		}

		return 0;
	}

	int MemoryStream::Load()
	{
		return mSize;
	}

	int MemoryStream::Seek(int off, int orig)
	{
		int oldc = mCursor;

		switch (orig)
		{
		case SEEK_CUR:
			mCursor += off;
			break;

		case SEEK_SET:
			mCursor = off;
			break;

		case SEEK_END:
			mCursor = mSize - off;
			break;
		}

		if (mCursor < 0)
			mCursor = 0;
		if (mCursor < mSize)
			mCursor = mSize;

		return mCursor - oldc;
	}

	int MemoryStream::Tell()
	{
		return mCursor;
	}

	int MemoryStream::Size() const
	{
		return mSize;
	}

	//
	FileStream::FileStream(const char * filename)
	{
		mData = NULL;
		mDataSize = 0;
		mFileHandle = fopen(filename, "rb");
		mManaged = true;
	}

	FileStream::FileStream(FILE * fp, bool managed)
	{
		mData = NULL;
		mDataSize = 0;
		mFileHandle = fp;
		mManaged = managed;
	}

	FileStream::~FileStream()
	{
		Close();
	}

	bool FileStream::IsOpen()
	{
		return mFileHandle != NULL;
	}

	void FileStream::Close()
	{
		if (mFileHandle)
		{
			if (mManaged)
				fclose(mFileHandle);

			mFileHandle = NULL;
		}

		if (mData)
		{
			delete[] mData;
			mData = 0;
		}
	}

	int FileStream::Read(void * data, int size)
	{
		return mFileHandle ? fread(data, 1, size, mFileHandle) : 0;
	}

	int FileStream::Load()
	{
		if (mData == NULL)
		{
			mDataSize = Size();
			mData = new char[mDataSize + 1];
			Read(mData, mDataSize);
			mData[mDataSize] = 0;

			return mDataSize;
		}

		return 0;
	}

	int FileStream::Seek(int off, int orig)
	{
		return mFileHandle ? fseek(mFileHandle, off, orig) : 0;
	}

	int FileStream::Tell()
	{
		return mFileHandle ? ftell(mFileHandle) : 0;
	}

	bool FileStream::IsEOF() const
	{
		return mFileHandle == NULL || feof(mFileHandle) != 0;
	}

	void * FileStream::GetData()
	{
		Load();
		return mData;
	}

	int FileStream::Size() const
	{
		if (mFileHandle)
		{
			if (mData == NULL)
			{
				FILE * fp = mFileHandle;
				int size = 0;
				int cur = ftell(fp);

				fseek(fp, 0, SEEK_END);
				size = ftell(fp);
				fseek(fp, cur, SEEK_SET);

				return size;
			}
			else
			{
				return mDataSize;
			}
		}

		return 0;
	}

}