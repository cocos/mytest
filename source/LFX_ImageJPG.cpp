#include "LFX_Image.h"

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4312 4456 4457)
#endif

#define STBI_NO_GIF
//#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#ifdef _MSC_VER
#pragma warning (pop)
#endif

namespace LFX {

	struct STBImageInitializer
	{
		STBImageInitializer()
		{
			static bool initialized = false;
			if (!initialized) {
				stbi_convert_iphone_png_to_rgb(1);
				stbi_set_unpremultiply_on_load(1);
				initialized = true;
			}
		}
	};

	bool JPG_Test(Stream & stream)
	{
		byte header[2];
		int nreads = stream.Read(header, 2);

		stream.Seek(-nreads, SEEK_CUR);

		return header[0] == 0xFF && header[1] == 0xD8;
	}

	bool JPG_Load(Image & image, Stream & stream)
	{
		STBImageInitializer Initializer;

		const void* fileData = stream.GetData();
		const int fileSize = stream.Size();

		int w = 0, h = 0, comp = 0;
		stbi_uc* data = stbi_load_from_memory((const stbi_uc*)fileData, fileSize, &w, &h, &comp, 0);
		if (data == nullptr) {
			return false;
		}

		if (comp != 1 && comp != 3 && comp != 4) {
			stbi_image_free(data);
			return false;
		}

		switch (comp) {
		case 1:
			image.channels = 1;
			break;
		case 3:
			image.channels = 3;
			break;
		case 4:
			image.channels = 4;
			break;
		}
		image.width = w;
		image.height = h;
		image.pixels.resize(w * h * comp);
		memcpy(image.pixels.data(), data, w * h * comp);
		stbi_image_free(data);

		return true;
	}

}

