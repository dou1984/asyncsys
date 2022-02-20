#ifndef __ASYNCSYS_H__
#define __ASYNCSYS_H__
#include <list>
#include <thread>
#include <mutex>
#include "asyncnet.h"
#include "sp.h"

namespace ashan
{
	class asyncsys final
	{
	public:
		int start(const std::string &addr, asyncnet::fcb &&_cli, asyncnet::fcb &&_close = nullptr);		
		int connect(const std::string &addr, asyncnet::fcb &&_cli, asyncnet::fcb &&_close = nullptr);

		static std::string parsedns(const std::string &url);
		static bool splitaddr(const std::string &addr, std::string &ip, int &port);

		static asyncsys& get();
	private:
		int __start(int fd, asyncnet::fcb _cli, asyncnet::fcb _close);

	private:
		std::list<std::thread> m_list;
		int max_thread_count = 10;
	};
}

#endif