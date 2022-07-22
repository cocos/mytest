#pragma once

#include "LFX_Stream.h"

namespace LFX {

	struct Image
	{
		int channels;
		int width;
		int height;
		std::vector<uint8_t> pixels;

		Image()
		{
			channels = 0;
			width = 0;
			height = 0;
		}

		Image(int w, int h, int channels)
			: width(w)
			, height(h)
			, channels(channels)
		{
			pixels.resize(w * h * channels);
		}
	};

	LFX_ENTRY bool BMP_Test(Stream & stream);
	LFX_ENTRY bool BMP_Load(Image & image, Stream & stream);
	LFX_ENTRY bool BMP_Save(FILE * fp, const Image & image);

	LFX_ENTRY bool PNG_Test(Stream & stream);
	LFX_ENTRY bool PNG_Load(Image & image, Stream & stream);
	LFX_ENTRY bool PNG_Save(FILE * fp, const Image & image);

	LFX_ENTRY bool TGA_Test(Stream & stream);
	LFX_ENTRY bool TGA_Load(Image & image, Stream & stream);

}

