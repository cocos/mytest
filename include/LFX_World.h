#pragma once

#include "LFX_Types.h"
#include "LFX_Light.h"
#include "LFX_Mesh.h"
#include "LFX_Terrain.h"
#include "LFX_Baker.h"
#include "LFX_Rasterizer.h"
#include "LFX_RasterizerSoft.h"

namespace LFX {

	static const int LFX_FILE_VERSION = 0x2000;
	static const int LFX_FILE_TERRAIN = 0x01;
	static const int LFX_FILE_MESH = 0x02;
	static const int LFX_FILE_LIGHT = 0x03;
	static const int LFX_FILE_EOF = 0x00;

	class EmbreeScene;

	class LFX_ENTRY World
	{
		friend class LFX_Thread;

	public:
		struct Settings
		{
			bool Selected;
			bool RGBEFormat;

			Float3 Ambient;
			Float3 SkyRadiance;

			int MSAA;
#ifdef LFX_FEATURE_EDGE_AA
			int EdgeAA;
#endif
			int Size;			// 128, 256, 512, 1024, 2048
			float Gamma;

			float GIScale;
			int GISamples;
			int GIPathLength;

			int AOLevel;
			float AOStrength;
			float AORadius;
			Float3 AOColor;

			int Threads;

			Settings()
			{
				Selected = false;
				RGBEFormat = false;

				MSAA = 1;
#ifdef LFX_FEATURE_EDGE_AA
				EdgeAA = 0;
#endif
				Size = 512;
				Gamma = 2.2f;

				GIScale = 0.5f;
				GISamples = 25;
				GIPathLength = 4;

				AOLevel = 0;
				AOStrength = 0.5f;
				AORadius = 1.0f;
				AOColor = Float3(0.5f, 0.5f, 0.5f);

				Threads = 1;
			}

			int GetBeastMSAA(int width, int height)
			{
				const int Resolution = 1024;

				int n = 1;
				int w = width, h = height;
				while (1)
				{
					if (w >= Resolution || h >= Resolution)
						break;

					w *= 2;
					h *= 2;
					n *= 2;
				}

				if (w > Resolution || h > Resolution) {
					n /= 2;
				}

				n = std::max(n, 1);
				if (MSAA > 1) {
					n = std::min(n, MSAA);
				}

				return n;
			}
		};

	public:
		enum {
			STAGE_START,

			STAGE_DIRECT_LIGHTING,
			STAGE_INDIRECT_LIGHTING,
			STAGE_AMBIENT_OCCLUSION,
			STAGE_POST_PROCESS,
			STAGE_END,
		};

	protected:
		static World * msInstance;

	public:
		static World * Instance() { return msInstance; }

	public:
		World();
		~World();

		Settings * GetSetting() { return &mSetting; }

		bool Load();
		void Save();
		void Clear();

		// Texture
		Texture * LoadTexture(const String & name);
		Texture * CreateTexture(const String & name, int w, int h, int channels, unsigned char *data);
		Texture * GetTexture(const String & name);

		// Light
		Light * CreateLight();
		void ClearLights();
		Light * GetLight(int i) { return mLights[i]; }
		int GetLightCount() { return mLights.size(); }

		// Mesh
		Mesh * CreateMesh(const String & name);
		Mesh * GetMesh(int i) { return mMeshes[i]; }
		int GetMeshCount() { return mMeshes.size(); }

		// Terrain
		Terrain * CreateTerrain(float * heightfield, const Terrain::Desc & desc);
		Terrain * GetTerrain(int i) { return mTerrains[i]; }
		int GetTerrainCount() { return mTerrains.size(); }

		void Build();
		void Start();
		int UpdateStage();
		int GetStage() { return mStage; }
		int GetProgress() { return mProgress; }
		int GetEntityCount() { return mEntitys.size(); }
		LFX_Baker * GetThread(int i) { return mThreads[i]; }
		void _onThreadCompeleted() { mProgress += 1; }

		bool RayCheck(Contact & contract, const Ray & ray, float len, int flags);
		bool Occluded(const Ray & ray, float len, int flags);
		bool _RayCheckImp(Contact & contract, const Ray & ray, float len, int flags);
		bool _OccludedImp(const Ray & ray, float len, int flags);

	protected:
		Settings mSetting;
		
		std::vector<Texture *> mTextures;
		std::vector<Light *> mLights;
		std::vector<Mesh *> mMeshes;
		std::vector<Terrain *> mTerrains;

		BSPTree<Mesh *> mBSPTree;
		EmbreeScene * mEmbreeScene;

		int mStage;
		std::vector<LFX_Baker::Unit> mEntitys;
		int mIndex;
		int mProgress;
		std::vector<LFX_Baker *> mThreads;
	};
}