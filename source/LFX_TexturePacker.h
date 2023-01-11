#pragma once

#include "LFX_Math.h"

namespace LFX {

	/**
	* Horizonal texture packer
	*/
	class HTexturePacker
	{
	public:
		HTexturePacker(int channels);

		/**
		* Insert texture
		*/
		void Insert(const unsigned char* pixels, int w, int h);

		/**
		* Get texture width
		*/
		int GetWidth() const { return mWidth; }
		/**
		* Get texture height
		*/
		int GetHeight() const { return mHeight; }
		/**
		* Get texture data buffer
		*/
		const std::vector<byte>& GetBuffer() const { return mBuffer; }

	protected:
		int mWidth;
		int mHeight;
		int mChannels;
		std::vector<byte> mBuffer;
	};

	/**
	* Vertical texture packer
	*/
	class VTexturePacker
	{
	public:
		VTexturePacker(int channels);

		/**
		* Insert texture
		*/
		void Insert(const unsigned char* pixels, int w, int h);

		/**
		* Get texture width
		*/
		int GetWidth() const { return mWidth; }
		/**
		* Get texture height
		*/
		int GetHeight() const { return mHeight; }
		/**
		* Get texture data buffer
		*/
		const std::vector<byte>& GetBuffer() const { return mBuffer; }

	protected:
		int mWidth;
		int mHeight;
		int mChannels;
		std::vector<byte> mBuffer;
	};

}