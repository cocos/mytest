#include "LFX_Mesh.h"
#include "LFX_World.h"
#include "LFX_AOBaker.h"
#include "LFX_RasterizerSoft.h"
#include "LFX_RasterizerScan2.h"
//#include "LFX_RasterizerZSpan.h"
#include "LFX_ILBakerRaytrace.h"
#include "LFX_EmbreeScene.h"

namespace LFX {

	Mesh::Mesh()
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
				mLightingMap[i] = Float4(0, 0, 0, 1);
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
				if (m.Maps[0] != NULL)
				{
					const Float2 & uv0 = mVertexBuffer[triangle.Index0].UV;
					const Float2 & uv1 = mVertexBuffer[triangle.Index1].UV;
					const Float2 & uv2 = mVertexBuffer[triangle.Index2].UV;

					Float2 uv = uv0 * (1 - tu - tv) + uv1 * tu + uv2 * tv;

					Float4 color = m.Maps[0]->SampleColor(uv.x, uv.y);
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
				if (m.Maps[0] != NULL)
				{
					const Float2 & uv0 = mVertexBuffer[triangle.Index0].UV;
					const Float2 & uv1 = mVertexBuffer[triangle.Index1].UV;
					const Float2 & uv2 = mVertexBuffer[triangle.Index2].UV;

					Float2 uv = uv0 * (1 - tu - tv) + uv1 * tu + uv2 * tv;

					Float4 color = m.Maps[0]->SampleColor(uv.x, uv.y);
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

		const int width = mLightingMapSize;
		const int height = mLightingMapSize;
		const int msaa = World::Instance()->GetSetting()->MSAA;
		const int border = LMAP_BORDER;

		std::vector<Float4> lmap(width * height);
		std::vector<float> mmap(width * height);

		for (int i = 0; i < mmap.size(); ++i) {
			mmap[i] = 0.0f;
		}

		RasterizerScan2 rs(this, width, height, msaa, border);
		rs.F = [this, &lmap, &mmap, width, msaa, &lights](const Float2& texel, const Vertex& v, int mtlId) {
			int x = static_cast<int>(texel.x);
			int y = static_cast<int>(texel.y);

			float shadowMask = 1.0f;
			Float3 color = Float3(0, 0, 0);
			for (auto* light : lights)
			{
#ifndef LFX_DEBUG_LUV
				color += _doLighting(v, mtlId, light, shadowMask);
#else
				color += Float3(0.5f, 0.5f, 0.5f);
#endif
				//color += Float3::ONE;
			}

			lmap[y * width + x] += Float4(color.x, color.y, color.z, 1/*samples*/);
#ifdef LFX_DEBUG_LUV
			lmap[y * width + x].w = 1;
#endif
			mmap[y * width + x] += shadowMask;
		};
		rs.DoRasterize();

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				Float4 c = lmap[y * width + x];
				if (c.w > 1) {
					float invSampels = 1.0f / c.w;
					c.x *= invSampels;
					c.y *= invSampels;
					c.z *= invSampels;
					lmap[y * width + x] = c;
					mmap[y * width + x] *= invSampels;
				}
			}
		}

		if (LMAP_OPTIMIZE_PX > 0) {
			Rasterizer::Optimize(&lmap[0], width, height, LMAP_OPTIMIZE_PX);
		}

		for (int j = 0; j < height; ++j)
		{
			for (int i = 0; i < width; ++i)
			{
				int index = j * width + i;
				Float4 color = lmap[index];
				float mask = mmap[index];
				//color = Float4(1, 1, 1, 1);
				mLightingMap[index] = Float4(color.x, color.y, color.z, mask);
			}
		}
	}

	void Mesh::CalcuIndirectLighting()
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

		RasterizerSoft* rasterizer = new RasterizerSoft(this, width, height);
		rasterizer->DoRasterize();

		ILBakerRaytrace baker;
		baker._cfg.SkyRadiance = World::Instance()->GetSetting()->SkyRadiance;
		baker._cfg.DiffuseScale = World::Instance()->GetSetting()->GIScale;
		baker._cfg.SqrtNumSamples = World::Instance()->GetSetting()->GISamples;
		baker._cfg.MaxPathLength = World::Instance()->GetSetting()->GIPathLength;
		baker._cfg.RussianRouletteDepth = -1;
		baker.Run(this, rasterizer->_width, rasterizer->_height, rasterizer->_rchart);

		int index = 0;
		std::vector<LFX::Float4> & ilm = this->_getLightingMap();
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

				auto& outColor = ilm[index++];
				outColor.x += color.x;
				outColor.y += color.y;
				outColor.z += color.z;
			}
		}

		delete rasterizer;
	}

	void Mesh::CalcuAmbientOcclusion()
	{
		AOBaker baker;

		int msaa = World::Instance()->GetSetting()->MSAA;
		int width = msaa * mLightingMapSize;
		int height = msaa * mLightingMapSize;

		RasterizerSoft* rasterizer = new RasterizerSoft(this, width, height);

		rasterizer->DoRasterize();

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

						const auto& p = rasterizer->_rchart[y * rasterizer->_width + x];
						color += baker.Calc(p, LFX_MESH | LFX_TERRAIN, this);
					}
				}

				color /= (float)msaa * msaa;

				auto& outColor = mLightingMap[j * mLightingMapSize + i];
				outColor.x *= color.x;
				outColor.y *= color.y;
				outColor.z *= color.z;
			}
		}

		delete rasterizer;
	}

	Float3 Mesh::_doLighting(const Vertex& v, int mtlId, Light* pLight, float& shadowMask)
	{
		float kl = 0.0f;
		Float3 color;

		if (pLight->DirectScale > 0)
		{
			World::Instance()->GetShader()->DoLighting(color, kl, v, pLight, &mMtlBuffer[mtlId]);
			if (kl >= 0 && pLight->CastShadow && mReceiveShadow)
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
					if (World::Instance()->GetScene()->Occluded(ray, len, LFX_MESH | LFX_TERRAIN))
					{
						kl = 0.0f;
						shadowMask = 0.0f;
					}
				}
			}
		}

		return kl > 0 ? color * pLight->DirectScale : Float3(0, 0, 0);
	}

	void Mesh::GetLightingMap(std::vector<RGBE> & colors)
	{
		assert(mLightingMapSize > 0);

		colors.resize(mLightingMap.size());

		for (int i = 0; i < colors.size(); ++i)
		{
			colors[i] = RGBE_FROM(Float3(mLightingMap[i].x, mLightingMap[i].y, mLightingMap[i].z));
		}
	}

	void Mesh::GetLightingMap(std::vector<Float4> & colors)
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

	std::vector<Float4> & Mesh::_getLightingMap()
	{
		return mLightingMap;
	}

	void Mesh::GetLightList(std::vector<Light *> & lights, bool forGI)
	{
		for (Light* light : World::Instance()->Lights())
		{
			if (forGI && !light->GIEnable) {
				continue;
			}

			if (IsLightVisible(light, GetBound()))
			{
				lights.push_back(light);
			}
		}
	}

}