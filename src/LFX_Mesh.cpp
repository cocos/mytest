#include "LFX_Mesh.h"
#include "LFX_World.h"
#include "LFX_AOBaker.h"
#include "LFX_ILBakerRaytrace.h"
#include "LFX_EmbreeScene.h"

namespace LFX {

	Mesh::Mesh(const String & name)
		: mName(name)
	{
		mUserData = NULL;
		mCastShadow = true;
		mReceiveShadow = true;
		mLightingMapSize = 0;
	}

	Mesh::~Mesh()
	{
		mVertexBuffer.clear();
		mTriBuffer.clear();
		mMtlBuffer.clear();

		mBSPTree.Clear();
	}

	void Mesh::Alloc(int numVertex, int numTriangle, int numMaterial)
	{
		if (numVertex >= 0) {
			mVertexBuffer.resize(numVertex);
		}
		if (numTriangle >= 0) {
			mTriBuffer.resize(numTriangle);
		}
		if (numMaterial > 0) {
			mMtlBuffer.resize(numMaterial);
		}
	}

	void Mesh::Lock(Vertex ** ppVertex, Triangle ** ppTriangle, Material ** ppMaterial)
	{
		*ppVertex = &mVertexBuffer[0];
		*ppTriangle = &mTriBuffer[0];
		*ppMaterial = &mMtlBuffer[0];
	}

	void Mesh::Unlock()
	{
	}

	void Mesh::Build()
	{
		if (mVertexBuffer.size() == 0)
			return;

		Aabb bound;
		bound.minimum = mVertexBuffer[0].Position;
		bound.maximum = mVertexBuffer[0].Position;

		mUVMin = Float2(0, 0);
		mUVMax = Float2(1, 1);
		for (int i = 1; i < mVertexBuffer.size(); ++i)
		{
			bound.minimum = Minimum(bound.minimum, mVertexBuffer[i].Position);
			bound.maximum = Maximum(bound.maximum, mVertexBuffer[i].Position);

			mUVMin = Minimum(mVertexBuffer[i].LUV, mUVMin);
			mUVMax = Maximum(mVertexBuffer[i].LUV, mUVMax);
		}

		mBSPTree.Build(bound, 8);

		for (int i = 0; i < mTriBuffer.size(); ++i)
		{
			bool hr = _addTri(mBSPTree.RootNode(), i);
			assert(hr);
		}

		_calcuTangent();
		_optimize(mBSPTree.RootNode());

		if (mLightingMapSize > 0)
		{
			mLightingMap.resize(mLightingMapSize * mLightingMapSize);
			for (int i = 0; i < mLightingMapSize * mLightingMapSize; ++i)
			{
				mLightingMap[i] = Float3(0, 0, 0);
			}
		}
	}

	bool Mesh::Valid()
	{
		return mBSPTree.RootNode() != NULL;
	}

	const Aabb & Mesh::GetBound()
	{
		assert(Valid());

		return mBSPTree.RootNode()->aabb;
	}

	void Mesh::_rayCheckImp(Contact & contract, BSPTree<int>::Node * node, const Ray & ray, float length)
	{
		float dist = 0;

		if (!Intersect(ray, &dist, node->aabb) || dist >= contract.td || dist >= length)
			return;

		for (int i = 0; i < node->elems.size(); ++i)
		{
			int triIndex = node->elems[i];
			const Triangle & triangle = mTriBuffer[triIndex];

			const Float3 & a = mVertexBuffer[triangle.Index0].Position;
			const Float3 & b = mVertexBuffer[triangle.Index1].Position;
			const Float3 & c = mVertexBuffer[triangle.Index2].Position;

			const Material & m = mMtlBuffer[triangle.MaterialId];

			float tu, tv;
			if (Intersect(ray, &dist, tu, tv, a, b, c) && dist < contract.td && dist <= length)
			{
				if (m.texture != NULL)
				{
					const Float2 & uv0 = mVertexBuffer[triangle.Index0].UV;
					const Float2 & uv1 = mVertexBuffer[triangle.Index1].UV;
					const Float2 & uv2 = mVertexBuffer[triangle.Index2].UV;

					Float2 uv = uv0 * (1 - tu - tv) + uv1 * tu + uv2 * tv;

					Float4 color = m.texture->SampleColor(uv.x, uv.y);
					if (color.w < 0.5f)
						continue;
				}

				contract.td = dist;
				contract.triIndex = triIndex;
				contract.tu = tu;
				contract.tv = tv;
				contract.entity = this;
				contract.mtl = &m;

				const Vertex & v0 = mVertexBuffer[triangle.Index0];
				const Vertex & v1 = mVertexBuffer[triangle.Index1];
				const Vertex & v2 = mVertexBuffer[triangle.Index2];
				Vertex::Lerp(contract.vhit, v0, v1, v2, tu, tv);
			}
		}

		if (node->child[0] != NULL)
			_rayCheckImp(contract, node->child[0], ray, length);

		if (node->child[1] != NULL)
			_rayCheckImp(contract, node->child[1], ray, length);
	}

	bool Mesh::_occludedImp(BSPTree<int>::Node * node, const Ray & ray, float length)
	{
		float dist = 0;
		if (!Intersect(ray, &dist, node->aabb) || dist >= length)
			return false;

		for (int i = 0; i < node->elems.size(); ++i)
		{
			int triIndex = node->elems[i];
			const Triangle & triangle = mTriBuffer[triIndex];

			const Float3 & a = mVertexBuffer[triangle.Index0].Position;
			const Float3 & b = mVertexBuffer[triangle.Index1].Position;
			const Float3 & c = mVertexBuffer[triangle.Index2].Position;

			const Material & m = mMtlBuffer[triangle.MaterialId];

			float tu, tv;
			if (Intersect(ray, &dist, tu, tv, a, b, c) && dist <= length)
			{
				if (m.texture != NULL)
				{
					const Float2 & uv0 = mVertexBuffer[triangle.Index0].UV;
					const Float2 & uv1 = mVertexBuffer[triangle.Index1].UV;
					const Float2 & uv2 = mVertexBuffer[triangle.Index2].UV;

					Float2 uv = uv0 * (1 - tu - tv) + uv1 * tu + uv2 * tv;

					Float4 color = m.texture->SampleColor(uv.x, uv.y);
					if (color.w < 0.5f)
						continue;

					return true;
				}

				return true;
			}
		}

		if (node->child[0] != NULL && _occludedImp(node->child[0], ray, length))
			return true;

		if (node->child[1] != NULL && _occludedImp(node->child[1], ray, length))
			return true;

		return false;
	}

	const Vertex & Mesh::_getVertex(int i)
	{
		return mVertexBuffer[i];
	}

	const Triangle & Mesh::_getTriangle(int i)
	{
		return mTriBuffer[i];
	}

	void Mesh::RayCheck(Contact & contract, const Ray & ray, float length)
	{
		assert(Valid());

		if (mCastShadow)
		{
			_rayCheckImp(contract, mBSPTree.RootNode(), ray, length);
		}
	}

	bool Mesh::Occluded(const Ray & ray, float length)
	{
		assert(Valid());

		if (mCastShadow)
		{
			return _occludedImp(mBSPTree.RootNode(), ray, length);
		}

		return false;
	}

	bool Mesh::_addTri(BSPTree<int>::Node * node, int t)
	{
		const Triangle & triangle = mTriBuffer[t];

		const Float3 & a = mVertexBuffer[triangle.Index0].Position;
		const Float3 & b = mVertexBuffer[triangle.Index1].Position;
		const Float3 & c = mVertexBuffer[triangle.Index2].Position;

		if (node->aabb.Contain(a) && node->aabb.Contain(b) && node->aabb.Contain(c))
		{
			if (node->child[0] != NULL && _addTri(node->child[0], t))
				return true;

			if (node->child[1] != NULL && _addTri(node->child[1], t))
				return true;

			node->elems.push_back(t);

			return true;
		}

		return false;
	}

	void Mesh::_calcuTangent()
	{
		struct FaceSt
		{
			int idx0, idx1, idx2;
			Float3 v0, v1, v2;
			Float2 uv0, uv1, uv2;
		};

		struct TSVertSt
		{
			Float3 norm;
			Float3 tan1;
			Float3 tan2;
		};

		int faceCount = mTriBuffer.size();
		int vertCount = mVertexBuffer.size();

		FaceSt* faceSts = new FaceSt[faceCount];
		TSVertSt* vertSts = new TSVertSt[vertCount];

		for (int f = 0; f < faceCount; ++f)
		{
			FaceSt& faceSt = faceSts[f];
			faceSt.idx0 = mTriBuffer[f].Index0;
			faceSt.idx1 = mTriBuffer[f].Index1;
			faceSt.idx2 = mTriBuffer[f].Index2;

			TSVertSt& vertSt0 = vertSts[faceSt.idx0];
			TSVertSt& vertSt1 = vertSts[faceSt.idx1];
			TSVertSt& vertSt2 = vertSts[faceSt.idx2];

			faceSt.v0 = mVertexBuffer[faceSt.idx0].Position;
			faceSt.v1 = mVertexBuffer[faceSt.idx1].Position;
			faceSt.v2 = mVertexBuffer[faceSt.idx2].Position;

			faceSt.uv0 = mVertexBuffer[faceSt.idx0].LUV;
			faceSt.uv1 = mVertexBuffer[faceSt.idx1].LUV;
			faceSt.uv2 = mVertexBuffer[faceSt.idx2].LUV;

			vertSt0.norm = mVertexBuffer[faceSt.idx0].Normal;
			vertSt1.norm = mVertexBuffer[faceSt.idx1].Normal;
			vertSt2.norm = mVertexBuffer[faceSt.idx2].Normal;

			// calculate tangent vectors ---------------------------

			Float3 v_1_0 = faceSt.v1 - faceSt.v0;
			Float3 v_2_0 = faceSt.v2 - faceSt.v0;
			float s1 = faceSt.uv1.x - faceSt.uv0.x;
			float s2 = faceSt.uv2.x - faceSt.uv0.x;
			float t1 = faceSt.uv1.y - faceSt.uv0.y;
			float t2 = faceSt.uv2.y - faceSt.uv0.y;

			float div = (s1 * t2 - s2 * t1);

			if (div != 0.0)
			{
				//	2D triangle area = (u1*v2-u2*v1)/2
				// weight the tangent vectors by the UV triangles area size (fix problems with base UV assignment)
				float a = t2 / div;
				float b = -t1 / div;
				float c = -s2 / div;
				float d = s1 / div;

				vertSt0.tan1 += v_1_0 * a + v_2_0 * b;
				vertSt1.tan1 += vertSt0.tan1;
				vertSt2.tan1 += vertSt0.tan1;
				vertSt0.tan2 += v_1_0 * c + v_2_0 * d;
				vertSt1.tan2 += vertSt0.tan2;
				vertSt2.tan2 += vertSt0.tan2;
			}
			else
			{
				vertSt0.tan1 += Float3(1, 0, 0);
				vertSt1.tan1 += vertSt0.tan1;
				vertSt2.tan1 += vertSt0.tan1;
				vertSt0.tan2 += Float3(0, 1, 0);
				vertSt1.tan2 += vertSt0.tan2;
				vertSt2.tan2 += vertSt0.tan2;
			}
		}

		for (int v = 0; v < vertCount; ++v)
		{
			TSVertSt& vertSt = vertSts[v];

			// Gram-Schmidt orthogonalization
			Float3 t = (vertSt.tan1 - vertSt.norm * vertSt.norm.dot(vertSt.tan1));
			Float3 b = (vertSt.tan2 - vertSt.norm * vertSt.norm.dot(vertSt.tan2));

			t.normalize();
			b.normalize();

			mVertexBuffer[v].Tangent = t;
			mVertexBuffer[v].Binormal = b;
		}

		delete[] vertSts;
		delete[] faceSts;
	}

	Aabb Mesh::_optimize(BSPTree<int>::Node * node)
	{
		if (node->child[0] == NULL)
		{
			if (node->elems.size() > 0)
			{
				Aabb bound;
				bound.Invalid();

				for (int i = 0; i < node->elems.size(); ++i)
				{
					const Triangle & triangle = mTriBuffer[node->elems[i]];
					const Float3 & a = mVertexBuffer[triangle.Index0].Position;
					const Float3 & b = mVertexBuffer[triangle.Index1].Position;
					const Float3 & c = mVertexBuffer[triangle.Index2].Position;

					bound.Merge(a);
					bound.Merge(b);
					bound.Merge(c);
				}

				node->aabb = bound;
			}
		}
		else
		{
			Aabb bound = _optimize(node->child[0]);
			bound.Merge(_optimize(node->child[1]));

			node->aabb = bound;
		}

		return node->aabb;
	}

	void Mesh::CalcuDirectLighting(const std::vector<Light *> & lights)
	{
		assert(mLightingMapSize > 0);

		int msaa = World::Instance()->GetSetting()->GetBeastMSAA(mLightingMapSize, mLightingMapSize);

		int width = msaa * mLightingMapSize;
		int height = msaa * mLightingMapSize;

		RasterizerSoft * rasterizer = new RasterizerSoft(this, width, height);

		rasterizer->DoRasterize2();

		rasterizer->DoLighting(lights);

		for (int j = 0; j < mLightingMapSize; ++j)
		{
			for (int i = 0; i < mLightingMapSize; ++i)
			{
				Float3 color = Float3(0, 0, 0);

				for (int n = 0; n < msaa; ++n)
				{
					for (int m = 0; m < msaa; ++m)
					{
						int x = i * msaa + m;
						int y = j * msaa + n;

						color += rasterizer->_rmap[y * width + x];
					}
				}

				color /= (float)msaa * msaa;

				mLightingMap[j * mLightingMapSize + i] = color + World::Instance()->GetSetting()->Ambient;
			}
		}
		
		delete rasterizer;
	}

	void Mesh::CalcuIndirectLighting(const std::vector<Light *> & lights)
	{
#ifdef LFX_FEATURE_GI_MSAA
		int msaa = World::Instance()->GetSetting()->RT_MSAA;
#else
		int msaa = 1;
#endif
		int lm_width = mLightingMapSize;
		int lm_height = mLightingMapSize;
		int width = msaa * mLightingMapSize;
		int height = msaa * mLightingMapSize;

		RasterizerSoft * rasterizer = new RasterizerSoft(this, width, height);
		rasterizer->DoRasterize2();

		ILBakerRaytrace baker;
		baker._cfg.SkyRadiance = World::Instance()->GetSetting()->SkyRadiance;
		baker._cfg.DiffuseScale = World::Instance()->GetSetting()->GIScale;
		baker._cfg.SqrtNumSamples = World::Instance()->GetSetting()->GISamples;
		baker._cfg.MaxPathLength = World::Instance()->GetSetting()->GIPathLength;
		baker._cfg.RussianRouletteDepth = -1;
		baker.Run(rasterizer);

		int index = 0;
		std::vector<LFX::Float3> & ilm = this->_getLightingMap();
		for (int j = 0; j < lm_height; ++j)
		{
			for (int i = 0; i < lm_width; ++i)
			{
				Float3 color = Float3(0, 0, 0);
				for (int y = 0; y < msaa; ++y)
				{
					for (int x = 0; x < msaa; ++x)
					{
						int u = i * msaa + x;
						int v = j * msaa + y;

						color.x += baker._ctx.BakeOutput[v * rasterizer->_width + u].x;
						color.y += baker._ctx.BakeOutput[v * rasterizer->_width + u].y;
						color.z += baker._ctx.BakeOutput[v * rasterizer->_width + u].z;
					}
				}

				color /= (float)msaa * msaa;

				ilm[index++] += color;
			}
		}

		delete rasterizer;
	}

	void Mesh::CalcuAmbientOcclusion()
	{
		AOBaker baker;

		int msaa = World::Instance()->GetSetting()->GetBeastMSAA(mLightingMapSize, mLightingMapSize);
		int width = msaa * mLightingMapSize;
		int height = msaa * mLightingMapSize;

		RasterizerSoft* rasterizer = new RasterizerSoft(this, width, height);
		rasterizer->DoRasterize2();

		for (int j = 0; j < mLightingMapSize; ++j)
		{
			for (int i = 0; i < mLightingMapSize; ++i)
			{
				Float3 color = Float3(0, 0, 0);

				for (int n = 0; n < msaa; ++n)
				{
					for (int m = 0; m < msaa; ++m)
					{
						int x = i * msaa + m;
						int y = j * msaa + n;

						const RVertex& p = rasterizer->_rchart[y * rasterizer->_width + x];
						color += baker.Calcu(p, LFX_MESH | LFX_TERRAIN, this);
					}
				}

				color /= (float)msaa * msaa;

				mLightingMap[j * mLightingMapSize + i] = mLightingMap[j * mLightingMapSize + i] * color;
			}
		}

		delete rasterizer;
	}

	void Mesh::PostProcess()
	{
#if 1
		if (mLightingMapSize > 0)
		{
			int xMapSize = mLightingMapSize;
			int zMapSize = mLightingMapSize;

			Rasterizer::DoBlur(&mLightingMap[0], xMapSize, zMapSize, xMapSize);
		}
#endif
	}

	Float3 Mesh::_doLighting(const Vertex & v, int mtlId, Light * pLight)
	{
		float kd = 0, ka = 0, ks = 0;

		DoLighting(kd, ka, ks, v, pLight);

		if (kd * ka * ks >= 0 && pLight->CastShadow && mReceiveShadow)
		{
			float len = 0;
			Ray ray;

			if (pLight->Type != Light::DIRECTION)
			{
				ray.dir = pLight->Position - v.Position;
				len = ray.dir.len();
				ray.dir.normalize();
			}
			else
			{
				ray.dir = -pLight->Direction;
				len = FLT_MAX;
			}

			ray.orig = v.Position + ray.dir * UNIT_LEN * 0.01f;

			if (len > 0.01f * UNIT_LEN)
			{
#if 0
				Contact contract;
				if (World::Instance()->RayCheck(contract, ray, len, eFlag::MESH))
				{
					kd = ka = ks = 0;
				}
#else
				if (World::Instance()->GetEmbreeScene()->Occluded(ray.orig, ray.dir, len, LFX_MESH | LFX_TERRAIN))
				{
					kd = ka = ks = 0;
				}
#endif
			}
		}

		Material * mtl = &mMtlBuffer[mtlId];

		return kd * ka * ks * mtl->diffuse * pLight->Color * pLight->DirectScale;
	}

	void Mesh::GetLightingMap(std::vector<RGBE> & colors)
	{
		assert(mLightingMapSize > 0);

		colors.resize(mLightingMap.size());

		for (int i = 0; i < colors.size(); ++i)
		{
			colors[i] = RGBE_FROM(mLightingMap[i]);
		}
	}

	void Mesh::GetLightingMap(std::vector<Float3> & colors)
	{
		assert(mLightingMapSize > 0);

		colors.resize(mLightingMap.size());

		for (int i = 0; i < colors.size(); ++i)
		{
			colors[i] = mLightingMap[i];
		}
	}

	void Mesh::GetGeometry(Vertex * pVertex, int * pIndex)
	{
		for (int i = 0; i < mVertexBuffer.size(); ++i)
		{
			pVertex[i] = mVertexBuffer[i];
		}
		
		for (int i = 0; i < mTriBuffer.size(); ++i)
		{
			pIndex[i * 3 + 0] = mTriBuffer[i].Index0;
			pIndex[i * 3 + 1] = mTriBuffer[i].Index1;
			pIndex[i * 3 + 2] = mTriBuffer[i].Index2;
		}
	}

	std::vector<Float3> & Mesh::_getLightingMap()
	{
		return mLightingMap;
	}

	void Mesh::GetLightList(std::vector<Light *> & lights, bool forGI)
	{
		for (int l = 0; l < World::Instance()->GetLightCount(); ++l)
		{
			Light * light = World::Instance()->GetLight(l);
			if (forGI && !light->GIEnable)
				continue;

			if (IsLightVisible(light, GetBound()))
			{
				lights.push_back(light);
			}
		}
	}

}