#pragma once

#include "LFX_Stream.h"

namespace LFX {

	struct Image
	{
		int channels;
		int width;
		int height;
		int bitdepth;
		std::vector<uint8_t> pixels;

		Image()
		{
			channels = 0;
			width = 0;
			height = 0;
			bitdepth = 8;
		}

		Image(int w, int h, int channels)
			: width(w)
			, height(h)
			, channels(channels)
		{
			pixels.resize(w * h * channels);
		}

		static void Copy(Image& dst, const Image& src, int x, int y)
		{
			for (int j = 0; j < src.height; ++j) {
				for (int i = 0; i < src.width; ++i) {
					const int dstIndex = (y + j) * dst.width + x + i;
					const int srcIndex = j * src.width + i;
					dst.pixels[dstIndex * dst.channels + 0] = src.pixels[srcIndex * src.channels + 0];
					dst.pixels[dstIndex * dst.channels + 1] = src.pixels[srcIndex * src.channels + 1];
					dst.pixels[dstIndex * dst.channels + 2] = src.pixels[srcIndex * src.channels + 2];
					if (dst.channels > 3 && src.channels > 3) {
						dst.pixels[dstIndex * dst.channels + 3] = src.pixels[srcIndex * src.channels + 3];
					}
				}
			}
		}

		static void Save(const Image& image, const char* filename)
		{
			LOGD("Save lighting map %s", filename);
			FILE* fp = fopen(filename, "wb");
			if (fp == nullptr) {
				LOGE("Save lighting map failed!!!");
				return;
			}

			LFX::PNG_Save(fp, image);
			fclose(fp);
		}
	};

	LFX_ENTRY bool BMP_Test(Stream & stream);
	LFX_ENTRY bool BMP_Load(Image & image, Stream & stream);
	LFX_ENTRY bool BMP_Save(FILE * fp, const Image & image);

	LFX_ENTRY bool PNG_Test(Stream & stream);
	LFX_ENTRY bool PNG_Load(Image& image, Stream& stream);
	LFX_ENTRY bool PNG_Save(FILE * fp, const Image & image);

	LFX_ENTRY bool TGA_Test(Stream & stream);
	LFX_ENTRY bool TGA_Load(Image & image, Stream & stream);

	LFX_ENTRY bool JPG_Test(Stream & stream);
	LFX_ENTRY bool JPG_Load(Image & image, Stream & stream);

}

