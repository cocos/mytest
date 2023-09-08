#include <iostream>
#include <set>
#include <cstdarg>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "sio_client.h"

#include "LFX_Log.h"
#include "LFX_World.h"
#include "LFX_Renderer.h"
#include "LFX_CyclesRenderer.h"
#include "LFX_GLTFExp.h"

enum {
	E_STARTING = 1,
	E_BAKING,
	E_FINISHED,
	E_STOPPED,
};

bool GExpportGLTF = true;
std::atomic<int> GStatus(0);
int GProgress = 0;
LFX::Log* GLog = NULL;
LFX::World* GWorld = NULL;
LFX::IRenderer* GRenderer = NULL;

void StartEngine(bool render)
{
	GWorld = new LFX::World();
	if (GExpportGLTF) {
		GWorld->GetSetting()->LoadTexture = false;
	}

	if (GWorld->Load()) {
		if (GExpportGLTF) {
			LOGD("-: Export to gltf scene");
			if (!LFX::GLTFExp::Export()) {
				LOGE("-: Export gltf failed");
			}
			GStatus = E_STOPPED;
		}
		else {
			GRenderer = new LFX::CRenderer;
			GRenderer->Build();
			GRenderer->Start();
			GStatus = E_STARTING;
		}
	}
	else {
		GStatus = E_STOPPED;
		LOGE("?: Load scene failed");
	}
}

time_t GetTicks()
{
#ifdef _WIN32
	return GetTickCount() * 1000;
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}

const char* progress_format(const char* tag, int progress)
{
	static char text[256];

	sprintf(text, "%s %d%%\n", tag, progress);

	return text;
}

int remote_main(int argc, char* argv[])
{
	const char* url = "http://127.0.0.1:3000";
	
	GLog = new LFX::Log("lfx.log");

	LOGI("Version %d", LFX_VERSION);

	std::string commands;
	for (int i = 0; i < argc; ++i) {
		commands += argv[i];
		if (i == 1) {
			url = argv[i];
		}

		commands += " ";
	}
	LOGI("CmdLine: %s", commands.c_str());

	sio::client h;
	h.connect(url);
	h.socket()->emit("Login");

	h.socket()->on("Start", [](sio::event &) {
		if (GStatus != 0) {
			LOGE("?: Start faield, the world is started");
			return;
		}
		
		GProgress = 0;
		StartEngine(true);
	});

	h.socket()->on("Stop", [](sio::event &) {
		SAFE_DELETE(GRenderer);
		SAFE_DELETE(GWorld);
		GStatus = E_STOPPED;
	});

	h.socket()->on("disconnect", [](sio::event &) {
		LOGE("?: Disconnect");
		SAFE_DELETE(GRenderer);
		SAFE_DELETE(GWorld);
		GStatus = E_STOPPED;
	});
	
	// waiting for start
	while (GStatus == 0) {
		LFX::Thread::Sleep(1);
	}

	// start
	if (GStatus == E_STARTING && GRenderer != NULL) {
		const char* text = progress_format("Start", 0);
		LOGI(text);
		h.socket()->emit("Start", std::string(text));
		GStatus = E_BAKING;
	}

	// bake
	while (GStatus == E_BAKING && GRenderer != NULL) {
		float kp = (GRenderer->GetProgress() + 1) / (float)(GRenderer->GetTaskCount() + 1);
		int progress = (int)(kp * 100);

		if (GProgress != progress) {
			GProgress = progress;

			const char* text = progress_format("Build lighting", progress);
			LOGI(text);
			h.socket()->emit("Progress", std::string(text));
		}

#if 0
		static time_t last_tick = GetTickCount();
		time_t current_ticks = GetTicks();
		if ((current_ticks - last_tick) / 1000000 > 5) {
			h.socket()->emit("Tick");
		}
#endif

		GRenderer->Update();

		if (GRenderer->End()) {
			if (GProgress != 100) {
				const char* text = progress_format("Build lighting", 100);
				LOGI(text);
				h.socket()->emit("Progress", std::string(text));
			}

			LOGI("Delete renderer");
			SAFE_DELETE(GRenderer);

			LOGI("Save world...");
			GWorld->Save();
			LOGI("Save world end.");

			LOGI("Clear world...");
			GWorld->Clear();
			LOGI("Clear world end.");

			LOGI("Emit finished");
			h.socket()->emit("Finished");
			
			LOGI("Delete world");
			SAFE_DELETE(GWorld);
			GStatus = E_FINISHED;
		}
	};

	LOGI("Wait to stop");
	int time = 0;
	while (GStatus != E_STOPPED && time < 60/*seconds*/) {
		LFX::Thread::Sleep(5);
		LOGI("Wait stop...");
		time += 5;
	}

	if (GStatus != E_STOPPED) {
		LOGI("Wait stop time out...");
	}

	LOGI("Shutdown");
	SAFE_DELETE(GLog);

	return 0;
}

int local_main(int argc, char* argv[])
{
	GLog = new LFX::Log("lfx.log");

	StartEngine(true);

	while (GRenderer != NULL) {
		float kp = (GRenderer->GetProgress() + 1) / (float)(GRenderer->GetTaskCount() + 1);
		int progress = (int)(kp * 100);

		if (GProgress != progress) {
			GProgress = progress;

			const char* text = progress_format("Build lighting", progress);
			LOGI(text);
		}

		GRenderer->Update();

		if (GRenderer->End()) {
			if (GProgress != 100) {
				const char* text = progress_format("Build lighting", 100);
				LOGI(text);
			}

			LOGI("Delete renderer");
			SAFE_DELETE(GRenderer);

			LOGI("Save world...");
			GWorld->Save();
			LOGI("Save world end.");

			LOGI("Clear world...");
			GWorld->Clear();
			LOGI("Clear world end.");

			LOGI("Delete world");
			SAFE_DELETE(GWorld);
			GStatus = E_FINISHED;
		}
	}

	return 0;
}

#ifdef LFX_CYLCES_RENDERER
int cycles_main(int argc, char* argv[])
{
	GLog = new LFX::Log("lfx.log");

	StartEngine(false);
	if (GStatus == E_STARTING) {
		LFX::CylcesRenderer renderer;
		renderer.ExportScene();
	}

	return 0;
}
#endif

#define LFX_REMOTE_MODE 0
#define LFX_CYCLES_TEST 0

int main(int argc, char* argv[])
{
#if LFX_REMOTE_MODE
	return remote_main(argc, argv);
#elif LFX_CYCLES_TEST
	return cycles_main(argc, argv);
#else
	return local_main(argc, argv);
#endif
}