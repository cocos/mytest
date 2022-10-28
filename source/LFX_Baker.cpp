#include "LFX_Baker.h"
#include "LFX_World.h"

namespace LFX {

	STBaker::STBaker(int id)
	{
		mId = id;
		mEntity = NULL;
		mIndex = 0;
		mCompeleted = true;
	}

	STBaker::~STBaker()
	{
	}

	void STBaker::Run()
	{
		while (1) {
			if (mStatus == STOP)
				break;

			if (mCompeleted)
				continue;

			bool hasLightForGI = false;
			for (auto* light : World::Instance()->Lights()) {
				if (light->GIEnable) {
					hasLightForGI = true;
					break;
				}
			}

			if (mEntity->GetType() == LFX_TERRAIN) {
				_calcuDirectLightingTerrain();
				if (hasLightForGI) {
					_calcuIndirectLightingTerrain();
				}
				_calcuAmbientOcclusionTerrain();
				_postProcess();
			}
			else if (mEntity->GetType() == LFX_MESH) {
				_calcuDirectLightingMesh();
				if (hasLightForGI) {
					_calcuIndirectLightingMesh();
				}
				_calcuAmbientOcclusionMesh();
				_postProcess();
			}
			else if (mEntity->GetType() == LFX_SHPROBE) {
				_calcuSHProbe();
			}
			else {
				assert(0 && "Invalid entity!");
			}

			World::Instance()->_onThreadCompeleted();

			mCompeleted = true;
		}
	}

	void STBaker::Enqueue(Entity * entity, int index)
	{
		mEntity = entity;
		mIndex = index;

		mCompeleted = false;
	}

	void STBaker::_calcuDirectLightingMesh()
	{
		Mesh * pMesh = (Mesh*)mEntity;
		if (pMesh->GetLightingMapSize())
		{
			std::vector<Light *> lights;
			for (auto* light: World::Instance()->Lights())
			{
				if (IsLightVisible(light, pMesh->GetBound()))
				{
					lights.push_back(light);
				}
			}

			if (lights.size() > 0) {
				pMesh->CalcuDirectLighting(lights);
			}
		}
	}

	void STBaker::_calcuDirectLightingTerrain()
	{
		Terrain * pTerrain = (Terrain*)mEntity;

		float blockSize = pTerrain->GetDesc().Dimension.x / pTerrain->GetDesc().BlockCount.x;

		int xblock = mIndex % pTerrain->GetDesc().BlockCount.x;
		int yblock = mIndex / pTerrain->GetDesc().BlockCount.x;

		Aabb bound;
		bound.minimum.x = xblock * blockSize;
		bound.minimum.y = 0;
		bound.minimum.z = yblock * blockSize;
		bound.maximum.x = xblock * blockSize + blockSize;
		bound.maximum.y = 10000;
		bound.maximum.z = yblock * blockSize + blockSize;
		bound.minimum += pTerrain->GetDesc().Position;
		bound.maximum += pTerrain->GetDesc().Position;

		std::vector<Light *> lights;
		for (auto* light : World::Instance()->Lights())
		{
			if (IsLightVisible(light, bound))
			{
				lights.push_back(light);
			}
		}

		if (lights.size() > 0) {
			pTerrain->CalcuDirectLighting(xblock, yblock, lights);
		}
	}

	void STBaker::_calcuIndirectLightingMesh()
	{
		if (World::Instance()->GetSetting()->GIScale > 0) {
			Mesh* pMesh = World::Instance()->Meshes()[mIndex];

			if (pMesh->GetLightingMapSize()) {
				pMesh->CalcuIndirectLighting();
			}
		}
	}

	void STBaker::_calcuIndirectLightingTerrain()
	{
		if (World::Instance()->GetSetting()->GIScale > 0) {
			Terrain* pTerrain = (Terrain*)mEntity;

			float blockSize = pTerrain->GetDesc().Dimension.x / pTerrain->GetDesc().BlockCount.x;
			int xblock = mIndex % pTerrain->GetDesc().BlockCount.x;
			int yblock = mIndex / pTerrain->GetDesc().BlockCount.x;

			pTerrain->CalcuIndirectLighting(xblock, yblock);
		}
	}

	void STBaker::_calcuAmbientOcclusionMesh()
	{
		if (World::Instance()->GetSetting()->AOLevel > 0) {
			Mesh* pMesh = World::Instance()->Meshes()[mIndex];

			if (pMesh->GetLightingMapSize() > 0) {
				pMesh->CalcuAmbientOcclusion();
			}
		}
	}

	void STBaker::_calcuAmbientOcclusionTerrain()
	{
		if (World::Instance()->GetSetting()->AOLevel > 0) {
			//...
		}
	}

	void STBaker::_calcuSHProbe()
	{
		SHBaker baker;
		baker.Run((SHProbe*)mEntity);
	}

	void STBaker::_postProcess()
	{
		if (mEntity->GetType() == LFX_TERRAIN)
		{
			Terrain * pTerrain = (Terrain*)mEntity;
			int xblock = mIndex % pTerrain->GetDesc().BlockCount.x;
			int yblock = mIndex / pTerrain->GetDesc().BlockCount.x;

			pTerrain->PostProcess(xblock, yblock);
		}
	}

}

