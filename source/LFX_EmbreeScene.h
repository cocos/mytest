#pragma once

#include "LFX_Scene.h"

#ifdef LFX_USE_EMBREE_SCENE

#include "embree2/rtcore.h"
#include "embree2/rtcore_ray.h"

namespace LFX {

	struct LFX_ENTRY EmbreeRay : public RTCRay
	{
		EmbreeRay(const Float3& origin, const Float3& direction, float len = FLT_MAX, int flags = 0xFFFFFFFF)
		{
			org[0] = origin.x;
			org[1] = origin.y;
			org[2] = origin.z;
			dir[0] = direction.x;
			dir[1] = direction.y;
			dir[2] = direction.z;
			tnear = 0;
			tfar = len;
			geomID = RTC_INVALID_GEOMETRY_ID;
			primID = RTC_INVALID_GEOMETRY_ID;
			instID = RTC_INVALID_GEOMETRY_ID;
			mask = flags;
			time = 0.0f;
		}

		bool Hit() const
		{
			return geomID != RTC_INVALID_GEOMETRY_ID;
		}

		Float3 Origin() const
		{
			return Float3(org[0], org[1], org[2]);
		}

		Float3 Direction() const
		{
			return Float3(dir[0], dir[1], dir[2]);
		}
	};

#define StaticAssert_(x) static_assert(x, #x);
	StaticAssert_(sizeof(EmbreeRay) == sizeof(RTCRay));

	class EmbreeScene : public Scene
	{
	public:
		EmbreeScene();
		~EmbreeScene();

		void Build();

		bool RayCheck(Contact& contact, const Ray& ray, float len, int mask) override;
		bool Occluded(const Ray& ray, float len, int mask) override;

	protected:
		void TriangleLerp(Vertex& vout, Entity* pEntity, int triIndex, float u, float v);
		Material* GetMaterial(Entity* pEntity, int triIndex);

	protected:
		RTCDevice rtcDevice;
		RTCScene rtcScene;
		std::vector<Entity*> mEntityMap;
	};

}

#endif