#include <iostream>
#include <set>
#include <cstdarg>

#include "sio_client.h"

#include "LFX_Log.h"
#include "LFX_World.h"
#include "LFX_Stream.h"

bool GQuit = false;
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
	GLog = new LFX::Log("lfx.log");

	sio::client h;
	h.connect("http://127.0.0.1:3000");
	h.socket()->emit("Login");

	h.socket()->on("Start", [](sio::event &) {
		GProgress = 0;
		SAFE_DELETE(GWorld);

		GWorld = new LFX::World;
		if (GWorld->Load()) {
			LOGE("?: Load scene failed");
		}

		GWorld->Build();

		GWorld->Start();
	});

	h.socket()->on("Stop", [](sio::event &) {
		SAFE_DELETE(GWorld);
		GQuit = true;
	});

	h.socket()->on("disconnect", [](sio::event &) {
		SAFE_DELETE(GWorld);
		GQuit = true;
	});
	
	// waiting for start
	while (!GQuit && GWorld == NULL) {
		LFX::Thread::Sleep(1);
	}

	// start
	if (!GQuit && GWorld != NULL) {
		const char* text = progress_format("Start", 0);
		LOGI(text);
		h.socket()->emit("Start", std::string(text));
	}

	// bake
	while (!GQuit && GWorld != NULL) {
		int stage = GWorld->UpdateStage();
		float kp = (GWorld->GetProgress() + 1) / (float)(GWorld->GetEntityCount() + 1);
		int progress = (int)(kp * 100);

		if (stage != LFX::World::STAGE_END && GProgress != progress) {
			const char * tag = "";
			switch (stage) {
			case LFX::World::STAGE_DIRECT_LIGHTING:
				tag = "DirectLighting";
				break;

			case LFX::World::STAGE_INDIRECT_LIGHTING:
				tag = "IndirectLighting";
				break;

			case LFX::World::STAGE_POST_PROCESS:
				tag = "Post Process";
				break;
			}

			GProgress = progress;

			const char* text = progress_format(tag, progress);
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

		if (stage == LFX::World::STAGE_END) {
			GWorld->Save();
			GWorld->Clear();

			h.socket()->emit("Finished");

			SAFE_DELETE(GWorld);
			LFX::Thread::Sleep(5);
		}
	};

	int time = 0;
	while (!GQuit && time < 30) {
		LFX::Thread::Sleep(5);
		time += 5;
	}

	SAFE_DELETE(GLog);

	return 0;
}