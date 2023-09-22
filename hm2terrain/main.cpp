#include "LFX_Log.h"
#include "LFX_Image.h"
#include "LFX_Texture.h"
#include "tinyexr.h"

using namespace LFX;

constexpr int TERRAIN_BLOCK_TILE_COMPLEXITY = 32;
constexpr int TERRAIN_BLOCK_VERTEX_COMPLEXITY = 33;
constexpr int TERRAIN_DATA_VERSION = 0x01010008; // V8
constexpr int TERRAIN_HEIGHT_BASE = 32768;
constexpr float TERRAIN_HEIGHT_FACTORY = 1.0 / 128.0;
constexpr float TERRAIN_HEIGHT_FMIN = (-TERRAIN_HEIGHT_BASE) * TERRAIN_HEIGHT_FACTORY;
constexpr float TERRAIN_HEIGHT_FMAX = (65535 - TERRAIN_HEIGHT_BASE) * TERRAIN_HEIGHT_FACTORY;

float Height2Float(std::uint16_t h)
{
	return ((int)h - TERRAIN_HEIGHT_BASE) * TERRAIN_HEIGHT_FACTORY;
}

std::uint16_t Float2Height(float h)
{
	int v = int(TERRAIN_HEIGHT_BASE + h / TERRAIN_HEIGHT_FACTORY);
	if (v < 0) {
		return 0;
	}
	else if (v > 65535) {
		return 65535;
	}

	return std::uint16_t(v);
}

bool LoadTexture(Texture* tex, const String& filename)
{
	Image img;
	FileStream stream(filename.c_str());

	if (PNG_Test(stream)) {
		PNG_Load(img, stream);
	}
	else if (JPG_Test(stream)) {
		JPG_Load(img, stream);
	}
	else if (TGA_Test(stream)) {
		TGA_Load(img, stream);
	}
	else if (BMP_Test(stream)) {
		BMP_Load(img, stream);
	}

	if (!img.pixels.empty()) {
		LOGI("Texture '%s' loaded", filename.c_str());
		tex->name = filename;
		tex->width = img.width;
		tex->height = img.height;
		tex->channels = img.channels;
		tex->bitdepth = img.bitdepth;
		tex->data = img.pixels;
	}
	else {
		LOGW("Load Texture '%s' failed", filename.c_str());
		// create dummy texture
		tex->name = filename;
		tex->width = 4;
		tex->height = 4;
		tex->channels = 3;
		tex->data.resize(3 * 4 * 4);
		memset(tex->data.data(), 0, tex->data.size());
		return false;
	}

	return true;
}

struct FImageEXR
{
	int width, height;
	int channels;
	std::vector<float> data;
};

bool LoadEXR(const String& filename, FImageEXR& image)
{
	EXRVersion exr_version;

	int ret = ParseEXRVersionFromFile(&exr_version, filename.c_str());
	if (ret != 0) {
		LOGE("Invalid EXR file: %s", filename.c_str());
		return false;
	}

#if 0
	if (!exr_version.multipart) {
		// must be multipart flag is true.
		return false;
	}
#endif

	// 2. Read EXR header
	EXRHeader exr_header;
	InitEXRHeader(&exr_header);

	const char* err = NULL; // or `nullptr` in C++11 or later.
	ret = ParseEXRHeaderFromFile(&exr_header, &exr_version, filename.c_str(), &err);
	if (ret != 0) {
		fprintf(stderr, "Parse EXR err: %s\n", err);
		FreeEXRErrorMessage(err); // free's buffer for an error message
		return ret;
	}

	// // Read HALF channel as FLOAT.
	// for (int i = 0; i < exr_header.num_channels; i++) {
	//   if (exr_header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
	//     exr_header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
	//   }
	// }

	EXRImage exr_image;
	InitEXRImage(&exr_image);

	ret = LoadEXRImageFromFile(&exr_image, &exr_header, filename.c_str(), &err);
	if (ret != 0) {
		LOGE("Load EXR err: %s\n", err);
		FreeEXRHeader(&exr_header);
		FreeEXRErrorMessage(err); // free's buffer for an error message
		return ret;
	}

	// 3. Access image data
	// `exr_image.images` will be filled when EXR is scanline format.
	// `exr_image.tiled` will be filled when EXR is tiled format.
	image.width = exr_image.width;
	image.height = exr_image.height;
	image.channels = exr_image.num_channels;
	image.data.resize(image.width * image.height * image.channels);
	float min = FLT_MAX, max = -FLT_MAX;
	for (int i = 0; i < image.width * image.height; ++i) {
		for (int c = 0; c < image.channels; ++c) {
			const float* src = (const float*)exr_image.images[c];
			const float value = src[i];
			image.data[i * image.channels + c] = value;
			min = std::min(min, value);
			max = std::max(max, value);
		}
	}

	// 4. Free image data
	FreeEXRImage(&exr_image);
	FreeEXRHeader(&exr_header);

	return true;
}

bool LoadRaw(const String& filename, std::vector<float>& field, int size)
{
	FILE* fp = fopen(filename.c_str(), "rb");
	if (fp == nullptr) {
		LOGE("Open raw data '%s' failed", filename.c_str());
		return false;
	}

	field.resize(size * size);
	fread(field.data(), sizeof(float), field.size(), fp);
	fclose(fp);
	return true;
}

bool SaveTexture(Texture* tex, const String& filename)
{
	FILE* fp = fopen(filename.c_str(), "wb");
	if (fp) {
		Image image;
		image.width = tex->width;
		image.height = tex->height;
		image.channels = tex->channels;
		image.bitdepth = tex->bitdepth;
		image.pixels = tex->data;
		PNG_Save(fp, image);
		fclose(fp);
		return true;
	}

	return false;
}

bool SaveRawMap(const std::vector<float>& rawData, const String& filename)
{
	FILE* fp = fopen(filename.c_str(), "wb");
	if (fp) {
		fwrite(rawData.data(), sizeof(float), rawData.size(), fp);
		fclose(fp);
		return true;
	}

	return false;
}

bool SaveHeightMap(const std::vector<float>& rawData, int width, float heightScale, const String& filename)
{
	FILE* fp = fopen(filename.c_str(), "wb");
	if (fp) {
		Image image;
		image.width = width;
		image.height = width;
		image.channels = 1;
		image.bitdepth = 8;

		image.pixels.resize(width * width);
		for (size_t i = 0; i < image.pixels.size(); ++i) {
			float v = (rawData[i] / heightScale) * 0.5f + 0.5f;
			v = Clamp<float>(v, 0.0f, 1.0f);
			image.pixels[i] = (uint8)(v * 255);
		}

		PNG_Save(fp, image);
		fclose(fp);
		return true;
	}

	return false;
}

int main(int args, char** argv)
{
	Log logger("hm2.log");

	std::int16_t weightMapSize = 128;
	std::int16_t lightMapSize = 256;
	std::double_t tileSize = 1.0;
	std::float_t heightScale = 256.0f;
	Texture heightMap;
	Texture weightMap;
	std::string rawFile;
	std::vector<float> rawData;
	std::int32_t width = 0;
	float minHeight = 0.0f;
	float maxHeight = 255.0f;
	//float minHeight = -19.3261f;
	//float maxHeight = 232.692f;

	for (int i = 0; i < args; ++i) {
		const char* key = argv[i];
		if (strcmp(argv[i], "-file") == 0) {
			if (!LoadTexture(&heightMap, argv[++i])) {
				return -1;
			}

			if (heightMap.width != heightMap.height) {
				LOGE("height map is not squared!!!");
				return -1;
			}
		}
		else if (strcmp(argv[i], "-exr") == 0) {
			FImageEXR image;
			if (!LoadEXR(argv[++i], image)) {
				LOGE("load exr image error!!!");
				return -1;
			}

			if (image.width != image.height) {
				LOGE("height map is not squared!!!");
				return -1;
			}

			width = image.width;
			rawData = image.data;
		}
		else if (strcmp(argv[i], "-wm") == 0) {
			if (!LoadTexture(&weightMap, argv[++i])) {
				return -1;
			}

			if (weightMap.width != weightMap.height) {
				LOGE("weight map is not squared!!!");
				return -1;
			}
		}
		else if (strcmp(argv[i], "-raw") == 0) {
			rawFile = argv[++i];
		}
		else if (strcmp(argv[i], "-width") == 0) {
			width = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-tileSize") == 0) {
			tileSize = (float)atof(argv[++i]);
		}
		else if (strcmp(argv[i], "-heightScale") == 0) {
			heightScale = (float)atof(argv[++i]);
		}
		else if (strcmp(argv[i], "-minHeight") == 0) {
			minHeight = (float)atof(argv[++i]);
		}
		else if (strcmp(argv[i], "-maxHeight") == 0) {
			maxHeight = (float)atof(argv[++i]);
		}
	}

	if (rawFile != "" && width <= 0) {
		LOGE("invalid raw width!!!");
		return -1;
	}
	if (tileSize <= 0) {
		LOGE("invalid terrain tileSize!!!");
		return -1;
	}
	if (minHeight >= maxHeight) {
		LOGE("invalid minHeight or maxHeight!!!");
		return -1;
	}
	if (heightScale <= 0) {
		LOGE("invalid terrain heightScale!!!");
		return -1;
	}

	std::int32_t blocks;
	std::int32_t complexity;
	std::vector<float> heightField;

	if (rawFile != "") {
		if (!LoadRaw(rawFile, rawData, width)) {
			return -1;
		}
	}
	else if (rawData.empty()) {
		width = heightMap.width;
	}

	blocks = (width - 1) / TERRAIN_BLOCK_TILE_COMPLEXITY;
	if (blocks * TERRAIN_BLOCK_TILE_COMPLEXITY + 1 < width) {
		++blocks;
	}
	complexity = blocks * TERRAIN_BLOCK_TILE_COMPLEXITY + 1;

	heightField.resize(complexity * complexity, 0);
	if (!rawData.empty()) {
		for (int v = 0; v < std::min(complexity, width); ++v) {
			for (int u = 0; u < std::min(complexity, width); ++u) {
				auto h = rawData[v * width + u];
				heightField[v * complexity + u] = minHeight + h * maxHeight;
			}
		}
	}
	else {
		for (int v = 0; v < std::min(complexity, width); ++v) {
			for (int u = 0; u < std::min(complexity, width); ++u) {
				Float4 color;
				heightMap.GetColor(color, u, v);

				float h = 0;
				h = minHeight + color.x * maxHeight;
				//h = ((color.x - 0.5f) * 2.0f) * heightScale;
				heightField[v * complexity + u] = h;
			}
		}
	}

	//SaveRawMap(heightField, "hm2raw.bin");
	SaveHeightMap(heightField, complexity, heightScale, "out_hm.png");
	if (weightMap.width != 0) {
		SaveTexture(&weightMap, "out_wm.png");
	}

	FILE* fp = fopen("out.terrain", "wb");
	fwrite(&TERRAIN_DATA_VERSION, sizeof(int), 1, fp);
	fwrite(&tileSize, sizeof(tileSize), 1, fp);
	fwrite(&blocks, sizeof(blocks), 1, fp);
	fwrite(&blocks, sizeof(blocks), 1, fp);
	fwrite(&weightMapSize, sizeof(weightMapSize), 1, fp);
	fwrite(&lightMapSize, sizeof(lightMapSize), 1, fp);

	// heights
	std::int32_t heightMapSize = complexity * complexity;
	fwrite(&heightMapSize, sizeof(heightMapSize), 1, fp);
	for (int v = 0; v < complexity; ++v) {
		for (int u = 0; u < complexity; ++u) {
			std::uint16_t h = Float2Height(heightField[v * complexity + u]);
			fwrite(&h, sizeof(h), 1, fp);
		}
	}
	
	// normal buffer
	std::int32_t normalBufferSize = 0;
	fwrite(&normalBufferSize, sizeof(normalBufferSize), 1, fp);

	// weight buffer
	if (!weightMap.data.empty()) {
		std::int32_t weightBufferDim = weightMapSize * blocks;
		std::int32_t weightBufferSize = weightBufferDim * weightBufferDim * 4;
		fwrite(&weightBufferSize, sizeof(weightBufferSize), 1, fp);
		for (int v = 0; v < weightBufferDim; ++v) {
			for (int u = 0; u < weightBufferDim; ++u) {
				Float4 fw(0, 0, 0, 0);
				float fu = u / float(weightBufferDim - 1);
				float fv = v / float(weightBufferDim - 1);
				// transform to terrain space
				float fx = fu * complexity;
				float fy = fv * complexity;
				// transform to weightmap space
				if (fx < width && fy < width) {
					float su = fx / (width - 1);
					float sv = fy / (width - 1);
					fw = weightMap.SampleColor(su, sv, false);
					if (weightMap.channels == 3) {
						fw.w = 0;
					}
				}
				
				// write weight data
				uint8 r = uint8(fw.x * 255);
				uint8 g = uint8(fw.y * 255);
				uint8 b = uint8(fw.z * 255);
				uint8 a = uint8(fw.w * 255);
				fwrite(&r, sizeof(r), 1, fp);
				fwrite(&g, sizeof(r), 1, fp);
				fwrite(&b, sizeof(r), 1, fp);
				fwrite(&a, sizeof(r), 1, fp);
			}
		}
	}
	else {
		std::int32_t weightBufferSize = 0;
		fwrite(&weightBufferSize, sizeof(weightBufferSize), 1, fp);
	}

	// layer buffer
	if (!weightMap.data.empty()) {
		std::int32_t layerBufferSize = blocks * blocks * 4;
		fwrite(&layerBufferSize, sizeof(layerBufferSize), 1, fp);
		for (int i = 0; i < blocks * blocks; ++i) {
			uint16 layer0 = 0;
			uint16 layer1 = 1;
			uint16 layer2 = 2;
			uint16 layer3 = 3;
			fwrite(&layer0, sizeof(layer0), 1, fp);
			fwrite(&layer1, sizeof(layer0), 1, fp);
			fwrite(&layer2, sizeof(layer0), 1, fp);
			fwrite(&layer3, sizeof(layer0), 1, fp);
		}
	}
	else {
		std::int32_t layerBufferSize = 0;
		fwrite(&layerBufferSize, sizeof(layerBufferSize), 1, fp);
	}

	// layers
	std::int32_t layerSize = 0;
	fwrite(&layerSize, sizeof(layerSize), 1, fp);
	
	fclose(fp);

}