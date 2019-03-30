#include "LFX_World.h"
#include "LFX_EmbreeScene.h"

namespace LFX {

	EmbreeScene::EmbreeScene()
	{
		rtcDevice = rtcNewDevice();
		RTCError embreeError = rtcDeviceGetError(rtcDevice);
		if (embreeError == RTC_UNSUPPORTED_CPU)
		{
			rtcDevice = NULL;
		}
		else if (embreeError != RTC_NO_ERROR)
		{
		}

		rtcDevice = NULL;

		rtcScene = NULL;
	}

	EmbreeScene::~EmbreeScene()
	{
		if (rtcScene != NULL)
			rtcDeleteScene(rtcScene);

		if (rtcDevice != NULL)
			rtcDeleteDevice(rtcDevice);
	}

	bool EmbreeScene::Build()
	{
		if (rtcDevice == NULL)
			return false;

		rtcScene = rtcDeviceNewScene(rtcDevice, RTC_SCENE_DYNAMIC, RTC_INTERSECT1);

		for (int i = 0; i < World::Instance()->GetMeshCount(); ++i)
		{
			Mesh * mesh = World::Instance()->GetMesh(i);
			int totalNumTriangles = mesh->NumOfTriangles();
			int totalNumVertices = mesh->NumOfVertices();

			unsigned int geoID = rtcNewTriangleMesh(rtcScene, RTC_GEOMETRY_STATIC, totalNumTriangles, totalNumVertices);

			rtcSetMask(rtcScene, geoID, LFX_MESH);

			Float4* meshVerts = reinterpret_cast<Float4*>(rtcMapBuffer(rtcScene, geoID, RTC_VERTEX_BUFFER));
			{
				Vertex * pVertex = NULL;
				Triangle * pTriangle = NULL;
				Material * pMaterial = NULL;

				mesh->Lock(&pVertex, &pTriangle, &pMaterial);
				for (int j = 0; j < mesh->NumOfVertices(); ++j)
				{
					*meshVerts++ = Float4(pVertex[j].Position.x, pVertex[j].Position.y, pVertex[j].Position.z, 0);
				}
				mesh->Unlock();
			}
			rtcUnmapBuffer(rtcScene, geoID, RTC_VERTEX_BUFFER);

			unsigned int * meshTriangles = reinterpret_cast<unsigned int*>(rtcMapBuffer(rtcScene, geoID, RTC_INDEX_BUFFER));
			{
				Vertex * pVertex = NULL;
				Triangle * pTriangle = NULL;
				Material * pMaterial = NULL;

				mesh->Lock(&pVertex, &pTriangle, &pMaterial);
				for (int j = 0; j < mesh->NumOfTriangles(); ++j)
				{
					*meshTriangles++ = pTriangle[j].Index0;
					*meshTriangles++ = pTriangle[j].Index1;
					*meshTriangles++ = pTriangle[j].Index2;
				}
				mesh->Unlock();
			}
			rtcUnmapBuffer(rtcScene, geoID, RTC_INDEX_BUFFER);

			mEntityMap.push_back(mesh);
		}

#define _ES_TERRAIN_ENABLED 1
#if _ES_TERRAIN_ENABLED
		{
			Terrain *pTerrain = World::Instance()->GetTerrain();
			if (pTerrain != NULL) {
				std::vector<Vertex> & pVertex = pTerrain->_getVertexBuffer();
				std::vector<Triangle> & pTriangle = pTerrain->_getTriBuffer();
				int totalNumTriangles = pTerrain->_getTriBuffer().size();
				int totalNumVertices = pTerrain->_getVertexBuffer().size();

				unsigned int geoID = rtcNewTriangleMesh(rtcScene, RTC_GEOMETRY_STATIC, totalNumTriangles, totalNumVertices);

				rtcSetMask(rtcScene, geoID, LFX_TERRAIN);

				Float4* meshVerts = reinterpret_cast<Float4*>(rtcMapBuffer(rtcScene, geoID, RTC_VERTEX_BUFFER));
				for (int j = 0; j < World::Instance()->GetTerrain()->_getVertexBuffer().size(); ++j)
				{
					*meshVerts++ = Float4(pVertex[j].Position.x, pVertex[j].Position.y, pVertex[j].Position.z, 0);
				}
				rtcUnmapBuffer(rtcScene, geoID, RTC_VERTEX_BUFFER);

				unsigned int * meshTriangles = reinterpret_cast<unsigned int*>(rtcMapBuffer(rtcScene, geoID, RTC_INDEX_BUFFER));
				for (int j = 0; j < World::Instance()->GetTerrain()->_getTriBuffer().size(); ++j)
				{
					*meshTriangles++ = pTriangle[j].Index0;
					*meshTriangles++ = pTriangle[j].Index1;
					*meshTriangles++ = pTriangle[j].Index2;
				}
				rtcUnmapBuffer(rtcScene, geoID, RTC_INDEX_BUFFER);

				mEntityMap.push_back(World::Instance()->GetTerrain());
			}
		}
#endif

#if 0
		// test
		unsigned int geoID = rtcNewTriangleMesh(rtcScene, RTC_GEOMETRY_STATIC, 2, 4);
		Float4* meshVerts = reinterpret_cast<Float4*>(rtcMapBuffer(rtcScene, geoID, RTC_VERTEX_BUFFER));
		{
			meshVerts[0] = Float4(0, 0, 0, 0);
			meshVerts[1] = Float4(1000, 0, 0, 0);
			meshVerts[2] = Float4(0, 0, 1000, 0);
			meshVerts[3] = Float4(1000, 0, 1000, 0);
		}
		rtcUnmapBuffer(rtcScene, geoID, RTC_VERTEX_BUFFER);

		unsigned int * meshTriangles = reinterpret_cast<unsigned int*>(rtcMapBuffer(rtcScene, geoID, RTC_INDEX_BUFFER));
		{
			*meshTriangles++ = 0;
			*meshTriangles++ = 1;
			*meshTriangles++ = 2;
			*meshTriangles++ = 2;
			*meshTriangles++ = 1;
			*meshTriangles++ = 3;
		}
		rtcUnmapBuffer(rtcScene, geoID, RTC_INDEX_BUFFER);
#endif

		rtcCommit(rtcScene);

		RTCError embreeError = rtcDeviceGetError(rtcDevice);
		assert(embreeError == RTC_NO_ERROR);
		if (embreeError != RTC_NO_ERROR)
		{
			return false;
		}

		return true;
	}


	bool EmbreeScene::RayCheck(Contact & contact, const Ray & ray, float len, int mask)
	{
		contact.td = FLT_MAX;
		contact.tu = 0;
		contact.tv = 0;
		contact.triIndex = -1;
		contact.entity = NULL;
		contact.backFacing = false;

		if (rtcDevice != NULL)
		{
			Float3 orig = ray.orig;

			int count = 0;
			while (count++ < 4)
			{
				EmbreeRay r(orig, ray.dir, len, mask);
				rtcIntersect(rtcScene, r);

				if (!r.Hit())
					break;

				void * entity = mEntityMap[r.geomID];
				if (entity != NULL && entity != World::Instance()->GetTerrain())
				{
					Mesh * mesh = (Mesh *)entity;
					Material * m = GetMaterial(entity, r.primID);
					if (m->texture != NULL)
					{
						const Triangle & triangle = mesh->_getTriangle(r.primID);
						const Float2 & uv0 = mesh->_getVertex(triangle.Index0).UV;
						const Float2 & uv1 = mesh->_getVertex(triangle.Index1).UV;
						const Float2 & uv2 = mesh->_getVertex(triangle.Index2).UV;

						Float2 uv = uv0 * (1 - r.u - r.v) + uv1 * r.u + uv2 * r.v;

						Float4 color = m->texture->SampleColor(uv.x, uv.y);
						if (color.w < 0.5f)
						{
							orig += (r.tfar + 0.01f) * ray.dir;
							len -= r.tfar;
							continue;
						}
					}
				}

				contact.td = r.tfar;
				contact.tu = r.u;
				contact.tv = r.v;
				contact.triIndex = r.primID;
				contact.entity = mEntityMap[r.geomID];
				contact.mtl = GetMaterial(contact.entity, r.primID);
				TriangleLerp(contact.vhit, contact.entity, r.primID, r.u, r.v);

				if (contact.entity != NULL)
				{
					Vertex a, b, c;
					if (contact.entity == World::Instance()->GetTerrain())
					{
						Triangle tri = World::Instance()->GetTerrain()->_getTriangle(contact.triIndex);
						a = World::Instance()->GetTerrain()->_getVertex(tri.Index0);
						b = World::Instance()->GetTerrain()->_getVertex(tri.Index1);
						c = World::Instance()->GetTerrain()->_getVertex(tri.Index2);
					}
					else
					{
						Mesh * mesh = (Mesh *)contact.entity;
						Triangle tri = mesh->_getTriangle(contact.triIndex);
						a = mesh->_getVertex(tri.Index0);
						b = mesh->_getVertex(tri.Index1);
						c = mesh->_getVertex(tri.Index2);
					}

					Float3 triNml = Float3::Normalize(Float3::Cross(c.Position - a.Position, b.Position - a.Position));
					contact.backFacing = Float3::Dot(triNml, ray.dir) >= 0.0f;
				}

				return true;
			}

			return false;
		}
		else
		{
			return World::Instance()->RayCheck(contact, ray, len, LFX_MESH | LFX_TERRAIN);
		}
		
		return false;
	}

	bool EmbreeScene::Occluded(const Float3& position, const Float3& direction, float len, int mask)
	{
		if (rtcDevice != NULL)
		{
			Float3 orig = position;

			int count = 0;
			while (count++ < 4)
			{
				EmbreeRay r(orig, direction, len, mask);
				rtcIntersect(rtcScene, r);

				if (!r.Hit())
					break;

				void * entity = mEntityMap[r.geomID];
				if (entity != NULL && entity != World::Instance()->GetTerrain())
				{
					Mesh * mesh = (Mesh *)entity;
					Material * m = GetMaterial(entity, r.primID);
					if (m->texture != NULL)
					{
						const Triangle & triangle = mesh->_getTriangle(r.primID);
						const Float2 & uv0 = mesh->_getVertex(triangle.Index0).UV;
						const Float2 & uv1 = mesh->_getVertex(triangle.Index1).UV;
						const Float2 & uv2 = mesh->_getVertex(triangle.Index2).UV;

						Float2 uv = uv0 * (1 - r.u - r.v) + uv1 * r.u + uv2 * r.v;

						Float4 color = m->texture->SampleColor(uv.x, uv.y);
						if (color.w < 0.5f)
						{
							orig += (r.tfar + 0.01f) * direction;
							len -= r.tfar;
							continue;
						}
					}
				}

				return true;
			}

			return false;
		}
		else
		{
			Ray ray;
			ray.orig = position;
			ray.dir = direction;
			return World::Instance()->Occluded(ray, len, LFX_MESH | LFX_TERRAIN);
		}
	}

	void EmbreeScene::TriangleLerp(Vertex & vout, void * pEntity, int triIndex, float u, float v)
	{
		Vertex * pVertexBuffer;
		Triangle * pTrangleBuffer;

		if (pEntity == World::Instance()->GetTerrain())
		{
			std::vector<Vertex> & pVertex = World::Instance()->GetTerrain()->_getVertexBuffer();
			std::vector<Triangle> & pTriangle = World::Instance()->GetTerrain()->_getTriBuffer();

			pVertexBuffer = &pVertex[0];
			pTrangleBuffer = &pTriangle[0];
		}
		else
		{
			Mesh * mesh = (Mesh *)pEntity;
			Vertex * pVertex = NULL;
			Triangle * pTriangle = NULL;
			Material * pMaterial = NULL;

			mesh->Lock(&pVertex, &pTriangle, &pMaterial);
			mesh->Unlock();

			pVertexBuffer = pVertex;
			pTrangleBuffer = pTriangle;
		}

		int ia = pTrangleBuffer[triIndex].Index0;
		int ib = pTrangleBuffer[triIndex].Index1;
		int ic = pTrangleBuffer[triIndex].Index2;

		const Vertex & va = pVertexBuffer[ia];
		const Vertex & vb = pVertexBuffer[ib];
		const Vertex & vc = pVertexBuffer[ic];
		
		vout = va + (vb - va) * u + (vc - va) * v;
		vout.Normal.normalize();
		vout.Tangent.normalize();
		vout.Binormal.normalize();
	}

	Material * EmbreeScene::GetMaterial(void * pEntity, int triIndex)
	{
		if (pEntity == World::Instance()->GetTerrain())
		{
			return World::Instance()->GetTerrain()->GetMaterial();
		}
		else
		{
			Mesh * mesh = (Mesh *)pEntity;
			Vertex * pVertex = NULL;
			Triangle * pTriangle = NULL;
			Material * pMaterial = NULL;
			int mtlId;

			mesh->Lock(&pVertex, &pTriangle, &pMaterial);
			mtlId = pTriangle[triIndex].MaterialId;
			mesh->Unlock();

			return &pMaterial[mtlId];
		}
	}
}