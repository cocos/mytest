#include <iostream>
#include <set>
#include <cstdarg>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "sio_client.h"

#include "LFX_Log.h"
#include "LFX_World.h"
#include "LFX_Stream.h"

enum {
	E_STARTING = 1,
	E_BAKING,
	E_STOPPED,
};

std::atomic<int> GStatus(0);
int GProgress = 0;
LFX::Log* GLog = NULL;
LFX::World* GWorld = NULL;

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

int main(int argc, char* argv[])
{
	const char* url = "http://127.0.0.1:3000";
	
	GLog = new LFX::Log("lfx.log");

	std::string commands;
	for (int i = 0; i < argc; ++i) {
		commands += argv[i];
		if (i == 1) {
			url = argv[i];
		}

		commands += " ";
	}
	LOGI("%s", commands.c_str());

	sio::client h;
	h.connect(url);
	h.socket()->emit("Login");

	h.socket()->on("Start", [](sio::event &) {
		if (GStatus != 0) {
			LOGE("?: Start faield, the world is started");
			return;
		}
		
		GProgress = 0;

		GWorld = new LFX::World;
		if (GWorld->Load()) {
			GWorld->Build();
			GWorld->Start();
			GStatus = E_STARTING;
		}
		else {
			GStatus = E_STOPPED;
			LOGE("?: Load scene failed");
		}
	});

	h.socket()->on("Stop", [](sio::event &) {
		SAFE_DELETE(GWorld);
		GStatus = E_STOPPED;
	});

	h.socket()->on("disconnect", [](sio::event &) {
		SAFE_DELETE(GWorld);
		GStatus = E_STOPPED;
	});
	
	// waiting for start
	while (GStatus == 0) {
		LFX::Thread::Sleep(1);
	}

	// start
	if (GStatus == E_STARTING && GWorld != NULL) {
		const char* text = progress_format("Start", 0);
		LOGI(text);
		h.socket()->emit("Start", std::string(text));

		GStatus = E_BAKING;
	}

	// bake
	while (GStatus == E_BAKING && GWorld != NULL) {
		float kp = (GWorld->GetProgress() + 1) / (float)(GWorld->GetTaskCount() + 1);
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

		GWorld->UpdateTask();

		if (GWorld->End()) {
			GWorld->Save();
			GWorld->Clear();

			h.socket()->emit("Finished");

			SAFE_DELETE(GWorld);
			GStatus = E_STOPPED;
		}
	};

	int time = 0;
	while (GStatus != E_STOPPED && time < 30) {
		LFX::Thread::Sleep(5);
		time += 5;
	}

	SAFE_DELETE(GLog);

	return 0;
}