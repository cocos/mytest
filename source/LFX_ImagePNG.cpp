#include "LFX_Image.h"

#include "lodepng.h"

namespace LFX {

#define PNG_DWORD(c0, c1, c2, c3) ((c0) << 24 | (c1) << 16 | (c2) << 8 | (c3))

	inline bool IS_BIG_ENDIANG()
	{
		return ((*(short *)"1") > 0xFF) != 0;
	}

	inline int ENDIAN_SWAP16(int i)
	{
		return (i & 0x00FF) << 8 | (i & 0xFF00) >> 8;
	}

	inline int ENDIAN_SWAP32(int i)
	{
		return (i & 0x000000FF) << 24 | (i & 0x0000FF00) << 8 | (i & 0x00FF0000) >> 8 | (i & 0xFF000000) >> 24;
	}

	template <class T>
	inline T ENDIAN_LITTLE_TO_HOST(T i)
	{
		if (IS_BIG_ENDIANG())
		{
			assert(sizeof(T) == 2 || sizeof(T) == 4);

			return (T)(sizeof(T) == 2 ? ENDIAN_SWAP16(i) : ENDIAN_SWAP32(i));
		}

		return i;
	}

	template <class T>
	inline T ENDIAN_BIG_TO_HOST(T i)
	{
		if (!IS_BIG_ENDIANG())
		{
			assert(sizeof(T) == 2 || sizeof(T) == 4);

			return (T)(sizeof(T) == 2 ? ENDIAN_SWAP16(i) : ENDIAN_SWAP32(i));
		}

		return i;
	}

	union PNG_Chunk
	{
		struct {
			int length;
			int id;
		};

		char ch[8];
	};

	struct PNG_Header
	{
		uint32_t width;
		uint32_t height;
		uint8_t bitdepth;
		uint8_t colortype;
		uint8_t compression;
		uint8_t filter;
		uint8_t interlace;
	};

	int PNG_Read8(uint8_t & i8, Stream & IS)
	{
		return IS.Read(&i8, 1);
	}

	int PNG_Read16(uint16_t & i16, Stream & IS)
	{
		int nreads = IS.Read(&i16, 2);

		i16 = ENDIAN_BIG_TO_HOST(i16);

		return nreads;
	}

	int PNG_Read32(uint32_t & i32, Stream & IS)
	{
		int nreads = IS.Read(&i32, 4);

		i32 = ENDIAN_BIG_TO_HOST(i32);

		return nreads;
	}

	int PNG_ReadChunk(PNG_Chunk & ck, Stream & IS)
	{
		int nreads = IS.Read(&ck, 8);

		ck.id = ENDIAN_BIG_TO_HOST(ck.id);
		ck.length = ENDIAN_BIG_TO_HOST(ck.length);

		return nreads;
	}


	bool PNG_Test(Stream & stream)
	{
		uint8_t png_magic[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
		uint8_t magic[8];

		int nreads = stream.Read(magic, 8);

		stream.Seek(-nreads, SEEK_CUR);

		return memcmp(magic, png_magic, 8) == 0;
	}

	bool PNG_Load(Image & image, Stream & stream)
	{
		Stream & IS = stream;

		int nreads = 0;
		uint8_t png_magic[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
		uint8_t magic[8];

		nreads += IS.Read(magic, 8);
		if (memcmp(magic, png_magic, 8) != 0)
			return NULL;

		PNG_Chunk ck;
		nreads += PNG_ReadChunk(ck, IS);

		if (ck.id != PNG_DWORD('I', 'H', 'D', 'R') && ck.length != 13)
			return NULL;

		PNG_Header header;
		nreads += PNG_Read32(header.width, IS);
		nreads += PNG_Read32(header.height, IS);
		nreads += PNG_Read8(header.bitdepth, IS);
		nreads += PNG_Read8(header.colortype, IS);

		if (header.width > 4096 || header.height > 4096)
			return NULL;

		if (header.bitdepth != 8)
			return NULL;

		image.width = header.width;
		image.height = header.height;

		LodePNGColorType colortype = LCT_RGBA;
		switch (header.colortype)
		{
		case 0:
			colortype = LCT_GREY;
			image.channels = 1;
			break;

		case 2:
			colortype = LCT_RGB;
			image.channels = 3;
			break;

		case 3:
			colortype = LCT_RGBA;
			image.channels = 4;
			break;

		case 4:
			colortype = LCT_GREY_ALPHA;
			image.channels = 2;
			break;

		case 6:
			colortype = LCT_RGBA;
			image.channels = 4;
			break;
		}

		if (image.channels == 0)
			return false;

		IS.Seek(-nreads, SEEK_CUR);

		//
		unsigned char * data = (unsigned char *)stream.GetData();
		int len = stream.Size();
		unsigned char* pixels = nullptr;
		unsigned int w = 0, h = 0;

		int err = lodepng_decode_memory(&pixels, &w, &h, data, len, colortype, 8);
		if (pixels != nullptr) {
			image.pixels.resize(w * h * image.channels);
			memcpy(image.pixels.data(), pixels, image.pixels.size());
			SAFE_DELETE_ARRAY(pixels);
		}

		return err == 0;
	}

	bool PNG_Save(FILE * fp, const Image & image)
	{
		LodePNGColorType colortype;
		switch (image.channels)
		{
		case 1:
			colortype = LCT_GREY;
			break;
		case 2:
			colortype = LCT_GREY_ALPHA;
			break;
		case 3:
			colortype = LCT_RGB;
			break;
		case 4:
			colortype = LCT_RGBA;
			break;

		default:
			return false;
		}

		uint8_t * data = NULL;
		size_t size = 0;

		LodePNGState state;
		lodepng_state_init(&state);
		state.info_raw.colortype = colortype;
		state.info_raw.bitdepth = 8;
		state.info_png.color.colortype = colortype;
		state.info_png.color.bitdepth = 8;
		state.encoder.auto_convert = false;
		lodepng_encode(&data, &size, image.pixels.data(), image.width, image.height, &state);
  		lodepng_state_cleanup(&state);

		if (data != NULL)
		{
			fwrite(data, size, 1, fp);
			free(data);
		}

		return true;
	}

}
