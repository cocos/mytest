#include "LFX_World.h"
#include "LFX_Image.h"
#include "LFX_Stream.h"
#include "LFX_TextureAtlas.h"
#include "LFX_DeviceStats.h"
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
		if (version != LFX_FILE_VERSION) {
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
		stream >> mSetting.GIScale;
		stream >> mSetting.GISamples;
		stream >> mSetting.GIPathLength;
		stream >> mSetting.AOLevel;
		stream >> mSetting.AOStrength;
		stream >> mSetting.AORadius;
		stream >> mSetting.AOColor;
		stream >> mSetting.Threads;
		stream >> mSetting.BakeLightMap;
		stream >> mSetting.BakeLightProbe;
		//mSetting.GIScale = 1.0f;
		// disable gamma correction
		mSetting.Gamma = 1;
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
					stream >> mtl[i].Diffuse;
					String tex = stream.ReadString();
					if (tex != "") {
						mtl[i].DiffuseMap = LoadTexture(tex);
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
				//l->GIEnable = true;
				//l->GIEnable = false;
				if (l->Type == Light::DIRECTION && l->DirectScale == 0) {
					l->SaveShadowMask = true;
				}
				if (!l->CastShadow) {
					l->GIEnable = false;
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

	void ConvertColor(Image& image, float& factor, std::vector<Float4>& colors, const World::Settings& settings)
	{
#if LFX_ENABLE_GAMMA
		if (settings.Gamma != 1.0f) {
			for (int k = 0; k < colors.size(); ++k)
			{
				colors[k] = Pow(colors[k], 1.0f / settings.Gamma);
			}
		}
#endif

		image.pixels.resize(image.width * image.height * image.channels);
		if (!settings.RGBEFormat) {
			for (int k = 0; k < colors.size(); ++k)
			{
				factor = std::max(colors[k].x, factor);
				factor = std::max(colors[k].y, factor);
				factor = std::max(colors[k].z, factor);
			}

			for (int k = 0; k < colors.size(); ++k)
			{
				Float3 color = Float3(colors[k].x, colors[k].y, colors[k].z);
				color /= factor;
				color.saturate();

				image.pixels[k * 4 + 0] = (uint8_t)std::min(int(color.x * 255), 255);
				image.pixels[k * 4 + 1] = (uint8_t)std::min(int(color.y * 255), 255);
				image.pixels[k * 4 + 2] = (uint8_t)std::min(int(color.z * 255), 255);
				image.pixels[k * 4 + 3] = (uint8_t)std::min(int(colors[k].w * 255), 255);
			}
		}
		else {
			for (int k = 0; k < colors.size(); ++k)
			{
				RGBE rgbe = RGBE_FROM(Float3(colors[k].x, colors[k].y, colors[k].z));

				image.pixels[k * 4 + 0] = rgbe.r;
				image.pixels[k * 4 + 1] = rgbe.g;
				image.pixels[k * 4 + 2] = rgbe.b;
				image.pixels[k * 4 + 3] = rgbe.e;
			}
		}
	}

	void CopyImage(Image& dst, const Image& src, int x, int y)
	{
		for (int j = 0; j < src.height; ++j)
		{
			for (int i = 0; i < src.width; ++i)
			{
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

	int GetLightmapChannels()
	{
		//mSetting.RGBEFormat ? 4 : 3;
		return 4; // rgb and shadow mask
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

		// Pack and save terrain lightmap
		for (int t = 0; t < mTerrains.size() && !mSetting.Selected; ++t) {
			Terrain* terrain = mTerrains[t];
			int lmap_index = 0;
			int lmap_size = terrain->GetDesc().LMapSize;
			int ntiles = std::max(1, mSetting.Size / lmap_size);
			int nblocks = terrain->GetDesc().BlockCount.x * terrain->GetDesc().BlockCount.y;

			fwrite(&LFX_FILE_TERRAIN, 4, 1, fp);
			fwrite(&t, 4, 1, fp); // index
			fwrite(&nblocks, 4, 1, fp);

			float lmap_factor = 1;
			for (int y = 0; y < terrain->GetDesc().BlockCount.y; ++y)
			{
				for (int x = 0; x < terrain->GetDesc().BlockCount.x; ++x)
				{
					std::vector<Float4> colors;
					colors.resize(lmap_size * lmap_size);
					terrain->GetLightingMap(x, y, colors);
					if (!mSetting.RGBEFormat)
					{
						for (int k = 0; k < colors.size(); ++k)
						{
							lmap_factor = std::max(colors[k].x, lmap_factor);
							lmap_factor = std::max(colors[k].y, lmap_factor);
							lmap_factor = std::max(colors[k].z, lmap_factor);
						}
					}
				}
			}

			for (int y = 0; y < terrain->GetDesc().BlockCount.y; y += ntiles)
			{
				for (int x = 0; x < terrain->GetDesc().BlockCount.x; x += ntiles)
				{
					const int w = std::min(ntiles, terrain->GetDesc().BlockCount.x - x);
					const int h = std::min(ntiles, terrain->GetDesc().BlockCount.y - y);
					const int dims = std::max(w * lmap_size, h * lmap_size);
					const int channels = GetLightmapChannels();

					LFX::Image image;
					image.pixels.resize(dims * dims * channels);
					image.width = dims;
					image.height = dims;
					image.channels = channels;
					memset(image.pixels.data(), 0, dims * dims * channels);

					LOGD("Pack terrain %d blocks %d %d %d %d", t, x, y, w, h);
					std::vector<TextureAtlasPacker::Item> packed_items;
					for (int j = 0; j < h; ++j)
					{
						for (int i = 0; i < w; ++i)
						{
							std::vector<Float4> colors;
							colors.resize(lmap_size * lmap_size);
							terrain->GetLightingMap(x + i, y + j, colors);

							float factor = lmap_factor;
							Image temp;
							temp.width = lmap_size;
							temp.height = lmap_size;
							temp.channels = channels;
							ConvertColor(temp, factor, colors, mSetting);

							TextureAtlasPacker::Item item;
#if LFX_VERSION >= 35
							item.Factor = factor;
#endif
							item.OffsetU = i * lmap_size / (float)dims;
							item.OffsetV = j * lmap_size / (float)dims;
							item.ScaleU = lmap_size / (float)dims;
							item.ScaleV = lmap_size / (float)dims;
							CopyImage(image, temp, i * lmap_size, j * lmap_size);
							packed_items.push_back(item);
						}
					}

					char filename[256];
					sprintf(filename, "%s/LFX_Terrain_%04d.png", path.c_str(), lmap_index);
					LOGD("Save lighting map %s", filename);
					FILE* tfp = fopen(filename, "wb");
					LFX::PNG_Save(tfp, image);
					fclose(tfp);

					for (int j = 0; j < h; ++j)
					{
						for (int i = 0; i < w; ++i)
						{
							int block_index = (y + j) * terrain->GetDesc().BlockCount.x + (x + i);

							const auto& item = packed_items[j * w + i];
							float offset = Terrain::kLMapBorder / (float)(lmap_size);
							float scale = 1 - offset * 2;

							LightMapInfo remapInfo;
							remapInfo.MapIndex = lmap_index;
							remapInfo.Offset[0] = item.OffsetU + offset * item.ScaleU;
							remapInfo.Offset[1] = item.OffsetV + offset * item.ScaleV;
							remapInfo.Scale = item.ScaleU * scale;
#if LFX_VERSION >= 35
							remapInfo.Factor = item.Factor;
#else
							remapInfo.ScaleV = item.ScaleV * scale;
#endif
							fwrite(&block_index, sizeof(int), 1, fp);
							fwrite(&remapInfo, sizeof(LightMapInfo), 1, fp);
						}
					}

					++lmap_index;
				}
			}
		}

		// Mesh
		TextureAtlasPacker::Options options;
		options.Width = mSetting.Size;
		options.Height = mSetting.Size;
		options.Channels = GetLightmapChannels();
		options.Border = 0;
		options.Space = 0;
		TextureAtlasPacker packer(options);

		std::vector<TextureAtlasPacker::Item> packed_items;
		for (int i = 0; i < mMeshes.size(); ++i)
		{
			LFX::Mesh* mesh = mMeshes[i];
			if (mesh->GetLightingMapSize() == 0) {
				continue;
			}

			LOGD("Pack mesh %d", i);

			const int dims = mesh->GetLightingMapSize();
			std::vector<Float4>& colors = mesh->_getLightingMap();

			float factor = 1.0f;
			LFX::Image image;
			image.width = dims;
			image.height = dims;
			image.channels = options.Channels;
			ConvertColor(image, factor, colors, mSetting);
			colors.clear();
#if 0
			// test
			char filename[256];
			sprintf(filename, "%s/LFX_Mesh_111.png", path.c_str(), i);
			FILE* tfp = fopen(filename, "wb");
			LFX::PNG_Save(tfp, image);
			fclose(tfp);
#endif
			TextureAtlasPacker::Item item;
#if LFX_VERSION >= 35
			item.Factor = factor;
#endif
			packer.Insert(image.pixels.data(), image.width, image.height, item);
			packed_items.push_back(item);
		}

		auto atlas = packer.GetAtlasArray();
		for (int i = 0; i < atlas.size(); ++i) {
			LFX::Image image;
			image.pixels = atlas[i]->Pixels;
			image.width = atlas[i]->Width;
			image.height = atlas[i]->Height;
			image.channels = options.Channels;

			char filename[256];
			sprintf(filename, "%s/LFX_Mesh_%04d.png", path.c_str(), i);
			LOGD("Save lighting map %s", filename);
			FILE* tfp = fopen(filename, "wb");
			LFX::PNG_Save(tfp, image);
			fclose(tfp);
		}

		// chunk
		int numPackItems = packed_items.size();
		if (numPackItems > 0) {
			fwrite(&LFX_FILE_MESH, sizeof(int), 1, fp);
			fwrite(&numPackItems, sizeof(int), 1, fp);

			int packIndex = 0;
			for (int i = 0; i < mMeshes.size(); ++i) {
				LFX::Mesh* mesh = mMeshes[i];
				if (mesh->GetLightingMapSize() == 0) {
					continue;
				}

				const TextureAtlasPacker::Item& item = packed_items[packIndex++];
				int size = mesh->GetLightingMapSize();
				float offset = LMAP_BORDER / (float)size;
				float scale = 1 - offset * 2;

				LightMapInfo remapInfo;
				remapInfo.MapIndex = item.Index;
				remapInfo.Offset[0] = item.OffsetU + offset * item.ScaleU;
				remapInfo.Offset[1] = item.OffsetV + offset * item.ScaleV;
				remapInfo.Scale = item.ScaleU * scale;
#if LFX_VERSION >= 35
				remapInfo.Factor = item.Factor;
#else
				remapInfo.ScaleV = item.ScaleV * scale;
#endif
				fwrite(&i, sizeof(int), 1, fp);
				fwrite(&remapInfo, sizeof(LightMapInfo), 1, fp);
			}
		}

		// SH Probe
		int numSHProbes = mSHProbes.size();
		if (numSHProbes > 0) {
			fwrite(&LFX_FILE_SHPROBE, sizeof(int), 1, fp);
			fwrite(&numSHProbes, sizeof(int), 1, fp);

			for (int i = 0; i < mSHProbes.size(); ++i) {
				const auto& probe = mSHProbes[i];
				const int numCoefs = probe.coefficients.size() * 3;
				fwrite(&probe.position, sizeof(probe.position), 1, fp);
				fwrite(&probe.normal, sizeof(probe.normal), 1, fp);
				fwrite(&numCoefs, sizeof(int), 1, fp);
				fwrite((const float*)probe.coefficients.data(), numCoefs * sizeof(float), 1, fp);
			}
		}

		// end
		fwrite(&LFX_FILE_EOF, 4, 1, fp);

		fclose(fp);

		LOGD("Writing lfx file end.");
	}

	void World::Clear()
	{
		for (auto i = 0; i < mThreads.size(); ++i)
		{
			mThreads[i]->Stop();
			delete mThreads[i];
		}
		mThreads.clear();

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
		FileStream fs(filename.c_str());

		if (PNG_Test(fs)) {
			PNG_Load(img, fs);
		}
		else if (JPG_Test(fs)) {
			JPG_Load(img, fs);
		}
		else if (TGA_Test(fs)) {
			TGA_Load(img, fs);
		}
		else if (BMP_Test(fs)) {
			BMP_Load(img, fs);
		}

		if (img.pixels.empty()) {
			LOGW("Load Texture '%s' failed", filename.c_str());
			return NULL;
		}

		LOGW("Texture '%s' loaded", filename.c_str());

		tex = new Texture;
		tex->name = filename;
		tex->width = img.width;
		tex->height = img.height;
		tex->channels = img.channels;
		tex->data = img.pixels;
		mTextures.push_back(tex);

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
		if (node->child[0] == NULL)
		{
			if (node->elems.size() > 0) {
				Aabb bound = node->elems[0]->GetBound();
				for (size_t i = 1; i < node->elems.size(); ++i) {
					bound.Merge(node->elems[i]->GetBound());
				}

				node->aabb = bound;
			}
		}
		else
		{
			Aabb bound = _World_Optimize(node->child[0]);
			bound.Merge(_World_Optimize(node->child[1]));
			node->aabb = bound;
		}

		return node->aabb;
	}

	void World::Build()
	{
		LOGI("-:Building meshes");
		for (size_t i = 0; i < mMeshes.size(); ++i) {
			mMeshes[i]->Build();
		}

		for (size_t i = 0; i < mMeshes.size(); ++i) {
			if (!mMeshes[i]->Valid()) {
				delete mMeshes[i];
				mMeshes.erase(mMeshes.begin() + i--);
			}
		}

		LOGI("-:Building terrain");
		for (int i = 0; i < mTerrains.size(); ++i) {
			mTerrains[i]->Build();
		}

//#undef LFX_USE_EMBREE_SCENE
#ifdef LFX_USE_EMBREE_SCENE
		LOGI("-:Building embree scene");
		mScene = new EmbreeScene;
#else
		LOGI("-:Building scene");
		mScene = new Scene;
#endif
		mScene->Build();
	}

	void World::Start()
	{
		for (size_t i = 0; i < mThreads.size(); ++i) {
			delete[] mThreads[i];
		}
		mThreads.clear();

		mTaskIndex = 0;
		mProgress = 0;

#ifdef _DEBUG
		mSetting.Threads = 1;
#elif LFX_MULTI_THREAD
		DeviceStats stats = DeviceStats::GetStats();
		mSetting.Threads = std::max(1, stats.Processors - 2);
#endif
		for (int i = 0; i < mSetting.Threads; ++i) {
			mThreads.push_back(new STBaker(i));
		}

		mTasks.clear();
		if (mSetting.BakeLightMap) {
			for (size_t i = 0; i < mMeshes.size(); ++i) {
				if (mMeshes[i]->GetLightingMapSize()) {
					mTasks.push_back({ mMeshes[i], (int)i });
				}
			}

			for (auto terrain : mTerrains) {
				for (int i = 0; i < terrain->GetDesc().BlockCount.x * terrain->GetDesc().BlockCount.y; ++i) {
					if (terrain->_getBlockValids()[i]) {
						mTasks.push_back({ terrain, i });
					}
				}
			}
		}
		
		if (mSetting.BakeLightProbe) {
			for (size_t i = 0; i < mSHProbes.size(); ++i) {
				mTasks.push_back({ &mSHProbes[i], (int)i });
			}
		}
		

		for (size_t i = 0; i < mThreads.size(); ++i) {
			LOGI("-: Starting thread %d", i);
			mThreads[i]->Start();
		}
	}

	bool World::End()
	{
		return mThreads.empty();
	}

	void World::UpdateTask()
	{
		STBaker* thread = GetFreeThread();
		if (thread == nullptr) {
			return;
		}

		STBaker::Task task;
		if (GetNextTask(task)) {
			thread->Enqueue(task.entity, task.index);
			return;
		}

		// ensure all task finished
		for (size_t i = 0; i < mThreads.size(); ++i) {
			if (!mThreads[i]->IsCompeleted()) {
				return;
			}
		}

		// end
		for (size_t i = 0; i < mThreads.size(); ++i) {
			mThreads[i]->Stop();
			delete mThreads[i];
		}
		mThreads.clear();

		mTasks.clear();
		mTaskIndex = 0;
	}

	bool World::GetNextTask(STBaker::Task& task)
	{
		if (mTaskIndex < mTasks.size()) {
			task = mTasks[mTaskIndex++];
			return true;
		}

		return false;
	}

	STBaker* World::GetFreeThread()
	{
		for (size_t i = 0; i < mThreads.size(); ++i) {
			if (mThreads[i]->IsCompeleted()) {
				return mThreads[i];
			}
		}

		return nullptr;
	}

}