#pragma once

#include "LFX_Log.h"
#include "LFX_Light.h"
#include "LFX_Mesh.h"
#include "LFX_Terrain.h"
#include "LFX_Scene.h"
#include "LFX_Shader.h"
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

			bool Filter;
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
				AOStrength = 1.0f;
				AORadius = 1.0f;
				AOColor = Float3(0.0f, 0.0f, 0.0f);

				Threads = 1;
				Filter = false;
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

		Shader* GetShader() { return mShader; }

		Texture* LoadTexture(const String& filename);
		Texture* CreateTexture(const String& name, int w, int h, int channels);
		Texture* GetTexture(const String& name);

		Mesh* CreateMesh();
		Light* CreateLight();
		SHProbe* CreateSHProbe();
		Terrain* CreateTerrain(float* heightfield, const Terrain::Desc& desc);
		Light* GetMainLight() const; // main direction light
		const std::vector<Mesh*>& GetMeshes() const { return mMeshes; }
		const std::vector<Light*>& GetLights() const { return mLights; }
		const std::vector<SHProbe>& GetSHProbes() const { return mSHProbes; }
		const std::vector<Terrain*>& GetTerrains() const { return mTerrains; }

		void BuildScene();
		Scene* GetScene() { return mScene; }

	protected:
		Settings mSetting;

		Shader* mShader;
		std::vector<Texture *> mTextures;
		std::vector<Light *> mLights;
		std::vector<Mesh *> mMeshes;
		std::vector<Terrain *> mTerrains;
		std::vector<SHProbe> mSHProbes;
		Scene* mScene;
	};
}