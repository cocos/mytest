#pragma once

#include "LFX_Thread.h"
#include "LFX_Entity.h"

namespace LFX {

	class LFX_ENTRY STBaker : public Thread
	{
	public:
		struct Task
		{
			Entity* entity;
			int index;
		};

	public:
		STBaker(int id);
		virtual ~STBaker();

		virtual void Run();

		bool IsCompeleted() { return mCompeleted; }
		void Enqueue(Entity* entity, int index);

	protected:
		void _calcuDirectLightingMesh();
		void _calcuDirectLightingTerrain();
		void _calcuIndirectLightingMesh();
		void _calcuIndirectLightingTerrain();
		void _calcuAmbientOcclusionMesh();
		void _calcuAmbientOcclusionTerrain();
		void _calcuSHProbe();
		void _postProcess();

	protected:
		int mId;
		Entity* mEntity;
		int mIndex;
		std::atomic_bool mCompeleted;
	};
}