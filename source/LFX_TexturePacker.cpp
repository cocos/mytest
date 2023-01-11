#include "LFX_TexturePacker.h"

namespace LFX {

	HTexturePacker::HTexturePacker(int channels)
	{
		mWidth = 0;
		mHeight = 0;
		mChannels = channels;
	}

	void HTexturePacker::Insert(const unsigned char* pixels, int w, int h)
	{
		int newWidth = mWidth + w;
		int newHeight = std::max(mHeight, h);
		int bytesPerPixel = mChannels;

		std::vector<byte> temp;
		temp.resize(newWidth * newHeight * bytesPerPixel, 0);
		for (int y = 0; y < mHeight; ++y) {
			memcpy(
				&temp[y * newWidth * bytesPerPixel],
				&mBuffer[y * mWidth * bytesPerPixel],
				mWidth * bytesPerPixel);
		}

		for (int y = 0; y < h; ++y) {
			memcpy(
				&temp[(y * newWidth + mWidth) * bytesPerPixel],
				&pixels[y * w * bytesPerPixel],
				w * bytesPerPixel
			);
		}

		mBuffer = temp;
		mWidth = newWidth;
		mHeight = newHeight;
	}

	//
	VTexturePacker::VTexturePacker(int channels)
	{
		mWidth = 0;
		mHeight = 0;
		mChannels = channels;
	}

	void VTexturePacker::Insert(const unsigned char* pixels, int w, int h)
	{
		int newWidth = std::max(mWidth, w);
		int newHeight = mHeight + h;
		int bytesPerPixel = mChannels;

		std::vector<byte> temp;
		temp.resize(newWidth * newHeight * bytesPerPixel, 0);
		for (int y = 0; y < mHeight; ++y) {
			memcpy(
				&temp[y * newWidth * bytesPerPixel],
				&mBuffer[y * mWidth * bytesPerPixel],
				mWidth * bytesPerPixel);
		}

		for (int y = 0; y < h; ++y) {
			memcpy(
				&temp[((y + mHeight) * newWidth) * bytesPerPixel],
				&pixels[y * w * bytesPerPixel],
				w * bytesPerPixel
			);
		}

		mBuffer = temp;
		mWidth = newWidth;
		mHeight = newHeight;
	}

}