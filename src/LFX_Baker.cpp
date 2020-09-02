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

			if (mEntity->GetType() == LFX_TERRAIN) {
				_calcuDirectLightingTerrain();
				_calcuIndirectLightingTerrain();
				_calcuAmbientOcclusionTerrain();
				_postProcess();
			}
			else {
				_calcuDirectLightingMesh();
				_calcuIndirectLightingMesh();
				_calcuAmbientOcclusionMesh();
				_postProcess();
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
			for (int l = 0; l < World::Instance()->GetLightCount(); ++l)
			{
				Light * light = World::Instance()->GetLight(l);
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
		int yblock = mIndex / pTerrain->GetDesc().BlockCount.y;

		Aabb bound;
		bound.minimum.x = xblock * blockSize;
		bound.minimum.y = 0;
		bound.minimum.z = yblock * blockSize;
		bound.maximum.x = xblock * blockSize + blockSize;
		bound.maximum.y = 10000;
		bound.maximum.z = yblock * blockSize + blockSize;

		std::vector<Light *> lights;
		for (int j = 0; j < World::Instance()->GetLightCount(); ++j)
		{
			Light * light = World::Instance()->GetLight(j);
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
			Mesh * pMesh = World::Instance()->GetMesh(mIndex);

			if (pMesh->GetLightingMapSize()) {
				std::vector<Light *> lights;
				pMesh->GetLightList(lights, true);

				if (lights.size() > 0) {
					pMesh->CalcuIndirectLighting(lights);
				}
			}
		}
	}

	void STBaker::_calcuIndirectLightingTerrain()
	{
		if (World::Instance()->GetSetting()->GIScale > 0) {
			Terrain * pTerrain = (Terrain*)mEntity;
			int nMapSize = pTerrain->GetDesc().LMapSize - Terrain::kLMapBorder * 2;

			float blockSize = pTerrain->GetDesc().Dimension.x / pTerrain->GetDesc().BlockCount.x;

			int xblock = mIndex % pTerrain->GetDesc().BlockCount.x;
			int yblock = mIndex / pTerrain->GetDesc().BlockCount.y;

			Aabb bound;
			bound.minimum.x = xblock * blockSize;
			bound.minimum.y = 0;
			bound.minimum.z = yblock * blockSize;
			bound.maximum.x = xblock * blockSize + blockSize;
			bound.maximum.y = 10000;
			bound.maximum.z = yblock * blockSize + blockSize;

			std::vector<Light *> lights;
			pTerrain->GetLightList(lights, xblock, yblock, true);
			if (lights.size() > 0) {
				pTerrain->CalcuIndirectLighting(xblock, yblock, lights);
			}
		}
	}

	void STBaker::_calcuAmbientOcclusionMesh()
	{
		if (World::Instance()->GetSetting()->AOLevel > 0) {
			Mesh * pMesh = World::Instance()->GetMesh(mIndex);

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

	void STBaker::_postProcess()
	{
		if (mEntity->GetType() == LFX_MESH)
		{
			Mesh * pMesh = (Mesh*)mEntity;
		}
		else
		{
			Terrain * pTerrain = (Terrain*)mEntity;
			int xblock = mIndex % pTerrain->GetDesc().BlockCount.x;
			int yblock = mIndex / pTerrain->GetDesc().BlockCount.y;

			pTerrain->PostProcess(xblock, yblock);
		}
	}

}

