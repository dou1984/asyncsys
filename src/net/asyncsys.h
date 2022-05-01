#ifndef __ASYNCSYS_H__
#define __ASYNCSYS_H__
#include <list>
#include <thread>
#include <mutex>
#include "../sp.h"
#include "../singleton.h"
#include "asyncnet.h"

#define MAX_SERVER_THREAD (10)

namespace ashan
{
	class asyncsys final : public singleton<asyncsys>
	{
	public:
		int start(const std::string &addr, asyncnet::fcb &&_cli, asyncnet::fcb &&_close = nullptr);
		int connect(const std::string &addr, asyncnet::fcb &&_cli, asyncnet::fcb &&_close = nullptr);

		static std::string parsedns(const std::string &url);
		static bool splitaddr(const std::string &addr, std::string &ip, int &port);
		static bool spliturl(const std::string &url, std::string &name, std::string &password, std::string &addr);

	private:
		int __start(int fd, asyncnet::fcb _cli, asyncnet::fcb _close);

	private:
		std::list<std::thread> m_list;
		int max_thread_count = MAX_SERVER_THREAD;
	};
}
#endif
