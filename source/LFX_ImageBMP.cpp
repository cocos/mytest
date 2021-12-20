#include "LFX_Image.h"

namespace LFX {

	struct BMP_Header
	{
		uint16_t type;
		uint32_t sizefile;
		uint16_t reserved1;
		uint16_t reserved2;
		uint32_t offbytes;

		union BMP_HeaderInfo
		{
			struct {
				int size;
				int width;
				int height;
				short plane;
				short bitcount;
				int compression;
				int sizeimage;
				int xpelspermeter;
				int ypelspermeter;
				int clrused;
				int clrimportant;
			};

			uint8_t m[40];

		} info;
	};

	struct BMP_RGBQuad
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	};

	bool BMP_Test(Stream & stream)
	{
		uint16_t type = 0;

		int nreads = stream.Read(&type, sizeof(uint16_t));

		stream.Seek(-nreads, SEEK_CUR);

		return type == 0x4d42;
	}

	bool BMP_Load(Image & image, Stream & IS)
	{
		BMP_Header header;
		header.type = 0;

		IS >> header.type;
		IS >> header.sizefile;
		IS >> header.reserved1;
		IS >> header.reserved2;
		IS >> header.offbytes;

		if (header.type != 0x4d42)
			return false;

		header.info.size = 0;
		IS.Read(&header.info, 40);

		if (header.info.size != 40)
		{
			assert(0 && "?: BMP format not supported");
			return false;
		}

		if (header.info.compression == 1 || header.info.compression == 2)
		{
			assert(0 && "?: BMP compression 'RLE', not supported");
			return false;
		}

		bool flip_vertically = header.info.height > 0;
		header.info.height = abs(header.info.height);

		if (header.info.width > 2048 || header.info.height > 2048)
		{
			assert(0 && "?: BMP size too large");
			return false;
		}

		image.width = header.info.width;
		image.height = header.info.height;

		if (header.info.bitcount == 8)
		{
			BMP_RGBQuad palette[256];
			IS.Read(palette, sizeof(BMP_RGBQuad) * 256);

			uint8_t * data = new uint8_t[image.width * image.height];
			IS.Read(data, image.width * image.height);

			image.pixels = new uint8_t[image.width * image.height * 3];
			for (int i = 0; i < image.width * image.height; ++i)
			{
				image.pixels[i * 3 + 0] = palette[data[i]].r;
				image.pixels[i * 3 + 1] = palette[data[i]].g;
				image.pixels[i * 3 + 2] = palette[data[i]].b;
			}

			delete[] data;

			image.channels = 3;
		}
		else if (header.info.bitcount == 24)
		{
			image.pixels = new uint8_t[image.width * image.height * 3];

			IS.Read(image.pixels, image.width * image.height * 3);
			for (int i = 0; i < image.width * image.height; ++i)
			{
				std::swap(image.pixels[i * 3 + 0], image.pixels[i * 3 + 2]);
			}

			image.channels = 3;
		}
		else if (header.info.bitcount == 32)
		{
			image.pixels = new uint8_t[image.width * image.height * 4];
			IS.Read(image.pixels, image.width * image.height * 4);
			for (int i = 0; i < image.width * image.height; ++i)
			{
				std::swap(image.pixels[i * 4 + 0], image.pixels[i * 4 + 2]);
			}

			image.channels = 4;
		}
		else
		{
			assert(0 && "?: BMP format not supported");
			return false;
		}

		if (flip_vertically)
		{
			int line_bytes = image.width * header.info.bitcount / 8;

			uint8_t * buffer = new uint8_t[line_bytes];
			for (int i = 0, j = image.height - 1; i < j; ++i, --j)
			{
				memcpy(buffer, &image.pixels[i * line_bytes], line_bytes);
				memcpy(&image.pixels[i * line_bytes], &image.pixels[j * line_bytes], line_bytes);
				memcpy(&image.pixels[j * line_bytes], buffer, line_bytes);
			}
			delete[] buffer;
		}

		return true;
	}

	bool BMP_Save(FILE * fp, const Image & image)
	{
		int bitcount = image.channels * 8;
		int chanels = bitcount / 8;
		int line_bytes = image.width * chanels;

		BMP_Header header;
		header.type = 0x4d42;
		header.sizefile = 14 + 40 + line_bytes * image.height;
		header.reserved1 = 0;
		header.reserved2 = 0;
		header.offbytes = 14 + 40;

		fwrite(&header.type, sizeof(header.type), 1, fp);
		fwrite(&header.sizefile, sizeof(header.sizefile), 1, fp);
		fwrite(&header.reserved1, sizeof(header.reserved1), 1, fp);
		fwrite(&header.reserved2, sizeof(header.reserved2), 1, fp);
		fwrite(&header.offbytes, sizeof(header.offbytes), 1, fp);

		header.info.size = 40;
		header.info.width = image.width;
		header.info.height = image.height;
		header.info.plane = 1;
		header.info.bitcount = bitcount;
		header.info.compression = 0;
		header.info.sizeimage = line_bytes * image.height;
		header.info.xpelspermeter = 0;
		header.info.ypelspermeter = 0;
		header.info.clrused = 0;
		header.info.clrimportant = 0;

		fwrite(&header.info, 40, 1, fp);

		uint8_t * buffer = new uint8_t[line_bytes];
		for (int j = image.height - 1; j >= 0; --j)
		{
			const uint8_t * c_pixels = image.pixels + line_bytes * j;

			memcpy(buffer, c_pixels, line_bytes);

			for (int i = 0; i < image.width; ++i)
			{
				std::swap(buffer[i * chanels + 0], buffer[i * chanels + 2]);
			}

			fwrite(buffer, line_bytes, 1, fp);
		}
		delete[] buffer;

		return true;
	}

}