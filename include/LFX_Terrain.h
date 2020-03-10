#pragma once

#include "LFX_BSP.h"
#include "LFX_Types.h"
#include "LFX_Entity.h"
#include "LFX_Light.h"

namespace LFX {

	class LFX_ENTRY Terrain : public Entity
	{
	public:
		enum {
			kLMapBorder = 2
		};

		struct Desc
		{
			int LMapSize;
			float GridSize;
			Float2 Dimension;
			Int2 VertexCount;
			Int2 GridCount;
			Int2 BlockCount;

			Desc()
			{
				LMapSize = 256;
				GridSize = 1.0f;
			}
		};

	public:
		Terrain(const Float3& pos, float * heightfiled, const Desc & desc);
		virtual ~Terrain();

		virtual int GetType() { return LFX_TERRAIN; }

		const Desc & GetDesc() { return mDesc; }
		Material * GetMaterial() { return &mMaterial; }

		void Build();

		std::vector<bool> & _getBlockValids() { return mBlockValid; }

		const Vertex & _getVertex(int i);
		const Triangle & _getTriangle(int i);
		const Vertex & _getVertex(int i, int j);
		const Float3 & _getPosition(int i, int j);
		const Float3 & _getNormal(int i, int j);
		bool GetHeightAt(float & h, float x, float y);
		bool GetNormalAt(Float3 & n, float x, float y);

		void RayCheck(Contact & contract, const Ray & ray, float length);
		bool Occluded(const Ray & ray, float length);

		void CalcuDirectLighting(int xblock, int yblock, const std::vector<Light *> & lights);
		void CalcuIndirectLighting(int xblock, int yblock, const std::vector<Light *> & lights);
		void CalcuAmbientOcclusion(int xblock, int yblock);
		void PostProcess(int xblock, int yblock);

		void GetLightingMap(int xBlock, int zBlock, std::vector<RGBE> & colors);
		void GetLightingMap(int xBlock, int zBlock, std::vector<Float3> & colors);
		void GetBlockGeometry(int xBlock, int zBlock, Vertex * vbuff, int * ibuff);
		Float3 * _getLightingMap(int xBlock, int zBlock);
		std::vector<Vertex> & _getVertexBuffer() { return mVertexBuffer; }
		std::vector<Triangle> & _getTriBuffer() { return mTriBuffer; }

		void GetLightList(std::vector<Light *> & lights, int xBlock, int zBlock, bool forGI);

	protected:
		bool _addTri(BSPTree<int>::Node * node, int triIndex);
		void _rayCheckImp(Contact & contract, BSPTree<int>::Node * node, const Ray & ray, float length);
		bool _occludedImp(BSPTree<int>::Node * node, const Ray & ray, float length);

		Float3 _doLighting(const Vertex & v, Light * pLight);

	protected:
		Desc mDesc;
		std::vector<Vertex> mVertexBuffer;
		std::vector<Triangle> mTriBuffer;
		BSPTree<int> mBSPTree;

		int mMapSizeU;
		int mMapSizeV;
		std::vector<Float3 *> mLightingMap;
		std::vector<bool> mBlockValid;

		Material mMaterial;
	};

}