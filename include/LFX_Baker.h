#pragma once

#include "LFX_Thread.h"
#include "LFX_Entity.h"

namespace LFX {

	class LFX_ENTRY LFX_Baker : public Thread
	{
	public:
		struct Unit
		{
			Entity * entity;
			int index;
		};

	public:
		LFX_Baker(int id);
		virtual ~LFX_Baker();

		virtual void Run();

		bool IsCompeleted() { return mCompeleted; }
		void Enqueue(Entity * entity, int index, int stage);

	protected:
		void _calcuDirectLightingMesh();
		void _calcuDirectLightingTerrain();
		void _calcuIndirectLightingMesh();
		void _calcuIndirectLightingTerrain();
		void _calcuAmbientOcclusionMesh();
		void _calcuAmbientOcclusionTerrain();
		void _postProcess();

	protected:
		int mId;
		Entity* mEntity;
		int mIndex;
		int mStage;
		std::atomic_bool mCompeleted;
	};
}