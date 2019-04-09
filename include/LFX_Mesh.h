#pragma once

#include "LFX_BSP.h"
#include "LFX_Light.h"
#include "LFX_Entity.h"

namespace LFX {

	class LFX_ENTRY Mesh : public Entity
	{
	public:
		Mesh(const String & name);
		virtual ~Mesh();

		virtual int GetType() { return LFX_MESH; }

		const String & GetName() { return mName; }
		
		void SetCastShadow(bool b) { mCastShadow = b; }
		bool GetCastShadow() { return mCastShadow; }

		void SetRecieveShadow(bool b) { mReceiveShadow = b; }
		bool GetRecieveShadow() { return mReceiveShadow; }

		void SetLightingMapSize(int size) { mLightingMapSize = size;}
		int GetLightingMapSize() { return mLightingMapSize; }

		int NumOfVertices() { return mVertexBuffer.size(); }
		int NumOfTriangles() { return mTriBuffer.size(); }
		int NumOfMaterial() { return mMtlBuffer.size(); }

		void Alloc(int numVertex, int numTriangle, int numMaterial);

		void Lock(Vertex ** ppVertex, Triangle ** ppTriangle, Material ** ppMaterial);
		void Unlock();

		void Build();
		bool Valid();
		const Aabb & GetBound();

		const Vertex & _getVertex(int i);
		const Triangle & _getTriangle(int i);

		void RayCheck(Contact & contract, const Ray & ray, float length);
		bool Occluded(const Ray & ray, float length);

		void CalcuDirectLighting(const std::vector<Light *> & lights);
		void CalcuIndirectLighting(const std::vector<Light *> & lights);
		void CalcuAmbientOcclusion();
		void PostProcess();
		Float3 _doLighting(const Vertex & v, int mtlId, Light * pLight);

		void GetLightingMap(std::vector<RGBE> & colors);
		void GetLightingMap(std::vector<Float3> & colors);
		void GetGeometry(Vertex * pVertex, int * pIndex);
		std::vector<Float3> & _getLightingMap();

		void GetLightList(std::vector<Light *> & lights, bool forGI);

	protected:
		bool _addTri(BSPTree<int>::Node * node, int triIndex);
		void _calcuTangent();
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
		void * mUserData;
		bool mCastShadow;
		bool mReceiveShadow;
		int mLightingMapSize;
		std::vector<Float3> mLightingMap;
	};

}