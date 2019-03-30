#pragma once

#include "LFX_File.h"

namespace LFX {

	class LFX_ENTRY Stream
	{
	public:
		Stream() {}
		virtual ~Stream() {}

		virtual bool IsOpen() = 0;
		virtual bool IsEOF() const = 0;
		virtual void Close() = 0;

		virtual int Read(void * data, int size) = 0;

		virtual int Load() = 0;
		virtual int Seek(int off, int orig) = 0;
		virtual int Tell() = 0;
		virtual int Size() const = 0;

		virtual void * GetData() = 0;

		template<class T>
		int ReadT(T & data) { return Read(&data, sizeof(T)); }
		template<class T>
		T ReadT() { T data; Read(&data, sizeof(T)); return data;  }
		String ReadString();

		template <class T> Stream & operator >>(T & data) { ReadT(data); return *this; }
	};

	inline String Stream::ReadString()
	{
		int length = 0;
		*this >> length;

		char text[2048];
		Read(text, length);
		text[length] = 0;

		return text;
	}

	//
	class LFX_ENTRY MemoryStream : public Stream
	{
	public:
		MemoryStream(BYTE * data, int size, bool managed);
		virtual ~MemoryStream();

		virtual bool IsOpen();
		virtual bool IsEOF() const;
		virtual void Close();

		virtual int Read(void * data, int size);

		virtual int Load();
		virtual int Seek(int off, int orig);
		virtual int Tell();
		virtual int Size() const;

		virtual void * GetData() { return mData; }

	protected:
		BYTE * mData;
		int mSize;
		int mCursor;
		bool mManaged;
	};

	//
	class LFX_ENTRY FileStream : public Stream
	{
	public:
		FileStream(const char * filename);
		FileStream(FILE * fp, bool managed);
		virtual ~FileStream();

		virtual bool
			IsOpen();
		virtual bool
			IsEOF() const;
		virtual void
			Close();

		virtual int
			Read(void * data, int size);

		virtual int
			Load();
		virtual int
			Seek(int off, int orig);
		virtual int
			Tell();
		virtual int
			Size() const;

		virtual void *
			GetData();

	protected:
		FILE * mFileHandle;
		char * mData;
		int mDataSize;
		bool mManaged;
	};

}