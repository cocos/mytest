#pragma once

#include "LFX_Types.h"
#include "LFX_Baker.h"
#include "LFX_Scene.h"
#include "LFX_Light.h"
#include "LFX_Mesh.h"
#include "LFX_Terrain.h"
#include "LFX_Shader.h"
#include "LFX_SHBaker.h"

namespace LFX {

	class IRenderer
	{
	public:
		virtual ~IRenderer() {}

		virtual void Build() = 0;
		virtual void Start() = 0;
		virtual void Update() = 0;
		virtual bool End() = 0;
		virtual int GetProgress() = 0;
		virtual int GetTaskCount() = 0;
	};

	class CRenderer : public IRenderer
	{
	public:
		CRenderer();
		~CRenderer();

		void Build() override;
		void Start() override;
		bool End() override;
		void Update() override;
		int GetProgress() override { return mProgress; }
		int GetTaskCount() override { return mTasks.size(); }

		STBaker* _getThread(int i) { return mThreads[i]; }
		void _onThreadCompeleted() { mProgress += 1; }

	protected:
		bool _getNextTask(STBaker::Task& task);
		STBaker* _getFreeThread();

	public:
		std::vector<STBaker::Task> mTasks;
		int mTaskIndex;
		int mProgress;
		std::vector<STBaker*> mThreads;
	};

}