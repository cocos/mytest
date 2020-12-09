#pragma once

#include "LFX_Log.h"
#include "LFX_Light.h"
#include "LFX_Mesh.h"
#include "LFX_Terrain.h"
#include "LFX_Baker.h"
#include "LFX_Rasterizer.h"

namespace LFX {

	static const int LFX_FILE_VERSION = 0x2000;
	static const int LFX_FILE_TERRAIN = 0x01;
	static const int LFX_FILE_MESH = 0x02;
	static const int LFX_FILE_LIGHT = 0x03;
	static const int LFX_FILE_EOF = 0x00;

	class EmbreeScene;

	class LFX_ENTRY World : public Singleton<World>
	{
		friend class LFX_Thread;

	public:
		struct Settings
		{
			bool Selected;
			bool RGBEFormat;
			bool Tonemapping;

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
				Tonemapping = false;

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
		};

	public:
		World();
		~World();

		Settings * GetSetting() { return &mSetting; }

		bool Load();
		void Save();
		void Clear();

		// Texture
		Texture * LoadTexture(const String & filename);
		Texture * CreateTexture(const String & name, int w, int h, int channels, unsigned char *data);
		Texture * GetTexture(const String & name);

		// Light
		Light * CreateLight();
		Light * GetLight(int i) { return mLights[i]; }
		int GetLightCount() { return mLights.size(); }

		// Mesh
		Mesh * CreateMesh();
		Mesh * GetMesh(int i) { return mMeshes[i]; }
		int GetMeshCount() { return mMeshes.size(); }

		// Terrain
		Terrain * CreateTerrain(const Float3& pos, float * heightfield, const Terrain::Desc & desc);
		Terrain * GetTerrain(int i) { return mTerrains[i]; }
		int GetTerrainCount() { return mTerrains.size(); }

		void Build();
		void Start();
		bool End();
		void UpdateTask();
		int GetProgress() { return mProgress; }
		int GetTaskCount() { return mTasks.size(); }
		STBaker* GetThread(int i) { return mThreads[i]; }
		void _onThreadCompeleted() { mProgress += 1; }

		bool RayCheck(Contact & contract, const Ray & ray, float len, int flags);
		bool Occluded(const Ray & ray, float len, int flags);
		bool _RayCheckImp(Contact & contract, const Ray & ray, float len, int flags);
		bool _OccludedImp(const Ray & ray, float len, int flags);

	protected:
		bool GetNextTask(STBaker::Task& task);
		STBaker* GetFreeThread();

	protected:
		Settings mSetting;

		std::vector<Texture *> mTextures;
		std::vector<Light *> mLights;
		std::vector<Mesh *> mMeshes;
		std::vector<Terrain *> mTerrains;

		BSPTree<Mesh *> mBSPTree;
		EmbreeScene * mEmbreeScene;

		std::vector<STBaker::Task> mTasks;
		int mTaskIndex;
		int mProgress;
		std::vector<STBaker *> mThreads;
	};
}