#include "LFX_Baker.h"
#include "LFX_World.h"

namespace LFX {

	LFX_Baker::LFX_Baker(int id)
	{
		mId = id;
		mEntity = NULL;
		mIndex = 0;
		mCompeleted = true;
	}

	LFX_Baker::~LFX_Baker()
	{
	}

	void LFX_Baker::Run()
	{
		while (1) {
			if (mStatus == STOP)
				break;

			if (mCompeleted)
				continue;

			if (mEntity == World::Instance()->GetTerrain()) {
				switch (mStage){
				case World::STAGE_DIRECT_LIGHTING:
					_calcuDirectLightingTerrain();
					break;

				case World::STAGE_INDIRECT_LIGHTING:
					_calcuIndirectLightingTerrain();
					break;

				case World::STAGE_AMBIENT_OCCLUSION:
					_calcuAmbientOcclusionTerrain();
					break;

				case World::STAGE_POST_PROCESS:
					_postProcess();
					break;
				}
			}
			else {
				switch (mStage) {
				case World::STAGE_DIRECT_LIGHTING:
					_calcuDirectLightingMesh();
					break;

				case World::STAGE_INDIRECT_LIGHTING:
					_calcuIndirectLightingMesh();
					break;

				case World::STAGE_AMBIENT_OCCLUSION:
					_calcuAmbientOcclusionMesh();
					break;

				case World::STAGE_POST_PROCESS:
					_postProcess();
					break;
				}
			}

			World::Instance()->_onThreadCompeleted();

			mCompeleted = true;
		}
	}

	void LFX_Baker::Enqueue(Entity * entity, int index, int stage)
	{
		mEntity = entity;
		mIndex = index;
		mStage = stage;

		mCompeleted = false;
	}

	void LFX_Baker::_calcuDirectLightingMesh()
	{
		Mesh * pMesh = World::Instance()->GetMesh(mIndex);
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

	void LFX_Baker::_calcuDirectLightingTerrain()
	{
		Terrain * pTerrain = World::Instance()->GetTerrain();

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
			World::Instance()->GetTerrain()->CalcuDirectLighting(xblock, yblock, lights);
		}
	}

	void LFX_Baker::_calcuIndirectLightingMesh()
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

	void LFX_Baker::_calcuIndirectLightingTerrain()
	{
		if (World::Instance()->GetSetting()->GIScale > 0) {
			Terrain * pTerrain = World::Instance()->GetTerrain();
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
				World::Instance()->GetTerrain()->CalcuIndirectLighting(xblock, yblock, lights);
			}
		}
	}

	void LFX_Baker::_calcuAmbientOcclusionMesh()
	{
		if (World::Instance()->GetSetting()->AOLevel > 0) {
			Mesh * pMesh = World::Instance()->GetMesh(mIndex);

			if (pMesh->GetLightingMapSize() > 0) {
				pMesh->CalcuAmbientOcclusion();
			}
		}
	}

	void LFX_Baker::_calcuAmbientOcclusionTerrain()
	{
		if (World::Instance()->GetSetting()->AOLevel > 0) {
			//...
		}
	}

	void LFX_Baker::_postProcess()
	{
		if (mEntity != World::Instance()->GetTerrain())
		{
			Mesh * pMesh = World::Instance()->GetMesh(mIndex);

			pMesh->PostProcess();
		}
		else
		{
			Terrain * pTerrain = World::Instance()->GetTerrain();
			int xblock = mIndex % pTerrain->GetDesc().BlockCount.x;
			int yblock = mIndex / pTerrain->GetDesc().BlockCount.y;

			pTerrain->PostProcess(xblock, yblock);
		}
	}

}

