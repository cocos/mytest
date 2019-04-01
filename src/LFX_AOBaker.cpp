#include "LFX_AOBaker.h"
#include "LFX_World.h"
#include "LFX_EmbreeScene.h"

namespace LFX {

	AOBaker::AOBaker()
	{
		int index = 0;

		for (int i = 0; i < 16; ++i)
		{
			hd[index].y = 0;
			hd[index].x = cos(i / 16.0f * Pi2);
			hd[index].z = sin(i / 16.0f * Pi2);
			hd[index].w = 1.0f;
			++index;
		}

		for (int i = 0; i < 8; ++i)
		{
			float r = sin(Pi / 6.0f);
			hd[index].y = cos(Pi / 6.0f);
			hd[index].x = cos(i / 8.0f * Pi2) * r;
			hd[index].z = sin(i / 8.0f * Pi2) * r;
			hd[index].w = 1.0f;
			++index;
		}

		for (int i = 0; i < 4; ++i)
		{
			float r = sin(Pi / 3.0f);
			hd[index].y = cos(Pi / 3.0f);
			hd[index].x = cos(i / 4.0f * Pi2) * r;
			hd[index].z = sin(i / 4.0f * Pi2) * r;
			hd[index].w = 1.0f;
			++index;
		}

		hd[index] = Float4(0, 1, 0, 1);
		
		//
		index = 0;

		for (int i = 0; i < 8; ++i)
		{
			ld[index].y = 0;
			ld[index].x = cos(i / 8.0f * Pi2);
			ld[index].z = sin(i / 8.0f * Pi2);
			ld[index].w = 1.0f;
			++index;
		}

		for (int i = 0; i < 4; ++i)
		{
			float r = sin(0.25f * Pi2);
			ld[index].y = cos(0.25f * Pi2);
			ld[index].x = cos(i / 4.0f * Pi2) * r;
			ld[index].z = sin(i / 4.0f * Pi2) * r;
			ld[index].w = 1.0f;
			++index;
		}

		ld[index] = Float4(0, 1, 0, 1);
	}

	AOBaker::~AOBaker()
	{
	}

	Float3 AOBaker::Calcu(const RVertex & v, int flags, void * entity)
	{
		float strength = World::Instance()->GetSetting()->AOStrength;
		float radius = World::Instance()->GetSetting()->AORadius;
		if (radius <= 0)
			return Float3(1, 1, 1);

		Float3 xaxis = Float3(1, 0, 0);
		Float3 zaxis = Float3(0, 0, 1);
		if (v.Normal.dot(xaxis) > 0.99f)
		{
			xaxis = Float3(0, -1, 0);
		}

		zaxis = Float3::Cross(xaxis, v.Normal);
		xaxis = Float3::Cross(v.Normal, zaxis);

		Mat3 form; 
		form.SetXBasis(xaxis);
		form.SetYBasis(v.Normal);
		form.SetZBasis(zaxis);

		float ao = 0;
		float i_radius = 1 / radius;

		if (World::Instance()->GetSetting()->AOLevel == 2) {
			const int hd_size = 29;

			for (int i = 0; i < hd_size; ++i)
			{
				Ray ray;
				ray.orig = v.Position + v.Normal * 0.01f;
				ray.dir = Float3(hd[i].x, hd[i].y, hd[i].z);
				Mat3::Transform(ray.dir, form);

				Contact contact;
				if (World::Instance()->GetEmbreeScene()->RayCheck(contact, ray, radius, flags) && contact.entity != entity)
				{
					float ka = Clamp(contact.vhit.Normal.dot(-v.Normal), -1.0f, 1.0f);
					ka = RadianToDegree(acos(ka));
					ka = 1 - Min(1.0f, ka / 120.0f);

					float kd = contact.td * i_radius;
					kd = 1 - Clamp(kd, 0.0f, 1.0f);

					ao += ka * kd * hd[i].w;
				}
			}

			ao = ao * strength / hd_size;
		}
		else
		{
			const int ld_size = 13;

			for (int i = 0; i < ld_size; ++i)
			{
				Ray ray;
				ray.orig = v.Position + v.Normal * 0.01f;
				ray.dir = Float3(ld[i].x, ld[i].y, ld[i].z);
				Mat3::Transform(ray.dir, form);

				Contact contact;
				if (World::Instance()->GetEmbreeScene()->RayCheck(contact, ray, radius, flags) && contact.entity != entity)
				{
					float ka = Clamp(contact.vhit.Normal.dot(-v.Normal), -1.0f, 1.0f);
					ka = RadianToDegree(acos(ka));
					ka = 1 - Min(1.0f, ka / 120.0f);

					float kd = contact.td * i_radius;
					kd = 1 - Clamp(kd, 0.0f, 1.0f);

					ao += ka * kd * ld[i].w;
				}
			}

			ao = ao * strength / ld_size;
		}

		ao = Clamp(ao, 0.0f, 1.0f);

		return Float3::Lerp(World::Instance()->GetSetting()->AOColor, Float3(1, 1, 1), 1 - ao);
	}

}
