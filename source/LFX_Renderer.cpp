#include "LFX_Renderer.h"
#include "LFX_World.h"
#include "LFX_DeviceStats.h"

namespace LFX {

	CRenderer::CRenderer()
	{
	}

	CRenderer::~CRenderer()
	{
		for (auto i = 0; i < mThreads.size(); ++i) {
			mThreads[i]->Stop();
			delete mThreads[i];
		}
		mThreads.clear();
	}

	void CRenderer::Build()
	{
		World::Instance()->BuildScene();
	}

	void CRenderer::Start()
	{
		for (size_t i = 0; i < mThreads.size(); ++i) {
			delete[] mThreads[i];
		}
		mThreads.clear();

		mTaskIndex = 0;
		mProgress = 0;

		int threads = 1;
#ifdef _DEBUG
		threads = 1;
#elif LFX_MULTI_THREAD
		DeviceStats stats = DeviceStats::GetStats();
		threads = std::max(1, stats.Processors - 2);
		//mSetting.Threads = std::max(1, stats.Processors / 2);
#endif
		//mSetting.Threads = 1;
		for (int i = 0; i < threads; ++i) {
			mThreads.push_back(new STBaker(this, i));
		}

		const auto& meshes = World::Instance()->GetMeshes();
		const auto& terrains = World::Instance()->GetTerrains();
		const auto& probes = World::Instance()->GetSHProbes();

		mTasks.clear();
		if (World::Instance()->GetSetting()->BakeLightMap) {
			int numMeshTasks = 0;
			for (size_t i = 0; i < meshes.size(); ++i) {
				if (meshes[i]->GetLightingMapSize()) {
					++numMeshTasks;
					mTasks.push_back({ meshes[i], (int)i });
				}
			}
			LOGI("-: Mesh tasks %d", numMeshTasks);

			int numTerrainTasks = 0;
			for (auto* terrain : terrains) {
				for (int i = 0; i < terrain->GetDesc().BlockCount.x * terrain->GetDesc().BlockCount.y; ++i) {
					if (terrain->_getBlockValids()[i]) {
						++numTerrainTasks;
						mTasks.push_back({ terrain, i });
					}
				}
			}
			LOGI("-: Terrain tasks %d", numTerrainTasks);
		}

		if (World::Instance()->GetSetting()->BakeLightProbe) {
			for (size_t i = 0; i < probes.size(); ++i) {
				mTasks.push_back({ (SHProbe*)(&probes[i]), (int)i });
			}
			LOGI("-: Probe tasks %d", (int)probes.size());
		}

		for (size_t i = 0; i < mThreads.size(); ++i) {
			LOGI("-: Starting thread %d", i);
			mThreads[i]->Start();
		}
	}

	bool CRenderer::End()
	{
		return mThreads.empty();
	}

	void CRenderer::Update()
	{
		STBaker* thread = _getFreeThread();
		if (thread == nullptr) {
			return;
		}

		STBaker::Task task;
		if (_getNextTask(task)) {
			thread->Enqueue(task.entity, task.index);
			return;
		}

		// ensure all task finished
		for (size_t i = 0; i < mThreads.size(); ++i) {
			if (!mThreads[i]->IsCompeleted()) {
				return;
			}
		}

		// end
		for (size_t i = 0; i < mThreads.size(); ++i) {
			mThreads[i]->Stop();
			delete mThreads[i];
		}
		mThreads.clear();

		mTasks.clear();
		mTaskIndex = 0;
	}

	bool CRenderer::_getNextTask(STBaker::Task& task)
	{
		if (mTaskIndex < mTasks.size()) {
			task = mTasks[mTaskIndex++];
			return true;
		}

		return false;
	}

	STBaker* CRenderer::_getFreeThread()
	{
		for (size_t i = 0; i < mThreads.size(); ++i) {
			if (mThreads[i]->IsCompeleted()) {
				return mThreads[i];
			}
		}

		return nullptr;
	}

}