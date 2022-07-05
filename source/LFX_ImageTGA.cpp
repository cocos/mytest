#include "LFX_Image.h"

namespace LFX {

	bool TGA_Test(Stream & stream)
	{
		uint8_t TGAHeader[12];
		uint8_t TGACompare[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		int nreads = stream.Read(TGAHeader, 12);
		if (TGAHeader[2] == 10) {
			TGAHeader[2] -= 8;
		}

		stream.Seek(-nreads, SEEK_CUR);

		return memcmp(TGAHeader, TGACompare, 12) == 0;
	}

	bool TGA_Load(Image& image, Stream& stream)
	{
		uint8_t TGAHeader[12];
		uint8_t TGACompare[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		bool rle = false;

		stream.Read(TGAHeader, 12);
		if (TGAHeader[2] == 10) {
			rle = true;
			TGAHeader[2] -= 8;
		}
		if (memcmp(TGAHeader, TGACompare, 12) != 0) {
			return false;
		}

		uint8_t header[6];
		stream.Read(header, 6);
		if (header[4] != 24 && header[4] != 32) {
			return false;
		}

		int bitcount = header[4];
		int chanels = bitcount == 24 ? 3 : 4;
		bool reversed = (header[5] & 0x20) == 0;

		image.channels = bitcount == 24 ? 3 : 4;
		image.width = header[1] * 256 + header[0];
		image.height = header[3] * 256 + header[2];
		image.pixels.resize(image.width * image.height * image.channels);

		if (!rle) {
			stream.Read(image.pixels.data(), image.width * image.height * chanels);

			uint8_t* pixels = image.pixels.data();
			int count = image.width * image.height * chanels;
			for (int i = 0; i < count; i += chanels) {
				std::swap(pixels[i], pixels[i + 2]);
			}
		}
		else {
			uint8_t rle_flag = 0;
			uint8_t rle_count = 0;
			uint8_t rle_repeating = 0;
			uint8_t raw_data[4] = { 0, 0, 0, 255 };
			uint8_t pixel[4] = { 0, 0, 0, 255 };
			for (int i = 0; i < image.width * image.height; ++i) {
				int read_pixel = 0;

				if (rle_count == 0) {
					stream >> rle_flag;
					rle_count = 1 + (rle_flag & 127);
					read_pixel = 1;
					rle_repeating = rle_flag >> 7;
				}
				else if (!rle_repeating) {
					read_pixel = 1;
				}

				if (read_pixel) {
					stream.Read(raw_data, chanels);
					switch (chanels) {
					case 3:
						pixel[0] = raw_data[2];
						pixel[1] = raw_data[1];
						pixel[2] = raw_data[0];
						break;

					case 4:
						pixel[0] = raw_data[2];
						pixel[1] = raw_data[1];
						pixel[2] = raw_data[0];
						pixel[3] = raw_data[3];
						break;
					}
				}

				uint8_t* d_pixels = image.pixels.data();
				switch (chanels) {
				case 3:
					d_pixels[i * 3 + 0] = pixel[0];
					d_pixels[i * 3 + 1] = pixel[1];
					d_pixels[i * 3 + 2] = pixel[2];
					break;

				case 4:
					d_pixels[i * 4 + 0] = pixel[0];
					d_pixels[i * 4 + 1] = pixel[1];
					d_pixels[i * 4 + 2] = pixel[2];
					d_pixels[i * 4 + 3] = pixel[3];
					break;
				}

				--rle_count;
			}
		}

		if (!image.pixels.empty() && reversed) {
			uint8_t* pixels = image.pixels.data();
			int line_uint8_ts = image.width * bitcount / 8;

			std::vector<uint8_t> buffer;
			buffer.resize(line_uint8_ts);
			for (int i = 0, j = image.height - 1; i < j; ++i, --j) {
				memcpy(buffer.data(), &pixels[i * line_uint8_ts], line_uint8_ts);
				memcpy(&pixels[i * line_uint8_ts], &pixels[j * line_uint8_ts], line_uint8_ts);
				memcpy(&pixels[j * line_uint8_ts], buffer.data(), line_uint8_ts);
			}
		}

		return true;
	}

}

