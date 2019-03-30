#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <set>
#include <websocketpp/common/thread.hpp>

#include "LFX_World.h"
#include "LFX_Stream.h"

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

/* on_open insert connection_hdl into channel
 * on_close remove connection_hdl from channel
 * on_message queue send to all channels
 */

enum action_type {
	SUBSCRIBE,
	UNSUBSCRIBE,
	MESSAGE
};

struct action {
	action(action_type t, connection_hdl h) : type(t), hdl(h) {}
	action(action_type t, connection_hdl h, server::message_ptr m)
		: type(t), hdl(h), msg(m) {}

	action_type type;
	websocketpp::connection_hdl hdl;
	server::message_ptr msg;
};

//
bool GQuit = false;
int GProgress = 0;
LFX::World* GWorld = NULL;

enum {
	PKI_NONE,

	PKI_START,
	PKI_STOP,

	PKO_LOG = 777,
	PKO_PROGRESS = 100,
};

struct OPacket
{
	virtual void* Data() = 0;
	virtual short Length() = 0;
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
class application
{
public:
	application() {
		// Initialize Asio Transport
		m_server.init_asio();

		// Register handler callbacks
		m_server.set_open_handler(bind(&application::on_open, this, ::_1));
		m_server.set_close_handler(bind(&application::on_close, this, ::_1));
		m_server.set_message_handler(bind(&application::on_message, this, ::_1, ::_2));
	}

	void run(uint16_t port) {
		// listen on specified port
		m_server.listen(port);

		// Start the server accept loop
		m_server.start_accept();

		// Start the ASIO io_service run loop
		try {
			m_server.run();
		}
		catch (const std::exception & e) {
			std::cout << e.what() << std::endl;
		}
	}

	void on_open(connection_hdl hdl) {
		{
			lock_guard<mutex> guard(m_action_lock);
			//std::cout << "on_open" << std::endl;
			m_actions.push(action(SUBSCRIBE, hdl));
		}
		//m_action_cond.notify_one();
	}

	void on_close(connection_hdl hdl) {
		{
			lock_guard<mutex> guard(m_action_lock);
			//std::cout << "on_close" << std::endl;
			m_actions.push(action(UNSUBSCRIBE, hdl));
		}
		//m_action_cond.notify_one();
	}

	void on_message(connection_hdl hdl, server::message_ptr msg) {
		// queue message up for sending by processing thread
		{
			lock_guard<mutex> guard(m_action_lock);
			//std::cout << "on_message" << std::endl;
			m_actions.push(action(MESSAGE, hdl, msg));
		}
		//m_action_cond.notify_one();
	}

	void process_messages() {
		while (!GQuit) {
			do {
				if (GWorld == NULL) {
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}

				lock_guard<mutex> guard(m_action_lock);
				if (!m_actions.empty()) {
					break;
				}

				if (GWorld != NULL) {
					update_world();
				}
			} while (1);

			unique_lock<mutex> lock(m_action_lock);
			action a = m_actions.front();
			m_actions.pop();
			lock.unlock();

			if (a.type == SUBSCRIBE) {
				lock_guard<mutex> guard(m_connection_lock);
				m_connections.insert(a.hdl);
			}
			else if (a.type == UNSUBSCRIBE) {
				lock_guard<mutex> guard(m_connection_lock);
				m_connections.erase(a.hdl);
			}
			else if (a.type == MESSAGE) {
				lock_guard<mutex> guard(m_connection_lock);

				if (a.msg->get_opcode() == websocketpp::frame::opcode::binary) {
					char* data = new char[a.msg->get_raw_payload().length() + 1];
					memcpy(data, a.msg->get_raw_payload().c_str(), a.msg->get_raw_payload().length());

					LFX::MemoryStream stream((BYTE*)data, (int)a.msg->get_raw_payload().length(), true);

					int pkId = 0;
					if (stream.Read(&pkId, sizeof(pkId)) == 4) {
						do_packet(a.hdl, pkId, stream);
					}
				}

#if 0
				con_list::iterator it;
				for (it = m_connections.begin(); it != m_connections.end(); ++it) {
					m_server.send(*it, a.msg);
				}
#endif
			}
			else {
				// undefined.
			}
		}

		m_server.stop();
	}

	void update_world()
	{
		int stage = LFX::World::Instance()->UpdateStage();
		float kp = (LFX::World::Instance()->GetProgress() + 1) / (float)(LFX::World::Instance()->GetEntityCount() + 1);
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

			printf("%s: %d%%\n", tag, progress);

			GProgress = progress;

			if (m_connections.size() > 0) {
				PKProgress opk;
				opk.data.Stage = stage;
				opk.data.Progress = GProgress;
				send_packet(*m_connections.begin(), PKProgress::PID, &opk);
			}
			
		}

		if (stage == LFX::World::STAGE_END) {
			GWorld->Save();
			GWorld->Clear();
			delete GWorld;
			GWorld = NULL;

			if (m_connections.size() > 0) {
				PKProgress opk;
				opk.data.Stage = LFX::World::STAGE_END;
				opk.data.Progress = 100;
				send_packet(*m_connections.begin(), PKProgress::PID, &opk);
			}
		}
	}

	void send_packet(websocketpp::connection_hdl cl, int pid, OPacket* pk)
	{
		char buff[2048];
		memcpy(buff, &pid, sizeof(int));
		memcpy(buff + 4, pk->Data(), pk->Length());

		m_server.send(cl, buff, 4 + pk->Length(), websocketpp::frame::opcode::value::binary);
	}

	void do_packet(websocketpp::connection_hdl cl, int id, LFX::MemoryStream& data)
	{
	#define ON_MESSAGE(pid, func) \
		if (pid == id) { \
			func(cl, data);\
			return ;\
		}

		ON_MESSAGE(PKI_START, on_start);
		ON_MESSAGE(PKI_STOP, on_stop);
	}

	void on_start(websocketpp::connection_hdl cl, LFX::MemoryStream& data)
	{
		GProgress = 0;
		SAFE_DELETE(GWorld);

		GWorld = new LFX::World;
		GWorld->Load();
		GWorld->Build();

		PKProgress opk;
		opk.data.Stage = LFX::World::STAGE_START;
		opk.data.Progress = 0;
		send_packet(cl, PKProgress::PID, &opk);

		GWorld->Start();
	}

	void on_stop(websocketpp::connection_hdl cl, LFX::MemoryStream& data)
	{
		SAFE_DELETE(GWorld);
		GQuit = true;
		m_server.stop_listening();
	}

private:
	typedef std::set<connection_hdl, std::owner_less<connection_hdl> > con_list;

	server m_server;
	con_list m_connections;
	std::queue<action> m_actions;

	mutex m_action_lock;
	mutex m_connection_lock;
	//condition_variable m_action_cond;
};

int main() {
	std::cout << "LightFX Server..." << std::endl;

	try {
		application app;

		// Start a thread to run the processing loop
		thread t(bind(&application::process_messages, &app));

		// Run the asio loop with the main thread
		app.run(9002);

		t.join();
	}
	catch (websocketpp::exception const & e) {
		std::cout << e.what() << std::endl;
	}
}