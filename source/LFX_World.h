#pragma once

#include "LFX_Log.h"
#include "LFX_Light.h"
#include "LFX_Mesh.h"
#include "LFX_Terrain.h"
#include "LFX_Scene.h"
#include "LFX_Shader.h"
#include "LFX_Baker.h"
#include "LFX_SHBaker.h"
#include "LFX_Rasterizer.h"

namespace LFX {

	class EmbreeScene;

	class LFX_ENTRY World : public Singleton<World>
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
			bool Highp;
			float Gamma;

			float GIScale;
			int GISamples;
			int GIPathLength;

			float GIProbeScale;
			int GIProbeSamples;
			int GIProbePathLength;

			int AOLevel;
			float AOStrength;
			float AORadius;
			Float3 AOColor;

			int Threads;

			bool BakeLightMap;
			bool BakeLightProbe;

			Settings()
			{
				Selected = false;
				RGBEFormat = false;

				MSAA = 1;
#ifdef LFX_FEATURE_EDGE_AA
				EdgeAA = 0;
#endif
				Size = 512;
				Highp = false;
				Gamma = 1.0f;

				GIScale = 1.0f;
				GISamples = 25;
				GIPathLength = 2;

				AOLevel = 0;
				AOStrength = 0.5f;
				AORadius = 1.0f;
				AOColor = Float3(0.5f, 0.5f, 0.5f);

				Threads = 1;
				BakeLightMap = true;
				BakeLightProbe = false;
			}
		};

	public:
		World();
		~World();

		Settings * GetSetting() { return &mSetting; }

		bool Load();
		void Save();
		void Clear();

		Scene* GetScene() { return mScene; }
		Shader* GetShader() { return mShader; }

		Texture* LoadTexture(const String& filename);
		Texture* CreateTexture(const String& name, int w, int h, int channels);
		Texture* GetTexture(const String& name);

		Mesh* CreateMesh();
		Light* CreateLight();
		SHProbe* CreateSHProbe();
		Terrain* CreateTerrain(float* heightfield, const Terrain::Desc& desc);
		Light* GetMainLight() const; // main direction light
		const std::vector<Mesh*>& Meshes() const { return mMeshes; }
		const std::vector<Light*>& Lights() const { return mLights; }
		const std::vector<SHProbe>& SHProbes() const { return mSHProbes; }
		const std::vector<Terrain*>& Terrains() const { return mTerrains; }

		void Build();
		void Start();
		bool End();
		void UpdateTask();
		int GetProgress() { return mProgress; }
		int GetTaskCount() { return mTasks.size(); }
		STBaker* GetThread(int i) { return mThreads[i]; }
		void _onThreadCompeleted() { mProgress += 1; }

	protected:
		bool GetNextTask(STBaker::Task& task);
		STBaker* GetFreeThread();

	protected:
		Settings mSetting;

		Scene* mScene;
		Shader* mShader;
		std::vector<Texture *> mTextures;
		std::vector<Light *> mLights;
		std::vector<Mesh *> mMeshes;
		std::vector<Terrain *> mTerrains;
		std::vector<SHProbe> mSHProbes;

		std::vector<STBaker::Task> mTasks;
		int mTaskIndex;
		int mProgress;
		std::vector<STBaker *> mThreads;
	};
}