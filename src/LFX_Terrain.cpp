#include "LFX_World.h"
#include "LFX_Terrain.h"
#include "LFX_AOBaker.h"
#include "LFX_ILBakerRaytrace.h"
#include "LFX_EmbreeScene.h"

namespace LFX {

	struct TerrainUtil
	{
		static float GetHeight(float * heightfield, const Terrain::Desc & desc, int i, int j)
		{
			assert(i < desc.VertexCount.x && j < desc.VertexCount.y);

			float height = heightfield[j * desc.VertexCount.x + i];

			return height;
		}

		static Float3 GetPosition(float * heightfield, const Terrain::Desc & desc, int i, int j)
		{
			float fx = i * desc.GridSize;
			float fy = GetHeight(heightfield, desc, i, j);
			float fz = j * desc.GridSize;

			return Float3(fx, fy, fz);
		}

		static Float3 GetNormal(float * heightfield, const Terrain::Desc & desc, int i, int j)
		{
			float flip = 1;
			Float3 here = GetPosition(heightfield, desc, i, j);
			Float3 right, up;

			if (i < desc.VertexCount.x - 1)
			{
				right = GetPosition(heightfield, desc, i + 1, j);
			}
			else
			{
				flip *= -1;
				right = GetPosition(heightfield, desc, i - 1, j);
			}

			if (j < desc.VertexCount.y - 1)
			{
				up = GetPosition(heightfield, desc, i, j + 1);
			}
			else
			{
				flip *= -1;
				up = GetPosition(heightfield, desc, i, j - 1);
			}

			right -= here;
			up -= here;

			Float3 normal = Float3::Cross(up, right) * flip;
			normal.normalize();

			return normal;
		}
	};

	

	Terrain::Terrain(float * heightfiled, const Desc & desc)
	{
		mDesc = desc;

		int xGridCount = mDesc.GridCount.x;
		int zGridCount = mDesc.GridCount.y;

		mVertexBuffer.reserve((xGridCount + 1) * (zGridCount + 1));
		mTriBuffer.reserve(xGridCount * zGridCount * 2);

		for (int j = 0; j < zGridCount + 1; ++j)
		{
			for (int i = 0; i < xGridCount + 1; ++i)
			{
				Vertex v;
				v.Position = TerrainUtil::GetPosition(heightfiled, desc, i, j);
				v.Normal = TerrainUtil::GetNormal(heightfiled, desc, i, j);
				v.Tangent = Float3(1, 0, 0);
				v.Binormal = Float3(0, 0, 1);
				v.UV = Float2(0, 0);
				v.LUV = Float2((float)(i) / xGridCount, (float)(j) / zGridCount);

				v.Binormal = Float3::Cross(v.Normal, v.Tangent);
				v.Tangent = Float3::Cross(v.Binormal, v.Normal);

				mVertexBuffer.push_back(v);
			}
		}

		for (int j = 0; j < zGridCount; ++j)
		{
			int row = j * (xGridCount + 1);
			int row_n = row + (xGridCount + 1);

			for (int i = 0; i < xGridCount; ++i)
			{
				Triangle t1, t2;

				t1.Index0 = row + i;
				t1.Index1 = row + i + 1;
				t1.Index2 = row_n + i;

				t2.Index0 = row_n + i;
				t2.Index1 = row + i + 1;
				t2.Index2 = row_n + i + 1;

				mTriBuffer.push_back(t1);
				mTriBuffer.push_back(t2);
			}
		}

		int mapSize = mDesc.LMapSize - kLMapBorder * 2;
		mLightingMap.resize(mDesc.BlockCount.x * mDesc.BlockCount.y);
		for (int i = 0; i < mLightingMap.size(); ++i)
		{
			mLightingMap[i] = new Float3[mapSize * mapSize];
			memset(mLightingMap[i], 0, sizeof(Float3) * mapSize * mapSize);
		}

		mBlockValid.resize(mDesc.BlockCount.x * mDesc.BlockCount.y);
		for (int i = 0; i < mBlockValid.size(); ++i)
		{
			mBlockValid[i] = true;
		}
	}

	Terrain::~Terrain()
	{
		for (int i = 0; i < mLightingMap.size(); ++i)
		{
			delete mLightingMap[i];
		}
		mLightingMap.clear();
	}

	void Terrain::Build()
	{
		int mapSize = mDesc.LMapSize - kLMapBorder * 2;

		mMapSizeU = mapSize * mDesc.BlockCount.x;
		mMapSizeV = mapSize * mDesc.BlockCount.y;
		
		Aabb bound;
		bound.minimum = mVertexBuffer[0].Position;
		bound.maximum = mVertexBuffer[0].Position;

		for (int i = 1; i < mVertexBuffer.size(); ++i)
		{
			bound.minimum = Minimum(bound.minimum, mVertexBuffer[i].Position);
			bound.maximum = Maximum(bound.maximum, mVertexBuffer[i].Position);
		}

		mBSPTree.Build(bound, 8);

		for (int i = 0; i < mTriBuffer.size(); ++i)
		{
			bool hr = _addTri(mBSPTree.RootNode(), i);

			assert(hr != NULL);
		}
	}

	const Vertex & Terrain::_getVertex(int i)
	{
		return mVertexBuffer[i];
	}

	const Triangle & Terrain::_getTriangle(int i)
	{
		return mTriBuffer[i];
	}

	const Vertex & Terrain::_getVertex(int i, int j)
	{
		assert(i < mDesc.VertexCount.x && j < mDesc.VertexCount.y);

		Vertex & v = mVertexBuffer[j * mDesc.VertexCount.x + i];

		return v;
	}

	const Float3 & Terrain::_getPosition(int i, int j)
	{
		assert(i < mDesc.VertexCount.x && j < mDesc.VertexCount.y);

		Vertex & v = mVertexBuffer[j * mDesc.VertexCount.x + i];

		return v.Position;
	}

	const Float3 & Terrain::_getNormal(int i, int j)
	{
		assert(i < mDesc.VertexCount.x && j < mDesc.VertexCount.y);

		Vertex & v = mVertexBuffer[j * mDesc.VertexCount.x + i];

		return v.Normal;
	}

	bool Terrain::GetHeightAt(float & h, float x, float z)
	{
		float fx = x / mDesc.GridSize;
		float fz = z / mDesc.GridSize;

		int ix0 = (int)fx;
		int iz0 = (int)fz;

		if (ix0 < 0 || ix0 > mDesc.VertexCount.x - 1 ||
			iz0 < 0 || iz0 > mDesc.VertexCount.y - 1)
			return false;

		float dx = fx - ix0;
		float dz = fz - iz0;

		int ix1 = ix0 + 1;
		int iz1 = iz0 + 1;

		ix1 = std::min(ix1, mDesc.VertexCount.x - 1);
		iz1 = std::min(iz1, mDesc.VertexCount.y - 1);

		float a = _getPosition(ix0, iz0).y;
		float b = _getPosition(ix1, iz0).y;
		float c = _getPosition(ix0, iz1).y;
		float d = _getPosition(ix1, iz1).y;
		float m = (b + c) * 0.5f;
		float h1, h2;

		if (dx + dz <= 1.0f)
		{
			d = m + (m - a);
		}
		else
		{
			a = m + (m - d);
		}

		h1 = a * (1.0f - dx) + b * dx;
		h2 = c * (1.0f - dx) + d * dx;

		h = h1 * (1.0f - dz) + h2 * dz;

		return true;
	}

	bool Terrain::GetNormalAt(Float3 & n, float x, float y)
	{
		float fx = x / mDesc.GridSize;
		float fz = y / mDesc.GridSize;

		int ix0 = (int)fx;
		int iz0 = (int)fz;

		if (ix0 < 0 || ix0 > mDesc.VertexCount.x - 1 ||
			iz0 < 0 || iz0 > mDesc.VertexCount.y - 1)
			return false;

		float dx = fx - ix0;
		float dz = fz - iz0;

		int ix1 = ix0 + 1;
		int iz1 = iz0 + 1;

		ix1 = std::min(ix1, mDesc.VertexCount.x - 1);
		iz1 = std::min(iz1, mDesc.VertexCount.y - 1);

		Float3 a = _getNormal(ix0, iz0);
		Float3 b = _getNormal(ix1, iz0);
		Float3 c = _getNormal(ix0, iz1);
		Float3 d = _getNormal(ix1, iz1);
		Float3 m = (b + c) * 0.5f;
		Float3 h1, h2;

		if (dx + dz <= 1.0f)
		{
			d = m + (m - a);
		}
		else
		{
			a = m + (m - d);
		}

		h1 = a * (1.0f - dx) + b * dx;
		h2 = c * (1.0f - dx) + d * dx;

		n = h1 * (1.0f - dz) + h2 * dz;
		n.normalize();

		return true;
	}

	void Terrain::_rayCheckImp(Contact & contract, BSPTree<int>::Node * node, const Ray & ray, float length)
	{
		float dist = 0;

		if (!Intersect(ray, &dist, node->aabb) || dist >= contract.td || dist >= length)
			return ;

		for (int i = 0; i < node->elems.size(); ++i)
		{
			int triIndex = node->elems[i];
			const Triangle & triangle = mTriBuffer[triIndex];

			const Float3 & a = mVertexBuffer[triangle.Index0].Position;
			const Float3 & b = mVertexBuffer[triangle.Index1].Position;
			const Float3 & c = mVertexBuffer[triangle.Index2].Position;

			float tu, tv;
			if (Intersect(ray, &dist, tu, tv, a, b, c) && dist < contract.td && dist <= length)
			{
				contract.td = dist;
				contract.triIndex = triIndex;
				contract.tu = tu;
				contract.tv = tv;
				contract.entity = this;
				contract.mtl = &mMaterial;

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

	void Terrain::RayCheck(Contact & contract, const Ray & ray, float length)
	{
		assert(mBSPTree.RootNode() != NULL);

		_rayCheckImp(contract, mBSPTree.RootNode(), ray, length);
	}

	bool Terrain::_occludedImp(BSPTree<int>::Node * node, const Ray & ray, float length)
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

			float tu, tv;
			if (Intersect(ray, &dist, tu, tv, a, b, c) && dist <= length)
				return true;
		}

		if (node->child[0] != NULL && _occludedImp(node->child[0], ray, length))
			return true;

		if (node->child[1] != NULL && _occludedImp(node->child[1], ray, length))
			return true;

		return false;
	}

	bool Terrain::Occluded(const Ray & ray, float length)
	{
		assert(mBSPTree.RootNode() != NULL);

		return _occludedImp(mBSPTree.RootNode(), ray, length);
	}

	bool Terrain::_addTri(BSPTree<int>::Node * node, int t)
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

	void Terrain::CalcuDirectLighting(int xblock, int yblock, const std::vector<Light *> & lights)
	{
		if (World::Instance()->GetSetting()->Selected) return;

		int mapSize = mDesc.LMapSize - Terrain::kLMapBorder * 2;
		int msaa = World::Instance()->GetSetting()->GetBeastMSAA(mapSize, mapSize);

		int sx = mapSize * xblock;
		int sy = mapSize * yblock;
		Float3 * lmap = mLightingMap[yblock * mDesc.BlockCount.x + xblock];

		for (int line = 0; line < mapSize; ++line)
		{
			int j = sy + line;
			for (int i = sx; i < sx + mapSize; ++i)
			{
				Float3 color(0, 0, 0);

				for (int y = 0; y < msaa; ++y)
				{
					for (int x = 0; x < msaa; ++x)
					{
						float u = (i + x / (float)msaa) / (mMapSizeU - 1);
						float v = (j + y / (float)msaa) / (mMapSizeV - 1);

						Vertex p;
						p.Position.x = u * mDesc.Dimension.x;
						p.Position.z = v * mDesc.Dimension.y;
						p.UV = Float2(0, 0);
						p.LUV = Float2(0, 0);

						GetHeightAt(p.Position.y, p.Position.x, p.Position.z);
						GetNormalAt(p.Normal, p.Position.x, p.Position.z);

						for (int l = 0; l < lights.size(); ++l)
						{
							color += _doLighting(p, lights[l]);
						}
					}
				}

				color /= (float)msaa * msaa;

				lmap[(j - sy) * mapSize + (i - sx)] = color + World::Instance()->GetSetting()->Ambient;
			}
		}
	}

	void Terrain::CalcuIndirectLighting(int xblock, int yblock, const std::vector<Light *> & lights)
	{
		if (World::Instance()->GetSetting()->Selected) return;

		int mapSize = mDesc.LMapSize - Terrain::kLMapBorder * 2;
#ifdef LFX_FEATURE_INDIRECT_MSAA
		int msaa = World::Instance()->GetSetting()->MSAA;
#else
		int msaa = 1;
#endif

		int sx = mapSize * xblock;
		int sy = mapSize * yblock;

		RasterizerSoft * rasterizer = new RasterizerSoft(NULL, mapSize * msaa, mapSize * msaa);

		int index = 0;
		for (int l = 0; l < mapSize; ++l)
		{
			int j = sy + l;
			for (int i = sx; i < sx + mapSize; ++i)
			{
				for (int y = 0; y < msaa; ++y)
				{
					for (int x = 0; x < msaa; ++x)
					{
						float u = (i + x / (float)msaa) / (mMapSizeU - 1);
						float v = (j + y / (float)msaa) / (mMapSizeV - 1);

						RVertex p;
						p.Position.x = u * mDesc.Dimension.x;
						p.Position.z = v * mDesc.Dimension.y;
						p.Tangent = Float3(1, 0, 0);
						p.Binormal = Float3(0, 0, 1);
						p.UV = Float2(0, 0);
						p.LUV = Float2(0, 0);
						p.MaterialId = 0;

						GetHeightAt(p.Position.y, p.Position.x, p.Position.z);
						GetNormalAt(p.Normal, p.Position.x, p.Position.z);

						p.Binormal = Float3::Cross(p.Normal, p.Tangent);
						p.Tangent = Float3::Cross(p.Binormal, p.Normal);

						rasterizer->_rchart[index++] = p;
					}
				}
			}
		}

		//
		ILBakerRaytrace baker;
		baker._cfg.SkyRadiance = World::Instance()->GetSetting()->SkyRadiance;
		baker._cfg.DiffuseScale = World::Instance()->GetSetting()->GIScale;
		baker._cfg.SqrtNumSamples = World::Instance()->GetSetting()->GISamples;
		baker._cfg.MaxPathLength = World::Instance()->GetSetting()->GIPathLength;
		baker._cfg.RussianRouletteDepth = -1;

		baker.Run(rasterizer);

		Float3 * lmap = mLightingMap[yblock * mDesc.BlockCount.x + xblock];
		for (int j = 0; j < mapSize; ++j)
		{ 
			for (int i = 0; i < mapSize; ++i)
			{
				Float3 color = Float3(0, 0, 0);
				for (int y = 0; y < msaa; ++y)
				{
					for (int x = 0; x < msaa; ++x)
					{
						int u = i * msaa + x;
						int v = j * msaa + y;

						color.x += baker._ctx.BakeOutput[v * mapSize * msaa + u].x;
						color.y += baker._ctx.BakeOutput[v * mapSize * msaa + u].y;
						color.z += baker._ctx.BakeOutput[v * mapSize * msaa + u].z;
					}
				}
				color /= (float)msaa * msaa;

				lmap[(j - 0) * mapSize + (i - 0)] += color;
			}
		}

		delete rasterizer;
	}

	void Terrain::GetLightingMap(int i, int j, std::vector<RGBE> & colors)
	{
		int mapSize = mDesc.LMapSize;
		Float3 * lmap = mLightingMap[j * mDesc.BlockCount.x + i];
		int lmapSize = mapSize - Terrain::kLMapBorder * 2;

		int index = 0;
		for (int y = 0; y < mapSize; ++y)
		{
			for (int x = 0; x < mapSize; ++x)
			{
				int xIndex = x - Terrain::kLMapBorder;
				int zIndex = y - Terrain::kLMapBorder;

				xIndex = Clamp<int>(xIndex, 0, lmapSize - 1);
				zIndex = Clamp<int>(zIndex, 0, lmapSize - 1);

				colors[index++] = RGBE_FROM(lmap[zIndex * lmapSize + xIndex]);
			}
		}
	}

	void Terrain::GetLightingMap(int i, int j, std::vector<Float3> & colors)
	{
		int mapSize = mDesc.LMapSize;
		Float3 * lmap = mLightingMap[j * mDesc.BlockCount.x + i];
		int lmapSize = mapSize - Terrain::kLMapBorder * 2;

		int index = 0;
		for (int y = 0; y < mapSize; ++y)
		{
			for (int x = 0; x < mapSize; ++x)
			{
				int xIndex = x - Terrain::kLMapBorder;
				int zIndex = y - Terrain::kLMapBorder;

				xIndex = Clamp<int>(xIndex, 0, lmapSize - 1);
				zIndex = Clamp<int>(zIndex, 0, lmapSize - 1);

				colors[index++] = lmap[zIndex * lmapSize + xIndex];
			}
		}
	}

	Float3 * Terrain::_getLightingMap(int xBlock, int zBlock)
	{
		return mLightingMap[zBlock * mDesc.BlockCount.x + xBlock];
	}

	void Terrain::GetBlockGeometry(int i, int j, Vertex * vbuff, int * ibuff)
	{
		int grids = mDesc.GridCount.x / mDesc.BlockCount.x;
		int xStart = i * grids;
		int zStart = j * grids;

		int index = 0;
		for (int y = 0; y < grids + 1; ++y)
		{
			for (int x = 0; x < grids + 1; ++x)
			{
				int xIndex = xStart + x;
				int zIndex = zStart + y;

				Vertex v = _getVertex(xIndex, zIndex);

				vbuff[index] = v;
				vbuff[index].LUV.x = x / (float)grids;
				vbuff[index].LUV.y = y / (float)grids;
				++index;
			}
		}

		for (int y = 0; y < grids; ++y)
		{
			for (int x = 0; x < grids; ++x)
			{
				int a = y * (grids + 1) + x;
				int b = y * (grids + 1) + x + 1;
				int c = (y + 1) * (grids + 1) + x;
				int d = (y + 1) * (grids + 1) + x + 1;

				*ibuff++ = a;
				*ibuff++ = c;
				*ibuff++ = b;

				*ibuff++ = b;
				*ibuff++ = c;
				*ibuff++ = d;
			}
		}
	}

	void Terrain::GetLightList(std::vector<Light *> & lights, int xblock, int yblock, bool forGI)
	{
		float blockSize = GetDesc().Dimension.x / GetDesc().BlockCount.x;

		Aabb bound;
		bound.minimum.x = xblock * blockSize;
		bound.minimum.y = 0;
		bound.minimum.z = yblock * blockSize;
		bound.maximum.x = xblock * blockSize + blockSize;
		bound.maximum.y = 10000;
		bound.maximum.z = yblock * blockSize + blockSize;

		for (int j = 0; j < World::Instance()->GetLightCount(); ++j)
		{
			Light * light = World::Instance()->GetLight(j);
			if (forGI && !light->GIEnable)
				continue;

			if (IsLightVisible(light, bound))
			{
				lights.push_back(light);
			}
		}
	}

	Float3 Terrain::_doLighting(const Vertex & v, Light * pLight)
	{
		float kd = 0, ka = 0, ks = 0;

		DoLighting(kd, ka, ks, v, pLight);

		if (kd * ka * ks >= 0 && pLight->CastShadow)
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
				if (World::Instance()->Occluded(ray, len, LFX_MESH))
				{
					kd = ka = ks = 0;
				}
			}
		}

		return kd * ka * ks * mMaterial.diffuse * pLight->Color * pLight->DirectScale;
	}

	void Terrain::CalcuAmbientOcclusion(int xblock, int yblock)
	{
		if (World::Instance()->GetSetting()->Selected) return;

		AOBaker baker;

		int mapSize = mDesc.LMapSize - Terrain::kLMapBorder * 2;
		int msaa = World::Instance()->GetSetting()->MSAA;

		int sx = mapSize * xblock;
		int sy = mapSize * yblock;

		Float3 * lmap = mLightingMap[yblock * mDesc.BlockCount.x + xblock];
		for (int l = 0; l < mapSize; ++l)
		{
			int j = sy + l;
			for (int i = sx; i < sx + mapSize; ++i)
			{
				Float3 color = Float3(0, 0, 0);

				for (int y = 0; y < msaa; ++y)
				{
					for (int x = 0; x < msaa; ++x)
					{
						float u = (i + x / (float)msaa) / (mMapSizeU - 1);
						float v = (j + y / (float)msaa) / (mMapSizeV - 1);

						RVertex p;
						p.Position.x = u * mDesc.Dimension.x;
						p.Position.z = v * mDesc.Dimension.y;
						p.Tangent = Float3(1, 0, 0);
						p.Binormal = Float3(0, 0, 1);
						p.UV = Float2(0, 0);
						p.LUV = Float2(0, 0);
						p.MaterialId = 0;

						GetHeightAt(p.Position.y, p.Position.x, p.Position.z);
						GetNormalAt(p.Normal, p.Position.x, p.Position.z);

						p.Binormal = Float3::Cross(p.Normal, p.Tangent);
						p.Tangent = Float3::Cross(p.Binormal, p.Normal);

						color += baker.Calcu(p, LFX_MESH, this);
					}
				}

				color /= (float)msaa * msaa;

				lmap[(j - 0) * mapSize + (i - 0)] += color;
			}
		}
	}

	void Terrain::PostProcess(int xblock, int yblock)
	{
#if 1
		Float3 * lmap = mLightingMap[yblock * mDesc.BlockCount.x + xblock];
		int mapSize = mDesc.LMapSize - Terrain::kLMapBorder * 2;

		Rasterizer::DoBlur(lmap, mapSize, mapSize, mapSize);
#endif
	}

}