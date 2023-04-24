#include "LFX_World.h"
#include "LFX_Image.h"
#include "LFX_Stream.h"
#include "LFX_TextureAtlas.h"
#include "LFX_TexturePacker.h"
#include "LFX_EmbreeScene.h"

namespace LFX {

	ImplementSingleton(World);

	World::World()
	{
		mScene = NULL;
		mShader = new Shader;
	}

	World::~World()
	{
		Clear();
		delete mShader;
	}

	static const int LFX_FILE_VERSION = 0x2000;
	static const int LFX_FILE_VERSION_372 = 0x2001;
	static const int LFX_FILE_VERSION_372_2 = 0x2002;
	static const int LFX_FILE_VERSION_372_3 = 0x2003;
	static const int LFX_FILE_VERSION_373 = 0x3730;

	static const int LFX_FILE_TERRAIN = 0x01;
	static const int LFX_FILE_MESH = 0x02;
	static const int LFX_FILE_LIGHT = 0x03;
	static const int LFX_FILE_SHPROBE = 0x04;
	static const int LFX_FILE_EOF = 0x00;

	bool World::Load()
	{
		String filename = "tmp/lfx.in";

		FileStream stream(filename.c_str());
		if (!stream.IsOpen()) {
			LOGE("Can not open file '%s'", filename.c_str());
			return false;
		}

		// Check verison
		int version;
		stream >> version;
		if (version != LFX_FILE_VERSION &&
			version != LFX_FILE_VERSION_372 &&
			version != LFX_FILE_VERSION_372_2 &&
			version != LFX_FILE_VERSION_372_3 &&
			version != LFX_FILE_VERSION_373) {
			LOGE("file head invalid");
			return false;
		}

		String name = stream.ReadString();

		// Load setting
#ifdef LFX_FORCE_RGBE
		mSetting.RGBEFormat = true;
#endif

		stream >> mSetting.Ambient;
		stream >> mSetting.SkyRadiance;
		stream >> mSetting.MSAA;
#ifdef LFX_FEATURE_EDGE_AA
		stream >> mSetting.EdgeAA;
#endif
		stream >> mSetting.Size;
		stream >> mSetting.Gamma;
		if (version >= LFX_FILE_VERSION_372) {
			stream >> mSetting.Highp;
		}
		stream >> mSetting.GIScale;
		stream >> mSetting.GISamples;
		stream >> mSetting.GIPathLength;
		stream >> mSetting.GIProbeScale;
		stream >> mSetting.GIProbeSamples;
		stream >> mSetting.GIProbePathLength;
		stream >> mSetting.AOLevel;
		stream >> mSetting.AOStrength;
		stream >> mSetting.AORadius;
		stream >> mSetting.AOColor;
		stream >> mSetting.Threads;
		if (version >= LFX_FILE_VERSION_373) {
			stream >> mSetting.Filter;
		}
		stream >> mSetting.BakeLightMap;
		stream >> mSetting.BakeLightProbe;
		// disable gamma correction
		mSetting.Gamma = 1;
		// Force set gi scale
		//mSetting.GIScale = 0;
		// Force set ao level
		//mSetting.AOLevel = 0;
		// Disable sky lighting
		mSetting.SkyRadiance = Float3(0, 0, 0);

		int ckId = 0;
		while (stream.Read(&ckId, sizeof(int))) {
			if (ckId == 0) {
				break;
			}

			switch (ckId) {
			case LFX_FILE_TERRAIN: {
				Terrain::Desc desc;
				stream >> desc.Position;
				stream >> desc.GridSize;
				stream >> desc.BlockCount;
				stream >> desc.LMapSize;
				desc.GridCount = desc.BlockCount * 32;
				desc.VertexCount = desc.GridCount + Int2(1, 1);
				desc.Dimension.x = desc.GridSize * desc.GridCount.x;
				desc.Dimension.y = desc.GridSize * desc.GridCount.y;

				std::vector<unsigned short> heightfield;
				heightfield.resize(desc.VertexCount.x * desc.VertexCount.y);
				stream.Read(&heightfield[0], heightfield.size() * 2);

				std::vector<float> heights;
				heights.resize(desc.VertexCount.x * desc.VertexCount.y);
				for (int i = 0; i < heights.size(); ++i) {
					const int TERRAIN_HEIGHT_BASE = 32768;
					const float TERRAIN_HEIGHT_FACTORY = 1.0f / 512.0f;
					heights[i] = (heightfield[i] - TERRAIN_HEIGHT_BASE) * TERRAIN_HEIGHT_FACTORY;
				}

				CreateTerrain(&heights[0], desc);
				break;
			}
				
			case LFX_FILE_MESH: {
				Mesh *m = CreateMesh();
				m->SetCastShadow(stream.ReadT<bool>());
				m->SetRecieveShadow(stream.ReadT<bool>());
				m->SetLightingMapSize(stream.ReadT<int>());

				int numVtxs, numTris, numMtls;
				stream >> numVtxs;
				stream >> numTris;
				stream >> numMtls;
				m->Alloc(numVtxs, numTris, numMtls);

				Vertex *vtx;
				Triangle *tri;
				Material *mtl;
				m->Lock(&vtx, &tri, &mtl);
				for (int i = 0; i < numVtxs; ++i) {
					stream >> vtx[i].Position;
					stream >> vtx[i].Normal;
					stream >> vtx[i].UV;
					stream >> vtx[i].LUV;
				}

				stream.Read(tri, sizeof(Triangle) * numTris);

				for (auto i = 0; i < numMtls; ++i) {
					if (version >= LFX_FILE_VERSION_373) {
						stream >> mtl[i].alphaCutoff;
					}
					stream >> mtl[i].Metallic;
					stream >> mtl[i].Roughness;
					stream >> mtl[i].Diffuse;
					if (version >= LFX_FILE_VERSION_372_2) {
						stream >> mtl[i].Emissive;
					}

					String diffuseMap = stream.ReadString();
					if (diffuseMap != "") {
						mtl[i].DiffuseMap = LoadTexture(diffuseMap);
					}
					String pbrMap = stream.ReadString();
					if (pbrMap != "") {
						mtl[i].PBRMap = LoadTexture(pbrMap);
					}
					if (version >= LFX_FILE_VERSION_372_2) {
						String emissiveMap = stream.ReadString();
						if (emissiveMap != "") {
							mtl[i].EmissiveMap = LoadTexture(emissiveMap);
						}
					}
				}
				m->Unlock();

#if 0
				// Test lightmap uv
				{
					std::vector<Float2> uvs;
					std::vector<Float2> luvs;
					std::vector<int> indices;
					uint8 color[3] = { 255, 255, 255 };

					uvs.resize(numVtxs);
					luvs.resize(numVtxs);
					for (int i = 0; i < luvs.size(); ++i) {
						uvs[i] = vtx[i].UV;
						luvs[i] = vtx[i].LUV;
					}

					indices.resize(numTris * 3);
					for (int i = 0; i < numTris; ++i) {
						indices[i * 3 + 0] = tri[i].Index0;
						indices[i * 3 + 1] = tri[i].Index1;
						indices[i * 3 + 2] = tri[i].Index2;
					}

					Image luv_image(1024, 1024, 3);
					memset(luv_image.pixels.data(), 0, luv_image.pixels.size());
					Debug::RasterizeMeshToImage(&luv_image, luvs, indices, color);
					FILE* tfp = fopen("test_luv.png", "wb");
					LFX::PNG_Save(tfp, luv_image);
					fclose(tfp);

					Image uv_image(1024, 1024, 3);
					Debug::RasterizeMeshToImage(&uv_image, uvs, indices, color);
					tfp = fopen("test_uv.png", "wb");
					LFX::PNG_Save(tfp, uv_image);
					fclose(tfp);
					break;
				}
#endif
				break;
			}

			case LFX_FILE_LIGHT: {
				Light *l = CreateLight();
				stream >> l->Type;
				stream >> l->Position;
				stream >> l->Direction;
				stream >> l->Color;
				stream >> l->AttenStart;
				stream >> l->AttenEnd;
				stream >> l->AttenFallOff;
				stream >> l->SpotInner;
				stream >> l->SpotOuter;
				stream >> l->SpotFallOff;
				stream >> l->DirectScale;
				stream >> l->IndirectScale;
				stream >> l->GIEnable;
				stream >> l->CastShadow;
				if (version >= LFX_FILE_VERSION_372_3) {
					stream >> l->ShadowMask;
				}
				if (l->Type == Light::DIRECTION && l->DirectScale == 0) {
					l->SaveShadowMask = true;
				}
				break;
			}

			case LFX_FILE_SHPROBE: {
				SHProbe* p = CreateSHProbe();
				stream >> p->position;
				stream >> p->normal;
				break;
			}

			default:
				LOGW("Unknown chunk %d", ckId);
				return false;
			};
		}

		return true;
	}

	struct PackedLightmapItem
	{
		TextureAtlasPacker::Item atlasItem;
		Image hpart;
		Image lpart;
		float factor;

		PackedLightmapItem() : factor(1)
		{
		}
	};

	void ConvertColor(PackedLightmapItem* item, int w, int h, std::vector<LightmapValue>& colors, const World::Settings& settings)
	{
#if LFX_ENABLE_GAMMA
		if (settings.Gamma != 1.0f) {
			for (int k = 0; k < colors.size(); ++k) {
				colors[k] = Pow(colors[k], 1.0f / settings.Gamma);
			}
		}
#endif
		item->hpart.width = w;
		item->hpart.height = h;
		item->hpart.channels = 4;
		item->hpart.pixels.resize(w * h * 4);

		if (settings.Highp) {
			item->lpart.width = item->hpart.width;
			item->lpart.height = item->hpart.height;
			item->lpart.channels = item->hpart.channels;
			item->lpart.pixels.resize(w * h * 4);
		}

		if (!settings.RGBEFormat) {
			float factor = item->factor;
			for (int k = 0; k < colors.size(); ++k) {
				factor = std::max(colors[k].Diffuse.x, factor);
				factor = std::max(colors[k].Diffuse.y, factor);
				factor = std::max(colors[k].Diffuse.z, factor);
			}
			item->factor = factor;

			if (!settings.Highp) {
				auto& hpart = item->hpart;
				for (int k = 0; k < colors.size(); ++k) {
					Float3 color = colors[k].Diffuse;
					color /= factor;
					color.saturate();

					hpart.pixels[k * 4 + 0] = (uint8_t)std::min(int(color.x * 255), 255);
					hpart.pixels[k * 4 + 1] = (uint8_t)std::min(int(color.y * 255), 255);
					hpart.pixels[k * 4 + 2] = (uint8_t)std::min(int(color.z * 255), 255);
					hpart.pixels[k * 4 + 3] = (uint8_t)std::min(int(colors[k].Shadow * 255), 255);
				}
			}
			else {
				auto& lpart = item->lpart;
				auto& hpart = item->hpart;
				for (int k = 0; k < colors.size(); ++k) {
					Float3 color = colors[k].Diffuse;
					color /= factor;
					color.saturate();

					uint16_t r = (uint16_t)std::min(int(color.x * 65535), 65535);
					uint16_t g = (uint16_t)std::min(int(color.y * 65535), 65535);
					uint16_t b = (uint16_t)std::min(int(color.z * 65535), 65535);

					hpart.pixels[k * 4 + 0] = (uint8_t)((r >> 8) & 0xFF);
					hpart.pixels[k * 4 + 1] = (uint8_t)((g >> 8) & 0xFF);
					hpart.pixels[k * 4 + 2] = (uint8_t)((b >> 8) & 0xFF);
					hpart.pixels[k * 4 + 3] = (uint8_t)std::min(int(colors[k].Shadow * 255), 255);

					lpart.pixels[k * 4 + 0] = (uint8_t)((r >> 0) & 0xFF);
					lpart.pixels[k * 4 + 1] = (uint8_t)((g >> 0) & 0xFF);
					lpart.pixels[k * 4 + 2] = (uint8_t)((b >> 0) & 0xFF);
					lpart.pixels[k * 4 + 3] = (uint8_t)std::min(int(colors[k].AO * 255), 255);
				}
			}
		}
		else {
			auto& image = item->hpart;
			for (int k = 0; k < colors.size(); ++k) {
				RGBE rgbe = RGBE_FROM(colors[k].Diffuse);
				image.pixels[k * 4 + 0] = rgbe.r;
				image.pixels[k * 4 + 1] = rgbe.g;
				image.pixels[k * 4 + 2] = rgbe.b;
				image.pixels[k * 4 + 3] = rgbe.e;
			}
		}
	}

	void CopyImage(Image& dst, const Image& src, int x, int y)
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

	void SaveImage(const Image& image, const char* filename)
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

	int GetLightmapChannels()
	{
		//mSetting.RGBEFormat ? 4 : 3;
		return 4; // rgb and shadow mask
	}

	void SaveLowpTerrainLightmap(FILE* fp, const String& path, Terrain* terrain, int terrainIdx, float lmap_factor)
	{
		const auto* settings = World::Instance()->GetSetting();
		const int lmapSize = terrain->GetDesc().LMapSize;
		const int ntiles = std::max(1, settings->Size / lmapSize);

		int lmapIndex = 0;
		for (int by = 0; by < terrain->GetDesc().BlockCount.y; by += ntiles) {
			for (int bx = 0; bx < terrain->GetDesc().BlockCount.x; bx += ntiles) {
				const int bw = std::min(ntiles, terrain->GetDesc().BlockCount.x - bx);
				const int bh = std::min(ntiles, terrain->GetDesc().BlockCount.y - by);
				const int dims = std::max(bw * lmapSize, bh * lmapSize);
				const int channels = GetLightmapChannels();

				LFX::Image hpart;
				std::vector<PackedLightmapItem*> packedItems;

				if (1) {
					hpart.width = dims;
					hpart.height = dims;
					hpart.channels = channels;
					hpart.pixels.resize(dims * dims * channels);
					memset(hpart.pixels.data(), 0, dims * dims * channels);

					LOGD("Pack terrain %d blocks %d %d %d %d", terrainIdx, bx, by, bw, bh);
					for (int j = 0; j < bh; ++j) {
						for (int i = 0; i < bw; ++i) {
							std::vector<LightmapValue> colors;
							colors.resize(lmapSize * lmapSize);
							terrain->GetLightingMap(bx + i, by + j, colors);

							PackedLightmapItem* item = new PackedLightmapItem();
							item->factor = lmap_factor;
							ConvertColor(item, lmapSize, lmapSize, colors, *settings);
#if LFX_VERSION >= 35
							item->atlasItem.Factor = item->factor;
#endif
							item->atlasItem.OffsetU = i * lmapSize / (float)dims;
							item->atlasItem.OffsetV = j * lmapSize / (float)dims;
							item->atlasItem.ScaleU = lmapSize / (float)dims;
							item->atlasItem.ScaleV = lmapSize / (float)dims;
							CopyImage(hpart, item->hpart, i * lmapSize, j * lmapSize);
							packedItems.push_back(item);
						}
					}
				}

				LFX::Image& image = hpart;
				char filename[256];
				sprintf(filename, "%s/LFX_Terrain_%04d.png", path.c_str(), lmapIndex);
				SaveImage(image, filename);

				for (int j = 0; j < bh; ++j) {
					for (int i = 0; i < bw; ++i) {
						int blockIndex = (by + j) * terrain->GetDesc().BlockCount.x + (bx + i);

						const auto& item = packedItems[j * bw + i];
						float offset = Terrain::kLMapBorder / (float)(lmapSize);
						float scale = 1 - offset * 2;

						LightMapInfo remapInfo;
						remapInfo.MapIndex = lmapIndex;
						remapInfo.Offset[0] = item->atlasItem.OffsetU + offset * item->atlasItem.ScaleU;
						remapInfo.Offset[1] = item->atlasItem.OffsetV + offset * item->atlasItem.ScaleV;
						remapInfo.Scale = item->atlasItem.ScaleU * scale;
#if LFX_VERSION >= 35
						remapInfo.Factor = item->atlasItem.Factor;
#else
						remapInfo.ScaleV = item.ScaleV * scale;
#endif
						fwrite(&blockIndex, sizeof(int), 1, fp);
						fwrite(&remapInfo, sizeof(LightMapInfo), 1, fp);
					}
				}

				for (auto* item : packedItems) {
					delete item;
				}
				packedItems.clear();

				++lmapIndex;
			}
		}
	}

	void SaveHighpTerrainLightmap(FILE* fp, const String& path, Terrain* terrain, int terrainIdx, float lmap_factor)
	{
		const auto* settings = World::Instance()->GetSetting();
		const int lmapSize = terrain->GetDesc().LMapSize;
		const int ntiles = std::max(1, settings->Size / lmapSize);

		int lmapIndex = 0;
		for (int by = 0; by < terrain->GetDesc().BlockCount.y; by += ntiles) {
			for (int bx = 0; bx < terrain->GetDesc().BlockCount.x; bx += ntiles) {
				const int bw = std::min(ntiles, terrain->GetDesc().BlockCount.x - bx);
				const int bh = std::min(ntiles, terrain->GetDesc().BlockCount.y - by);
				const int dims = std::max(bw * lmapSize, bh * lmapSize);
				const int channels = GetLightmapChannels();

				LFX::Image hparts[2], lparts[2];
				std::vector<PackedLightmapItem*> packedItems;

				// part 1
				if (1) {
					LFX::Image& hpart = hparts[0];
					LFX::Image& lpart = lparts[0];

					hpart.width = dims;
					hpart.height = dims;
					hpart.channels = channels;
					hpart.pixels.resize(dims * dims * channels);
					memset(hpart.pixels.data(), 0, dims * dims * channels);

					LOGD("Pack terrain %d blocks %d %d %d %d", terrainIdx, bx, by, bw, bh);
					for (int j = 0; j < bh; ++j) {
						for (int i = 0; i < bw; ++i) {
							std::vector<LightmapValue> colors;
							colors.resize(lmapSize * lmapSize);
							terrain->GetLightingMap(bx + i, by + j, colors);

							PackedLightmapItem* item = new PackedLightmapItem();
							item->factor = lmap_factor;
							ConvertColor(item, lmapSize, lmapSize, colors, *settings);
#if LFX_VERSION >= 35
							item->atlasItem.Factor = item->factor;
#endif
							item->atlasItem.OffsetU = i * lmapSize / (float)dims;
							item->atlasItem.OffsetV = j * lmapSize / (float)dims;
							item->atlasItem.ScaleU = lmapSize / (float)dims;
							item->atlasItem.ScaleV = lmapSize / (float)dims;
							CopyImage(hpart, item->hpart, i * lmapSize, j * lmapSize);
							if (!item->lpart.pixels.empty()) {
								lpart.width = hpart.width;
								lpart.height = hpart.height;
								lpart.channels = hpart.channels;
								lpart.pixels.resize(lpart.width * lpart.height * lpart.channels, 0);
								CopyImage(lpart, item->lpart, i * lmapSize, j * lmapSize);
							}
							packedItems.push_back(item);
						}
					}

					if (settings->Highp) {
						HTexturePacker hPacker0(hpart.channels);
						hPacker0.Insert(hpart.pixels.data(), hpart.width, hpart.height);
						hPacker0.Insert(lpart.pixels.data(), lpart.width, lpart.height);

						hpart.width = hPacker0.GetWidth();
						hpart.height = hPacker0.GetHeight();
						hpart.pixels = hPacker0.GetBuffer();
					}
				}

				LFX::Image& image = hparts[0];
				char filename[256];
				sprintf(filename, "%s/LFX_Terrain_%04d.png", path.c_str(), lmapIndex);
				SaveImage(image, filename);

				for (int j = 0; j < bh; ++j) {
					for (int i = 0; i < bw; ++i) {
						int blockIndex = (by + j) * terrain->GetDesc().BlockCount.x + (bx + i);

						const auto& item = packedItems[j * bw + i];
						float offset = Terrain::kLMapBorder / (float)(lmapSize);
						float scale = 1 - offset * 2;

						LightMapInfo remapInfo;
						remapInfo.MapIndex = lmapIndex;
						remapInfo.Offset[0] = item->atlasItem.OffsetU + offset * item->atlasItem.ScaleU;
						remapInfo.Offset[1] = item->atlasItem.OffsetV + offset * item->atlasItem.ScaleV;
						remapInfo.Scale = item->atlasItem.ScaleU * scale;
						if (settings->Highp) {
							remapInfo.Scale *= 0.5f;
							remapInfo.Offset.x *= 0.5f;
						}
#if LFX_VERSION >= 35
						remapInfo.Factor = item->atlasItem.Factor;
#else
						remapInfo.ScaleV = item.ScaleV * scale;
#endif
						fwrite(&blockIndex, sizeof(int), 1, fp);
						fwrite(&remapInfo, sizeof(LightMapInfo), 1, fp);
					}
				}

				for (auto* item : packedItems) {
					delete item;
				}
				packedItems.clear();

				++lmapIndex;
			}
		}
	}
	
	void SaveLightmaps(FILE* fp, const String& path)
	{
		const auto* settings = World::Instance()->GetSetting();
		const auto& terrains = World::Instance()->GetTerrains();
		const auto& meshes = World::Instance()->GetMeshes();

		// Pack and save terrain lightmap
		for (int t = 0; t < terrains.size() && !settings->Selected; ++t) {
			Terrain* terrain = terrains[t];
			const int lmapSize = terrain->GetDesc().LMapSize;
			const int numBlocks = terrain->GetDesc().BlockCount.x * terrain->GetDesc().BlockCount.y;

			fwrite(&LFX_FILE_TERRAIN, 4, 1, fp);
			fwrite(&t, 4, 1, fp); // index
			fwrite(&numBlocks, 4, 1, fp);

			float lmap_factor = 1;
			for (int y = 0; y < terrain->GetDesc().BlockCount.y; ++y) {
				for (int x = 0; x < terrain->GetDesc().BlockCount.x; ++x) {
					std::vector<LightmapValue> colors;
					colors.resize(lmapSize * lmapSize);
					terrain->GetLightingMap(x, y, colors);
					if (!settings->RGBEFormat) {
						for (int k = 0; k < colors.size(); ++k) {
							lmap_factor = std::max(colors[k].Diffuse.x, lmap_factor);
							lmap_factor = std::max(colors[k].Diffuse.y, lmap_factor);
							lmap_factor = std::max(colors[k].Diffuse.z, lmap_factor);
						}
					}
				}
			}

			if (settings->Highp) {
				SaveHighpTerrainLightmap(fp, path, terrain, t, lmap_factor);
			}
			else {
				SaveLowpTerrainLightmap(fp, path, terrain, t, lmap_factor);
			}
		}

		// Mesh
		TextureAtlasPacker::Options options;
		options.Width = settings->Size;
		options.Height = settings->Size;
		options.Channels = GetLightmapChannels();
		options.Border = 0;
		options.Space = 0;
		TextureAtlasPacker packer(options);

		std::vector<PackedLightmapItem*> packedItems;
		for (int i = 0; i < meshes.size(); ++i) {
			LFX::Mesh* mesh = meshes[i];
			if (mesh->GetLightingMapSize() == 0) {
				continue;
			}

			LOGD("Pack mesh %d", i);

			const int dims = mesh->GetLightingMapSize();
			auto& colors = mesh->_getLightingMap();

			PackedLightmapItem* item = new PackedLightmapItem();
			ConvertColor(item, dims, dims, colors, *settings);
			colors.clear();
#if 0
			// test
			char filename[256];
			sprintf(filename, "%s/LFX_Mesh_111.png", path.c_str(), i);
			SaveImage(image, filename);
#endif
#if LFX_VERSION >= 35
			item->atlasItem.Factor = item->factor;
#endif
			packer.Insert(item->hpart.pixels.data(), dims, dims, item->atlasItem);
			packedItems.push_back(item);
		}

		const auto& atlases = packer.GetAtlasArray();
		for (int i = 0; i < atlases.size(); ++i) {
			LFX::Image hparts[2], lparts[2];

			if (1) {
				LFX::Image& hpart = hparts[0];
				LFX::Image& lpart = lparts[0];

				hpart.pixels = atlases[i]->Pixels;
				hpart.width = atlases[i]->Width;
				hpart.height = atlases[i]->Height;
				hpart.channels = options.Channels;
				if (settings->Highp) {
					lpart.width = hpart.width;
					lpart.height = hpart.height;
					lpart.channels = hpart.channels;
					lpart.pixels.resize(lpart.width * lpart.height * lpart.channels, 0);

					for (const auto* packItem : packedItems) {
						if (packItem->atlasItem.Index != i) {
							continue;
						}
						if (packItem->lpart.pixels.empty()) {
							continue;
						}

						for (int v = 0; v < packItem->lpart.height; ++v) {
							for (int u = 0; u < packItem->lpart.width; ++u) {
								assert(packItem->atlasItem.Rect.w == packItem->lpart.width
									&& packItem->atlasItem.Rect.h == packItem->lpart.height);
								const int du = packItem->atlasItem.Rect.x + u;
								const int dv = packItem->atlasItem.Rect.y + v;

								const int srcIndex = v * packItem->lpart.width + u;
								const int dstIndex = dv * lpart.width + du;
								lpart.pixels[dstIndex * 4 + 0] = packItem->lpart.pixels[srcIndex * 4 + 0];
								lpart.pixels[dstIndex * 4 + 1] = packItem->lpart.pixels[srcIndex * 4 + 1];
								lpart.pixels[dstIndex * 4 + 2] = packItem->lpart.pixels[srcIndex * 4 + 2];
								lpart.pixels[dstIndex * 4 + 3] = packItem->lpart.pixels[srcIndex * 4 + 3];
							}
						}
					}
				}
			}

			int mapIdx = i;
#if LFX_HPMAP_MERGE
			if (settings->Highp) {
				LFX::Image& hpart = hparts[1];
				LFX::Image& lpart = lparts[1];

				hpart.width = atlases[i]->Width;
				hpart.height = atlases[i]->Height;
				hpart.channels = options.Channels;
				hpart.pixels.resize(hpart.width * hpart.height* hpart.channels, 0);

				lpart.width = hpart.width;
				lpart.height = hpart.height;
				lpart.channels = hpart.channels;
				lpart.pixels.resize(lpart.width * lpart.height * lpart.channels, 0);

				if (++i < (int)atlases.size()) {
					hpart.pixels = atlases[i]->Pixels;
					for (const auto* packItem : packedItems) {
						if (packItem->atlasItem.Index != i) {
							continue;
						}
						if (packItem->lpart.pixels.empty()) {
							continue;
						}

						for (int v = 0; v < packItem->lpart.height; ++v) {
							for (int u = 0; u < packItem->lpart.width; ++u) {
								assert(packItem->atlasItem.Rect.w == packItem->lpart.width
									&& packItem->atlasItem.Rect.h == packItem->lpart.height);
								const int du = packItem->atlasItem.Rect.x + u;
								const int dv = packItem->atlasItem.Rect.y + v;

								const int srcIndex = v * packItem->lpart.width + u;
								const int dstIndex = dv * lpart.width + du;
								lpart.pixels[dstIndex * 4 + 0] = packItem->lpart.pixels[srcIndex * 4 + 0];
								lpart.pixels[dstIndex * 4 + 1] = packItem->lpart.pixels[srcIndex * 4 + 1];
								lpart.pixels[dstIndex * 4 + 2] = packItem->lpart.pixels[srcIndex * 4 + 2];
								lpart.pixels[dstIndex * 4 + 3] = packItem->lpart.pixels[srcIndex * 4 + 3];
							}
						}
					}
				}

				mapIdx /= 2;
			}
#endif
			
			if (settings->Highp) {
				HTexturePacker hPacker0(hparts[0].channels);
				hPacker0.Insert(hparts[0].pixels.data(), hparts[0].width, hparts[0].height);
				hPacker0.Insert(lparts[0].pixels.data(), lparts[0].width, lparts[0].height);

#if LFX_HPMAP_MERGE
				HTexturePacker hPacker1(hparts[1].channels);
				hPacker1.Insert(hparts[1].pixels.data(), hparts[1].width, hparts[1].height);
				hPacker1.Insert(lparts[1].pixels.data(), lparts[1].width, lparts[1].height);

				VTexturePacker vPacker(hparts[0].channels);
				vPacker.Insert(hPacker0.GetBuffer().data(), hPacker0.GetWidth(), hPacker0.GetHeight());
				vPacker.Insert(hPacker1.GetBuffer().data(), hPacker1.GetWidth(), hPacker1.GetHeight());

				hparts[0].width = vPacker.GetWidth();
				hparts[0].height = vPacker.GetHeight();
				hparts[0].pixels = vPacker.GetBuffer();
#else
				hparts[0].width = hPacker0.GetWidth();
				hparts[0].height = hPacker0.GetHeight();
				hparts[0].pixels = hPacker0.GetBuffer();
#endif
			}

			char filename[256];
			sprintf(filename, "%s/LFX_Mesh_%04d.png", path.c_str(), mapIdx);
			SaveImage(hparts[0], filename);
		}

		// chunk
		int numPackItems = packedItems.size();
		if (numPackItems > 0) {
			fwrite(&LFX_FILE_MESH, sizeof(int), 1, fp);
			fwrite(&numPackItems, sizeof(int), 1, fp);

			int packIndex = 0;
			for (int i = 0; i < meshes.size(); ++i) {
				LFX::Mesh* mesh = meshes[i];
				if (mesh->GetLightingMapSize() == 0) {
					continue;
				}

				const auto* item = packedItems[packIndex++];
				const int size = mesh->GetLightingMapSize();
				const float offset = LMAP_BORDER / (float)size;
				const float scale = 1 - offset * 2;

				LightMapInfo remapInfo;
				remapInfo.MapIndex = item->atlasItem.Index;
				remapInfo.Offset[0] = item->atlasItem.OffsetU + offset * item->atlasItem.ScaleU;
				remapInfo.Offset[1] = item->atlasItem.OffsetV + offset * item->atlasItem.ScaleV;
				remapInfo.Scale = item->atlasItem.ScaleU * scale;
				if (settings->Highp) {
					remapInfo.Scale *= 0.5f;
					remapInfo.Offset.x *= 0.5f;
#if LFX_HPMAP_MERGE
					remapInfo.Offset.y *= 0.5f;
					// 下半部分要偏移0.5
					if (remapInfo.MapIndex % 2) {
						remapInfo.Offset.y += 0.5f;
					}
					remapInfo.MapIndex /= 2;
#endif
				}

#if LFX_VERSION >= 35
				remapInfo.Factor = item->atlasItem.Factor;
#else
				remapInfo.ScaleV = item.ScaleV * scale;
#endif
				fwrite(&i, sizeof(int), 1, fp);
				fwrite(&remapInfo, sizeof(LightMapInfo), 1, fp);
			}
		}

		for (auto* item : packedItems) {
			delete item;
		}
		packedItems.clear();
	}

	void SaveLightProbes(FILE* fp)
	{
		const auto& probes = World::Instance()->GetSHProbes();

		const int numSHProbes = probes.size();
		if (numSHProbes > 0) {
			fwrite(&LFX_FILE_SHPROBE, sizeof(int), 1, fp);
			fwrite(&numSHProbes, sizeof(int), 1, fp);
			for (int i = 0; i < probes.size(); ++i) {
				const auto& probe = probes[i];
				const int numCoefs = probe.coefficients.size() * 3;
				fwrite(&probe.position, sizeof(probe.position), 1, fp);
				fwrite(&probe.normal, sizeof(probe.normal), 1, fp);
				fwrite(&numCoefs, sizeof(int), 1, fp);
				fwrite((const float*)probe.coefficients.data(), numCoefs * sizeof(float), 1, fp);
			}
		}
	}

	void World::Save()
	{
		String path = "output";
		//FileUtil::DeleteDir(path);
		FileUtil::MakeDir(path);

		String lfx_file = path + "/lfx.out";
		FILE* fp = fopen(lfx_file.c_str(), "wb");
		if (fp == NULL) {
			LOGE("Can not open file '%s'", lfx_file.c_str());
			return;
		}

		LOGD("Writing lfx file '%s'", lfx_file.c_str());

		fwrite(&LFX_FILE_VERSION, 4, 1, fp);

		if (mSetting.BakeLightMap) {
			SaveLightmaps(fp, path);
		}

		if (mSetting.BakeLightProbe) {
			SaveLightProbes(fp);
		}

		// end
		fwrite(&LFX_FILE_EOF, 4, 1, fp);

		fclose(fp);

		LOGD("Writing lfx file end.");
	}

	void World::Clear()
	{
		for (auto i = 0; i < mTextures.size(); ++i) {
			delete mTextures[i];
		}
		mTextures.clear();

		for (auto i = 0; i < mMeshes.size(); ++i)
		{
			delete mMeshes[i];
		}
		mMeshes.clear();

		for (auto i = 0; i < mLights.size(); ++i)
		{
			delete mLights[i];
		}
		mLights.clear();

		for (int i = 0; i < mTerrains.size(); ++i)
		{
			delete mTerrains[i];
		}
		mTerrains.clear();

		SAFE_DELETE(mScene);
	}

	Texture* World::LoadTexture(const String & filename)
	{
		Texture* tex = GetTexture(filename);
		if (tex != NULL) {
			return tex;
		}

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

		if (img.pixels.empty()) {
			LOGI("Texture '%s' loaded", filename.c_str());
			tex = new Texture;
			tex->name = filename;
			tex->width = img.width;
			tex->height = img.height;
			tex->channels = img.channels;
			tex->data = img.pixels;
			mTextures.push_back(tex);
		}
		else {
			LOGW("Load Texture '%s' failed", filename.c_str());
			// create dummy texture
			tex = new Texture;
			tex->name = filename;
			tex->width = 4;
			tex->height = 4;
			tex->channels = 3;
			tex->data.resize(3 * 4 * 4);
			memset(tex->data.data(), 0, tex->data.size());
			mTextures.push_back(tex);
		}

		return tex;
	}

	Texture* World::CreateTexture(const String & nam, int w, int h, int channels)
	{
		Texture* t = new Texture;
		t->name = nam;
		t->width = w;
		t->height = h;
		t->channels = channels;
		t->data.resize(w * h * channels, 0);
		mTextures.push_back(t);

		return t;
	}

	Texture* World::GetTexture(const String & name)
	{
		for (int i = 0; i < mTextures.size(); ++i) {
			if (mTextures[i]->name == name)
				return mTextures[i];
		}

		return NULL;
	}

	Light* World::CreateLight()
	{
		Light* pLight = new Light;
		mLights.push_back(pLight);
		return pLight;
	}

	SHProbe* World::CreateSHProbe()
	{
		mSHProbes.emplace_back();
		return &mSHProbes.back();
	}

	Mesh* World::CreateMesh()
	{
		Mesh* pMesh = new Mesh();
		mMeshes.push_back(pMesh);
		return pMesh;
	}

	Terrain* World::CreateTerrain(float* heightfield, const Terrain::Desc & desc)
	{
		Terrain* terrain = new Terrain(heightfield, desc);
		mTerrains.push_back(terrain);
		return terrain;
	}

	Light* World::GetMainLight() const
	{
		for (auto* light : mLights) {
			if (light->Type == Light::DIRECTION) {
				return light;
			}
		}

		return nullptr;
	}

	bool _World_AttachMesh(BSPTree<Mesh*>::Node* node, Mesh* mesh)
	{
		Aabb bound = mesh->GetBound();

		if (node->aabb.Contain(bound))
		{
			if (node->child[0] != NULL && _World_AttachMesh(node->child[0], mesh))
				return true;

			if (node->child[1] != NULL && _World_AttachMesh(node->child[1], mesh))
				return true;

			node->elems.push_back(mesh);

			return true;
		}

		return false;
	}

	Aabb _World_Optimize(BSPTree<Mesh *>::Node * node)
	{
		if (node->child[0] == NULL) {
			if (node->elems.size() > 0) {
				Aabb bounds = node->elems[0]->GetBound();
				for (size_t i = 1; i < node->elems.size(); ++i) {
					bounds.Merge(node->elems[i]->GetBound());
				}

				node->aabb = bounds;
			}
		}
		else {
			Aabb bounds = _World_Optimize(node->child[0]);
			bounds.Merge(_World_Optimize(node->child[1]));
			node->aabb = bounds;
		}

		return node->aabb;
	}

	void World::BuildScene()
	{
		LOGI("-: Building meshes %d", (int)mMeshes.size());
		for (auto* mesh : mMeshes) {
			mesh->Build();
		}

		for (size_t i = 0; i < mMeshes.size(); ++i) {
			if (!mMeshes[i]->Valid()) {
				mMeshes.erase(mMeshes.begin() + i--);
			}
		}

		LOGI("-: Building terrain %d", (int)mTerrains.size());
		for (auto* terrain : mTerrains) {
			terrain->Build();
		}

		//#undef LFX_USE_EMBREE_SCENE
#ifdef LFX_USE_EMBREE_SCENE
		LOGI("-: Building embree scene");
		mScene = new EmbreeScene();
#else
		LOGI("-: Building scene");
		mScene = new Scene();
#endif
		mScene->Build();
	}

}