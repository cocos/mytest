#include "LFX_World.h"
#include "LFX_EmbreeScene.h"
#include "LFX_Stream.h"
#include "LFX_Image.h"
#include "LFX_TextureAtlas.h"

namespace LFX {

	ImplementSingleton(World);

	World::World()
	{
		mEmbreeScene = NULL;
	}

	World::~World()
	{
		Clear();
	}

	bool World::Load()
	{
		String filename = "tmp/lfx.i";

		FileStream stream(filename.c_str());
		if (!stream.IsOpen()) {
			return false;
		}

		// Check verison
		int version;
		stream >> version;
		if (version != LFX_FILE_VERSION) {
			return false;
		}

		String name = stream.ReadString();

		// Load setting
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

		int ckId = 0;
		while (stream.Read(&ckId, sizeof(int))) {
			if (ckId == 0) {
				break;
			}

			switch (ckId) {
			case LFX_FILE_TERRAIN: {
				Float3 pos;
				Terrain::Desc desc;
				stream >> pos;
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

				CreateTerrain(pos, &heights[0], desc);
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
					stream >> mtl[i].diffuse;
					String tex = stream.ReadString();
					if (tex != "") {
						mtl[i].texture = LoadTexture(tex);
					}
				}
				m->Unlock();

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
				break;
			}
			default:
				LOGW("Unknown chunk %d", ckId);
				return false;
			};
		}

		return true;
	}

	void World::Save()
	{
		String path = "output";
		//FileUtil::DeleteDir(path);
		FileUtil::MakeDir(path);

		String lfx_file = path + "/lfx.o";
		FILE * fp = fopen(lfx_file.c_str(), "wb");
		if (fp == NULL) {
			LOGE("Can not open file '%s'", lfx_file.c_str());
			return;
		}

		//
		fwrite(&LFX_FILE_VERSION, 4, 1, fp);

		// Pack and save terrain lightmap

		for (int t = 0; t < mTerrains.size() && !mSetting.Selected; ++t) {
			Terrain* terrain = mTerrains[t];
			int LSize = terrain->GetDesc().LMapSize;

			TextureAtlasPacker::Options options;
			options.Width = mSetting.Size;
			options.Height = mSetting.Size;
			options.Channels = mSetting.RGBEFormat ? 4 : 3;
			options.Border = 0;
			options.Space = 0;

			TextureAtlasPacker packer(options);
			std::vector<TextureAtlasPacker::Item> packed_items;
			for (int j = 0; j < terrain->GetDesc().BlockCount.y; ++j)
			{
				for (int i = 0; i < terrain->GetDesc().BlockCount.x; ++i)
				{
					std::vector<Float3> colors;
					colors.resize(LSize * LSize);

					terrain->GetLightingMap(i, j, colors);
					for (int k = 0; k < colors.size(); ++k)
					{
						colors[k] = Pow(colors[k], 1.0f / mSetting.Gamma);
						Saturate(colors[k]);
					}

					Image image;
					image.width = LSize;
					image.height = LSize;
					if (!mSetting.RGBEFormat) {
						image.pixels = new uint8_t[LSize * LSize * 3];
						image.channels = 3;
						for (int k = 0; k < colors.size(); ++k)
						{
							colors[k].saturate();

							image.pixels[k * 3 + 0] = (uint8_t)(colors[k].x * 255);
							image.pixels[k * 3 + 1] = (uint8_t)(colors[k].y * 255);
							image.pixels[k * 3 + 2] = (uint8_t)(colors[k].z * 255);
						}
					}
					else {
						image.pixels = new uint8_t[LSize * LSize * 4];
						image.channels = 4;
						for (int k = 0; k < colors.size(); ++k)
						{
							RGBE rgbe = RGBE_FROM(colors[k]);

							image.pixels[k * 4 + 0] = rgbe.r;
							image.pixels[k * 4 + 1] = rgbe.g;
							image.pixels[k * 4 + 2] = rgbe.b;
							image.pixels[k * 4 + 3] = rgbe.e;
						}
					}

					TextureAtlasPacker::Item item;
					packer.Insert(image.pixels, image.width, image.height, item);
					packed_items.push_back(item);
				}
			}

			auto atlas = packer.GetAtlasArray();
			for (int i = 0; i < atlas.size(); ++i)
			{
				LFX::Image image;
				image.pixels = &atlas[i]->Pixels[0];
				image.width = atlas[i]->Width;
				image.height = atlas[i]->Height;
				image.channels = 3;

				char filename[256];
				sprintf(filename, "%s/LFX_Terrain_%04d.png", path.c_str(), i);
				FILE * tfp = fopen(filename, "wb");
				LFX::PNG_Save(tfp, image);
				fclose(tfp);

				image.pixels = NULL;
			}

			// chunk
			fwrite(&LFX_FILE_TERRAIN, 4, 1, fp);
			fwrite(&t, 4, 1, fp); // index

			int numOfBlocks = terrain->GetDesc().BlockCount.x * terrain->GetDesc().BlockCount.y;
			fwrite(&numOfBlocks, 4, 1, fp);

			int blockIdx = 0;
			for (int j = 0; j < terrain->GetDesc().BlockCount.y; ++j)
			{
				for (int i = 0; i < terrain->GetDesc().BlockCount.x; ++i)
				{
					const auto & item = packed_items[blockIdx++];
					float offset = Terrain::kLMapBorder / (float)(LSize);
					float scale = 1 - offset * 2;

					LightMapInfo remapInfo;
					remapInfo.index = item.index;
					remapInfo.offset[0] = item.offsetU + offset * item.scaleU;
					remapInfo.offset[1] = item.offsetV + offset * item.scaleV;
					remapInfo.scale[0] = item.scaleU * scale;
					remapInfo.scale[1] = item.scaleV * scale;
					fwrite(&remapInfo, sizeof(LightMapInfo), 1, fp);
				}
			}
		}

		// Mesh
		TextureAtlasPacker::Options options;
		options.Width = mSetting.Size;
		options.Height = mSetting.Size;
		options.Channels = mSetting.RGBEFormat ? 4 : 3;
		options.Border = 1;
		options.Space = 0;
		TextureAtlasPacker packer(options);

		std::vector<TextureAtlasPacker::Item> packed_items;
		for (int i = 0; i < mMeshes.size(); ++i)
		{
			LFX::Mesh * mesh = LFX::World::Instance()->GetMesh(i);
			if (mesh->GetLightingMapSize() == 0)
				continue;

			std::vector<LFX::Float3> & colors = mesh->_getLightingMap();
			for (int k = 0; k < colors.size(); ++k)
			{
				colors[k] = Pow(colors[k], 1.0f / World::Instance()->GetSetting()->Gamma);
			}

			int LSize = mesh->GetLightingMapSize();

			LFX::Image image;
			image.width = LSize;
			image.height = LSize;
			if (!mSetting.RGBEFormat) {
				image.pixels = new uint8_t[LSize * LSize * 3];
				image.channels = 3;
				for (int k = 0; k < colors.size(); ++k)
				{
					colors[k].saturate();

					image.pixels[k * 3 + 0] = (uint8_t)(colors[k].x * 255);
					image.pixels[k * 3 + 1] = (uint8_t)(colors[k].y * 255);
					image.pixels[k * 3 + 2] = (uint8_t)(colors[k].z * 255);
				}
			}
			else {
				image.pixels = new uint8_t[LSize * LSize * 4];
				image.channels = 4;
				for (int k = 0; k < colors.size(); ++k)
				{
					RGBE rgbe = RGBE_FROM(colors[k]);

					image.pixels[k * 4 + 0] = rgbe.r;
					image.pixels[k * 4 + 1] = rgbe.g;
					image.pixels[k * 4 + 2] = rgbe.b;
					image.pixels[k * 4 + 3] = rgbe.e;
				}
			}
			colors.clear();

			TextureAtlasPacker::Item item;
			packer.Insert(image.pixels, image.width, image.height, item);
			packed_items.push_back(item);
		}

		auto atlas = packer.GetAtlasArray();
		for (int i = 0; i < atlas.size(); ++i)
		{
			LFX::Image image;
			image.pixels = &atlas[i]->Pixels[0];
			image.width = atlas[i]->Width;
			image.height = atlas[i]->Height;
			image.channels = 3;

			char filename[256];
			sprintf(filename, "%s/LFX_Mesh_%04d.png", path.c_str(), i);
			FILE * tfp = fopen(filename, "wb");
			LFX::PNG_Save(tfp, image);
			fclose(tfp);

			image.pixels = NULL;
		}

		// chunk
		int numPackItems = packed_items.size();
		if (numPackItems > 0) {
			fwrite(&LFX_FILE_MESH, sizeof(int), 1, fp);
			fwrite(&numPackItems, sizeof(int), 1, fp);

			int packIndex = 0;
			for (int i = 0; i < mMeshes.size(); ++i)
			{
				LFX::Mesh * mesh = LFX::World::Instance()->GetMesh(i);
				if (mesh->GetLightingMapSize() == 0)
					continue;

				const TextureAtlasPacker::Item & item = packed_items[packIndex++];
				int size = mesh->GetLightingMapSize();
#if 0
				float offset = LFX_LMAP_SPACE / (float)size;
				float scale = 1 - offset * 2;

				OutLightMapInfo remapInfo;
				remapInfo.index = item.index;
				remapInfo.offset[0] = item.offsetU + offset * item.scaleU;
				remapInfo.offset[1] = item.offsetV + offset * item.scaleV;
				remapInfo.scale[0] = item.scaleU * scale;
				remapInfo.scale[1] = item.scaleV * scale;
#else
				LightMapInfo remapInfo;
				remapInfo.index = item.index;
				remapInfo.offset[0] = item.offsetU;
				remapInfo.offset[1] = item.offsetV;
				remapInfo.scale[0] = item.scaleU;
				remapInfo.scale[1] = item.scaleV;
#endif
				fwrite(&i, sizeof(int), 1, fp);
				fwrite(&remapInfo, sizeof(LightMapInfo), 1, fp);
			}
		}

		// end
		fwrite(&LFX_FILE_EOF, 4, 1, fp);

		fclose(fp);
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

		if (mEmbreeScene)
			delete mEmbreeScene;
		mEmbreeScene = NULL;
	}

	Texture * World::LoadTexture(const String & name)
	{
		Texture* tex = GetTexture(name);
		if (tex != NULL) {
			return tex;
		}

		char filename[256];
		for (int i = 0; i < strlen(filename); ++i) {
			if (filename[i] == '/') {
				filename[i] = '$';
			}
		}

		String imagefile = String("lightfx/") + filename;

		Image img;
		FileStream fs(imagefile.c_str());
		if (!PNG_Load(img, fs)) {
			LOGW("Load Texture '%s' failed", name.c_str());
			return NULL;
		}

		tex = new Texture;
		tex->name = name;
		tex->width = img.width;
		tex->height = img.height;
		tex->channels = img.channels;
		tex->data = img.pixels;
		mTextures.push_back(tex);

		img.pixels = NULL;

		return tex;
	}

	Texture * World::CreateTexture(const String & nam, int w, int h, int channels, unsigned char *data)
	{
		Texture * t = new Texture;
		t->name = nam;
		t->width = w;
		t->height = h;
		t->channels = channels;
		t->data = data;
		mTextures.push_back(t);

		return t;
	}

	Texture * World::GetTexture(const String & name)
	{
		for (int i = 0; i < mTextures.size(); ++i) {
			if (mTextures[i]->name == name)
				return mTextures[i];
		}

		return NULL;
	}

	Light * World::CreateLight()
	{
		Light * pLight = new Light;

		mLights.push_back(pLight);

		return pLight;
	}

	Mesh * World::CreateMesh()
	{
		Mesh * pMesh = new Mesh();

		mMeshes.push_back(pMesh);

		return pMesh;
	}

	Terrain * World::CreateTerrain(const Float3& pos, float* heightfield, const Terrain::Desc & desc)
	{
		Terrain* terrain = new Terrain(pos, heightfield, desc);

		mTerrains.push_back(terrain);

		return terrain;
	}

	bool _World_AttachMesh(BSPTree<Mesh *>::Node * node, Mesh * mesh)
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
			if (node->elems.size() > 0)
			{
				Aabb bound = node->elems[0]->GetBound();
				for (size_t i = 1; i < node->elems.size(); ++i)
				{
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
		for (size_t i = 0; i < mMeshes.size(); ++i)
		{
			mMeshes[i]->Build();
		}

		for (size_t i = 0; i < mMeshes.size(); ++i)
		{
			if (!mMeshes[i]->Valid())
			{
				delete mMeshes[i];
				mMeshes.erase(mMeshes.begin() + i--);
			}
		}

		Aabb worldBound;
		worldBound.Invalid();

		for (size_t i = 0; i < mMeshes.size(); ++i)
		{
			Aabb bound = mMeshes[i]->GetBound();

			worldBound.Merge(bound);
		}

		if (worldBound.Valid())
		{
			mBSPTree.Build(worldBound, 8);
			for (size_t i = 0; i < mMeshes.size(); ++i)
			{
				_World_AttachMesh(mBSPTree.RootNode(), mMeshes[i]);
			}

			_World_Optimize(mBSPTree.RootNode());
		}

		LOGI("-:Building terrain");
		for (int i = 0; i < mTerrains.size(); ++i) {
			mTerrains[i]->Build();
		}

		//
		LOGI("-:Building bv scene");
		mEmbreeScene = new EmbreeScene;
		mEmbreeScene->Build();

		mStage = STAGE_END;
	}

	void World::Start()
	{
		for (size_t i = 0; i < mThreads.size(); ++i)
		{
			delete[] mThreads[i];
		}
		mThreads.clear();

		mStage = STAGE_DIRECT_LIGHTING;
		mIndex = 0;
		mProgress = 0;

		for (int i = 0; i < mSetting.Threads; ++i)
		{
			mThreads.push_back(new LFX_Baker(i));
		}

		mEntitys.clear();
		for (size_t i = 0; i < mMeshes.size(); ++i)
		{
			if (mMeshes[i]->GetLightingMapSize())
			{
				mEntitys.push_back({ mMeshes[i], (int)i });
			}
		}
		for (auto terrain : mTerrains) {
			for (int i = 0; i < terrain->GetDesc().BlockCount.x * terrain->GetDesc().BlockCount.y; ++i)
			{
				if (terrain->_getBlockValids()[i])
				{
					mEntitys.push_back({ terrain, i });
				}
			}
		}

		for (size_t i = 0; i < mThreads.size(); ++i)
		{
			mThreads[i]->Start();
		}
	}

	int World::UpdateStage()
	{
		bool Compeleted = false;

		if (mStage >= STAGE_DIRECT_LIGHTING || mStage <= STAGE_POST_PROCESS)
		{
			if (mIndex == mEntitys.size())
			{
				Compeleted = true;
				for (size_t i = 0; i < mThreads.size(); ++i)
				{
					Compeleted &= mThreads[i]->IsCompeleted();
				}
			}
			else
			{
				for (size_t i = 0; i < mThreads.size() && mIndex < mEntitys.size(); ++i)
				{
					if (mThreads[i]->IsCompeleted())
					{
						mThreads[i]->Enqueue(mEntitys[mIndex].entity, mEntitys[mIndex].index, mStage);
						++mIndex;
					}
				}
			}
		}

		if (Compeleted)
		{
			mStage = mStage + 1;
			mIndex = 0;
			mProgress = 0;
		}

		if (mStage == STAGE_END)
		{
			for (size_t i = 0; i < mThreads.size(); ++i)
			{
				mThreads[i]->Stop();
				delete mThreads[i];
			}
			mThreads.clear();

			mEntitys.clear();
		}

		return mStage;
	}

	void _rayCheck(Contact & contract, BSPTree<Mesh *>::Node * node, const Ray & ray, float len)
	{
		float dist = 0;

		if (!Intersect(ray, &dist, node->aabb) || contract.td < dist)
			return;

		for (size_t i = 0; i < node->elems.size(); ++i)
		{
			node->elems[i]->RayCheck(contract, ray, len);
		}

		if (node->child[0] != NULL)
		{
			_rayCheck(contract, node->child[0], ray, len);
		}

		if (node->child[1] != NULL)
		{
			_rayCheck(contract, node->child[1], ray, len);
		}
	}

	bool World::RayCheck(Contact & contract, const Ray & ray, float len, int flags)
	{
		return mEmbreeScene->RayCheck(contract, ray, len, flags);
	}

	bool World::_RayCheckImp(Contact & contract, const Ray & ray, float len, int flags)
	{
		contract.td = FLT_MAX;
		contract.tu = 0;
		contract.tv = 0;
		contract.triIndex = -1;
		contract.entity = NULL;
		contract.backFacing = false;

		if ((flags & LFX_MESH) && mBSPTree.RootNode() != NULL)
			_rayCheck(contract, mBSPTree.RootNode(), ray, len);
		if ((flags & LFX_TERRAIN) && mTerrains.size() > 0) {
			for (int i = 0; i < mTerrains.size(); ++i) {
				mTerrains[i]->RayCheck(contract, ray, len);
			}
		}

		if (contract.entity != NULL)
		{
			Vertex a, b, c;
			if (contract.entity->GetType() == LFX_TERRAIN)
			{
				Terrain* terrain = (Terrain*)contract.entity;

				Triangle tri = terrain->_getTriangle(contract.triIndex);
				a = terrain->_getVertex(tri.Index0);
				b = terrain->_getVertex(tri.Index1);
				c = terrain->_getVertex(tri.Index2);
			}
			else
			{
				Mesh * mesh = (Mesh *)contract.entity;

				Triangle tri = mesh->_getTriangle(contract.triIndex);
				a = mesh->_getVertex(tri.Index0);
				b = mesh->_getVertex(tri.Index1);
				c = mesh->_getVertex(tri.Index2);
			}

			Float3 triNml = Float3::Normalize(Float3::Cross(c.Position - a.Position, b.Position - a.Position));
			contract.backFacing = Float3::Dot(triNml, ray.dir) >= 0.0f;
		}

		return contract.entity != NULL;
	}

	bool _occluded(BSPTree<Mesh *>::Node * node, const Ray & ray, float len)
	{
		float dist = 0;

		if (!Intersect(ray, &dist, node->aabb))
			return false;

		for (size_t i = 0; i < node->elems.size(); ++i)
		{
			if (node->elems[i]->Occluded(ray, len))
				return true;
		}

		if (node->child[0] != NULL)
		{
			if (_occluded(node->child[0], ray, len))
				return true;
		}

		if (node->child[1] != NULL)
		{
			if (_occluded(node->child[1], ray, len))
				return true;
		}

		return false;
	}

	bool World::Occluded(const Ray & ray, float len, int flags)
	{
		return mEmbreeScene->Occluded(ray.orig, ray.dir, len, flags);
	}

	bool World::_OccludedImp(const Ray & ray, float len, int flags)
	{
		if ((flags & LFX_MESH) && mBSPTree.RootNode() != NULL) {
			if (_occluded(mBSPTree.RootNode(), ray, len)) {
				return true;
			}
		}
		if ((flags & LFX_TERRAIN)) {
			for (int i = 0; i < mTerrains.size(); ++i) {
				if (mTerrains[i]->Occluded(ray, len)) {
					return true;
				}
			}
		}

		return false;
	}

}