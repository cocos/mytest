#pragma once

#include "LFX_Types.h"
#include "LFX_Mesh.h"

namespace LFX {

	class Scene
	{
	public:
		Scene();
		virtual ~Scene();

		virtual void Build();

		virtual bool RayCheck(Contact& contact, const Ray& ray, float len, int mask);
		virtual bool Occluded(const Ray& ray, float len, int mask);

		bool _RayCheckImp(Contact& contact, const Ray& ray, float len, int flags);
		bool _OccludedImp(const Ray& ray, float len, int flags);

	protected:
		BSPTree<Mesh*> mBVTree;
	};

}