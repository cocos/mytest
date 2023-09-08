#pragma once

#include "LFX_BSP.h"
#include "LFX_Light.h"
#include "LFX_Entity.h"

namespace LFX {

	class LFX_ENTRY Mesh : public Entity
	{
	public:
		Mesh();
		virtual ~Mesh();

		virtual int GetType() { return LFX_MESH; }

		void SetName(const String& name);
		const String& GetName() const { return mName; }

		void SetCastShadow(bool b) { mCastShadow = b; }
		bool GetCastShadow() { return mCastShadow; }

		void SetRecieveShadow(bool b) { mReceiveShadow = b; }
		bool GetRecieveShadow() { return mReceiveShadow; }

		void SetLightingMapSize(int size) { mLightingMapSize = size;}
		int GetLightingMapSize() { return mLightingMapSize; }

		void Alloc(int numVertex, int numTriangle, int numMaterial);

		void Lock(Vertex ** ppVertex, Triangle ** ppTriangle, Material ** ppMaterial);
		void Unlock();

		void Build();
		bool Valid();
		const Aabb & GetBound();

		int NumOfVertices() { return mVertexBuffer.size(); }
		int NumOfTriangles() { return mTriBuffer.size(); }
		int NumOfMaterial() { return mMtlBuffer.size(); }
		const auto& _getVertex(int i) { return mVertexBuffer[i]; }
		const auto& _getTriangle(int i) { return mTriBuffer[i]; }
		const auto& _getMaterial(int i) { return mMtlBuffer[i]; }
		const auto& _getVertexBuffer() { return mVertexBuffer; }
		const auto& _getTriangleBuffer() { return mTriBuffer; }
		const auto& _getMaterialBuffer() { return mMtlBuffer; }

		void RayCheck(Contact & contract, const Ray & ray, float length);
		bool Occluded(const Ray & ray, float length);

		void CalcuDirectLighting(const std::vector<Light *> & lights);
		void CalcuIndirectLighting();
		void CalcuAmbientOcclusion();
		Float3 _doDirectLighting(const Vertex& v, int mtlId, Light* pLight, float& shadowMask);

		void GetLightingMap(std::vector<RGBE> & colors);
		void GetLightingMap(std::vector<LightmapValue> & colors);
		void GetGeometry(Vertex * pVertex, int * pIndex);
		std::vector<LightmapValue> & _getLightingMap();

		void GetLightList(std::vector<Light *> & lights, bool forGI);

	protected:
		bool _addTri(BSPTree<int>::Node * node, int triIndex);
		void _generateTangent();
		Aabb _optimize(BSPTree<int>::Node * node);

		void _rayCheckImp(Contact & contract, BSPTree<int>::Node * node, const Ray & ray, float length);
		bool _occludedImp(BSPTree<int>::Node * node, const Ray & ray, float length);

	protected:
		std::vector<Vertex> mVertexBuffer;
		std::vector<Triangle> mTriBuffer;
		std::vector<Material> mMtlBuffer;
		BSPTree<int> mBSPTree;
		Float2 mUVMin, mUVMax;

		String mName;
		bool mCastShadow;
		bool mReceiveShadow;
		int mLightingMapSize;
		std::vector<LightmapValue> mLightingMap;
	};

}