#include <iostream>
#include <set>
#include <cstdarg>

#include "sio_client.h"

#include "LFX_World.h"
#include "LFX_Stream.h"

bool GQuit = false;
int GProgress = 0;
LFX::World* GWorld = NULL;

const char* progress_format(const char* tag, int progress)
{
	static char text[256];

	sprintf(text, "%s %d%%\n", tag, progress);

	return text;
}

void on_start(sio::event &)
{
	GProgress = 0;
	SAFE_DELETE(GWorld);

	GWorld = new LFX::World;
	GWorld->Load();
	GWorld->Build();

	GWorld->Start();
}

void on_stop(sio::event &)
{
	SAFE_DELETE(GWorld);
	GQuit = true;
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

int main()
{
	sio::client h;
	h.connect("http://127.0.0.1:3000");
	h.socket()->emit("Login");

	h.socket()->on("Start", on_start);
	h.socket()->on("Stop", on_stop);
	h.socket()->on("disconnect", [](sio::event &e) {
		on_stop(e);
	});
	
	// waiting for start
	while (!GQuit && GWorld == NULL) {
		LFX::Thread::Sleep(1);
	}

	// start
	if (!GQuit && GWorld != NULL) {
		const char* text = progress_format("Start", 0);
		printf(text);
		h.socket()->emit("Start", std::string(text));
	}

	// bake
	time_t last_tick = GetTickCount();
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
			printf(text);
			h.socket()->emit("Progress", std::string(text));
		}

		time_t current_ticks = GetTicks();
		if ((current_ticks - last_tick) / 1000000 > 1) {
			h.socket()->emit("Tick");
		}

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

	return 0;
}