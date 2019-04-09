#include "LFX_World.h"
#include "LFX_Stream.h"


bool mQuit = false;
int mProgress = 0;
LFX::World* mWorld = NULL;

#define PACK_MAX_SIZE 2048

enum {
	PKI_NONE,

	PKI_START,
	PKI_STOP,

	PKO_LOG = 777,
	PKO_PROGRESS = 100,
};

struct PKHeader
{
	short id;
	short length;
};

#if 0
struct OPacket
{
	virtual void* Data() = 0;
	virtual short Length() = 0;

	static void Send(int pid, OPacket* pk)
	{
		PKHeader head;
		head.id = pid;
		head.length = pk->Length();

#if 0
		cl->OStream()->Write(&head, sizeof(head));
		cl->OStream()->Write(pk->Data(), head.length);
#endif
	}
};

struct PKProgress : public OPacket
{
	static const int PID = PKO_PROGRESS;

	struct {
		int Stage;
		int Progress;
	} data;

	virtual short Length()
	{
		return (short)sizeof(data);
	}

	virtual void* Data()
	{
		return &data;
	}
};

struct PKLog : public OPacket
{
	static const int PID = PKO_LOG;

	enum {
		C_INFO,
		C_DEBUG,
		C_WARN,
		C_ERROR,
	};

	struct {
		int Channel;
		int Length;
		char Message[1024];
	} data;

	void log(int channel, const char* format, ...)
	{
		data.Channel = channel;

		va_list arglist;
		va_start(arglist, format);
		data.Length = vsprintf(data.Message, format, arglist);
		va_end(arglist);
	}

	virtual short Length()
	{
		return 4 + data.Length;
	}

	virtual void* Data()
	{
		return &data;
	}
};

//
struct PKStart
{
	static const int PID = PKI_START;

	void operator()(LFX::TcpClient* cl, LFX::MemoryStream& data)
	{
		mProgress = 0;

		SAFE_DELETE(mWorld);

		mWorld = new LFX::World;
		mWorld->Load();
		mWorld->Build();

		PKProgress opk;
		opk.data.Stage = LFX::World::STAGE_START;
		opk.data.Progress = 0;
		OPacket::Send(cl, PKProgress::PID, &opk);

		mWorld->Start();
	}
};

struct PKStop
{
	static const int PID = PKI_STOP;

	void operator()(LFX::TcpClient* cl, LFX::MemoryStream& data)
	{
		SAFE_DELETE(mWorld);
	}
};

static void DoPacket(LFX::TcpClient* cl, int pid, LFX::MemoryStream& data)
{
#define ON_MESSAGE(classname) \
	if (classname::PID == pid) { \
		classname ipk; \
		ipk(cl, data);\
		return ;\
	}

	ON_MESSAGE(PKStart);
	ON_MESSAGE(PKStop);
}
#endif

#if 0
int main()
{
	LFX::Socket::Init();
	LFX::TcpServer gServer;

	// start server
	printf("LightFX Server starting...\n");

	if (!gServer.Bind(8188)) {
		printf("LightFX Server bind failed...\n");
		gServer.Close();
		LFX::Socket::Shutdown();
		return 1;
	}

	//
	printf("LightFX Server running...\n");

	while (!mQuit) {
		gServer.Select(0, 50);

		// progress packet
		for (int i = 0; i < gServer.GetClientCount(); ++i) {
			LFX::TcpClient* cl = gServer.GetClient(i);

			while (cl->Valid()) {
				int buffSize = cl->IStream()->Size();
				if (buffSize == 0)
					break;

				// skip compact
				char compact[PACK_COMPACT_SIZE + 1];
				if (cl->IStream()->Read(compact, PACK_COMPACT_SIZE) == 0)
					break;

				compact[PACK_COMPACT_SIZE] = 0;
				if (strcmp(compact, PACK_COMPACT) != 0) {
					LOGE("compact error");
					cl->Close(0);
					break;
				}

				int skip = PACK_COMPACT_SIZE;

				// read packet head
				PKHeader head;
				if (cl->IStream()->Read(&head, sizeof(head)) == 0) {
					cl->IStream()->Skip(-skip);
					break;
				}

				skip += sizeof(head);

				if (head.length <= 0 || head.length > PACK_MAX_SIZE) {
					LOGE("packet size invalid, id %d, size %d", head.id, head.length);
					cl->Close(0);
					break;
				}

				// read data
				uint8_t buffer[PACK_MAX_SIZE];
				if (cl->IStream()->Read(buffer, head.length) == 0) {
					cl->IStream()->Skip(-skip);
					break;
				}

				LOGD("Do packet %d", head.id);
				DoPacket(cl, head.id, LFX::MemoryStream(buffer, head.length, false));
			}
		}

		if (gServer.GetClientCount() == 0) {
			SAFE_DELETE(mWorld);
			LFX::Thread::Sleep(1.0f);
			continue;
		}

		//
		static int s_tick = 0;
		if (s_tick++ == 1000) {
			PKLog opk;
			opk.log(0, "123456");
			OPacket::Send(gServer.GetClient(gServer.GetClientCount() - 1), PKProgress::PID, &opk);
			s_tick = 0;
		}

		// update world
		if (mWorld != NULL) {
			int stage = LFX::World::Instance()->UpdateStage();
			float kp = (LFX::World::Instance()->GetProgress() + 1) / (float)(LFX::World::Instance()->GetEntityCount() + 1);
			int progress = (int)(kp * 100);

			if (stage != LFX::World::STAGE_END && mProgress != progress) {
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

				LOGD("%s: %d%%", tag, progress);

				mProgress = progress;

				PKProgress opk;
				opk.data.Stage = stage;
				opk.data.Progress = mProgress;
				OPacket::Send(gServer.GetClient(0), PKProgress::PID, &opk);
			}

			if (stage == LFX::World::STAGE_END) {
				mWorld->Save();
				mWorld->Clear();
				delete mWorld;
				mWorld = NULL;

				PKProgress opk;
				opk.data.Stage = LFX::World::STAGE_END;
				opk.data.Progress = 100;
				OPacket::Send(gServer.GetClient(0), PKProgress::PID, &opk);
			}
		}
	}

	SAFE_DELETE(mWorld);

	//
	printf("LightFX Server shutdown...\n");

	gServer.Close();
	LFX::Socket::Shutdown();

	return 0;
}
#endif

#if 0

int main()
{
	LFX::Server sv;
	sv.Run();

	while (sv.GetStatus() != LFX::Thread::STOP) {
		// handle messages
		LFX::Server::Mutex.Lock();

		for (int i = 0; i < LFX::Server::MessageQueue.size(); ++i) {
			LFX::MemoryStream* stream = LFX::Server::MessageQueue[i];

			PKHeader head;
			if (stream->Read(&head, sizeof(head)) != sizeof(head)) {
				continue;
			}

			uint8_t buffer[PACK_MAX_SIZE];
			if (stream->Read(buffer, head.length) == 0) {
				continue;
			}

			//DoPacket(cl, head.id, LFX::MemoryStream(buffer, head.length, false));
		}

		for (int i = 0; i < LFX::Server::MessageQueue.size(); ++i) {
			delete LFX::Server::MessageQueue[i];
		}
		LFX::Server::MessageQueue.clear();

		LFX::Server::Mutex.Unlock();
	}

	for (int i = 0; i < LFX::Server::MessageQueue.size(); ++i) {
		delete LFX::Server::MessageQueue[i];
	}

	return 0;
}

#endif
