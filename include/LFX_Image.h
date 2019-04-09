#pragma once

#include "LFX_Stream.h"

namespace LFX {

	struct Image
	{
		uint8_t * pixels;

		int channels;
		int width;
		int height;

		Image()
		{
			pixels = 0;
			channels = 0;
			width = 0;
			height = 0;
		}

		~Image()
		{
			if (pixels)
				delete[] pixels;
		}
	};

	LFX_ENTRY bool
		BMP_Test(Stream & stream);
	LFX_ENTRY bool
		BMP_Load(Image & image, Stream & stream);
	LFX_ENTRY bool
		BMP_Save(FILE * fp, const Image & image);

	LFX_ENTRY bool
		PNG_Test(Stream & stream);
	LFX_ENTRY bool
		PNG_Load(Image & image, Stream & stream);
	LFX_ENTRY bool
		PNG_Save(FILE * fp, const Image & image);

}

