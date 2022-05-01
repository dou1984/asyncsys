#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <regex>
#include "mysys.h"
#include "log.h"
#include "net/asyncnet.h"

namespace ashan
{
	void mysys::ignore_signal(int __signal)
	{
		struct sigaction sa;
		sa.sa_handler = SIG_IGN;
		sa.sa_flags = 0;
		if (sigemptyset(&sa.sa_mask) == -1 || sigaction(__signal, &sa, 0) == -1)
		{
			perror("failed to ignore SIGPIPE; sigaction");
			exit(EXIT_FAILURE);
		}
	}
	void mysys::ignore_signal()
	{
		ignore_signal(SIGPIPE);
	}
	void mysys::set_log_level(int _level)
	{
		::set_log_level(_level);
	}
	std::thread mysys::createthread(fcb &&_callback)
	{
		std::thread t(
			[_callback = std::move(_callback)]()
			{
				_callback();
				asyncnet::get().wait();
			});
		t.detach();
		return t;
	}
	void mysys::fork(fcb &&_main_cb, fcb &&_child_cb)
	{
		auto id = ::fork();
		if (id < 0)
		{
			LOG_ERR("fork error %d:%s", errno, strerror(errno));
		}
		else if (id == 0)
		{
			LOG_DBG("start child process %d", getpid());
			_child_cb();
		}
		else
		{
			LOG_DBG("start main process %d", getpid());
			_main_cb();
		}
	}
	void mysys::stop()
	{
		asyncnet::notify_all(
			[](auto _net, auto _fd)
			{ _net->stop(); });
	}
	void mysys::splite(const std::string &m, const std::string &s, const std::function<void(const std::string &)> &f)
	{
		std::regex re(s);
		std::sregex_token_iterator p(m.begin(), m.end(), re, -1);
		std::sregex_token_iterator end;
		while (p != end)
		{
			f(*p++);
		}
	}
}