#include "LFX_Image.h"

#include "lodepng.h"

namespace LFX {
	
#define PNG_DWORD(c0, c1, c2, c3) ((c0) << 24 | (c1) << 16 | (c2) << 8 | (c3))

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
		DWORD width;
		DWORD height;
		BYTE bitdepth;
		BYTE colortype;
		BYTE compression;
		BYTE filter;
		BYTE interlace;
	};

	int PNG_Read8(BYTE & i8, Stream & IS)
	{
		return IS.Read(&i8, 1);
	}

	int PNG_Read16(WORD & i16, Stream & IS)
	{
		int nreads = IS.Read(&i16, 2);

		//i16 = ENDIAN_BIG_TO_HOST(i16);

		return nreads;
	}

	int PNG_Read32(DWORD & i32, Stream & IS)
	{
		int nreads = IS.Read(&i32, 4);

		//i32 = ENDIAN_BIG_TO_HOST(i32);

		return nreads;
	}

	int PNG_ReadChunk(PNG_Chunk & ck, Stream & IS)
	{
		int nreads = IS.Read(&ck, 8);

		//ck.id = ENDIAN_BIG_TO_HOST(ck.id);
		//ck.length = ENDIAN_BIG_TO_HOST(ck.length);

		return nreads;
	}


	bool PNG_Test(Stream & stream)
	{
		BYTE png_magic[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
		BYTE magic[8];

		int nreads = stream.Read(magic, 8);

		stream.Seek(-nreads, SEEK_CUR);

		return memcmp(magic, png_magic, 8) == 0;
	}

	bool PNG_Load(Image & image, Stream & stream)
	{
		Stream & IS = stream;

		int nreads = 0;
		BYTE png_magic[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
		BYTE magic[8];

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

		LodePNGColorType colortype;
		int chanels = 0;

		switch (header.colortype)
		{
		case 0:
			colortype = LCT_GREY;
			chanels = 1;
			break;

		case 2:
			colortype = LCT_RGB;
			chanels = 3;
			break;

		case 3:
			colortype = LCT_RGBA;
			chanels = 4;
			break;

		case 4:
			colortype = LCT_GREY_ALPHA;
			chanels = 2;
			break;

		case 6:
			colortype = LCT_RGBA;
			chanels = 4;
			break;
		}

		if (chanels == 0)
			return false;

		IS.Seek(-nreads, SEEK_CUR);

		//
		unsigned char * data = (unsigned char *)stream.GetData();
		int len = stream.Size();
		unsigned int w = 0, h = 0;

		int err = lodepng_decode_memory(&image.pixels, &w, &h, data, len, colortype, 8);

		return err == 0;
	}

	bool PNG_Save(FILE * fp, const Image & image, bool auto_convert)
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

		BYTE * data = NULL;
		size_t size = 0;

		lodepng_encode_memory(&data, &size, image.pixels, image.width, image.height, colortype, 8, auto_convert);
		if (data != NULL)
		{
			fwrite(data, size, 1, fp);
			free(data);
		}

		return true;
	}

}