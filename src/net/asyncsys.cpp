#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <regex>
#include "../log.h"
#include "../sp.h"
#include "asyncsys.h"
#include "asyncnet.h"

namespace ashan
{
	std::string asyncsys::parsedns(const std::string &url)
	{
		struct addrinfo *_result = nullptr;
		struct addrinfo _info = {};
		_info.ai_family = AF_INET;
		_info.ai_socktype = SOCK_STREAM;
		_info.ai_flags = AI_PASSIVE;
		int code = getaddrinfo(url.c_str(), nullptr, &_info, &_result);
		if (code == 0)
		{
			for (struct addrinfo *ptr = _result; ptr != nullptr; ptr = ptr->ai_next)
			{
				struct sockaddr_in *__s = (struct sockaddr_in *)ptr->ai_addr;
				if (nullptr != __s)
				{
					std::string _addr;
					_addr.resize(100);
					inet_ntop(AF_INET, &__s->sin_addr.s_addr, (char *)_addr.data(), _addr.size());
					return _addr;
				}
			}
		}
		return "";
	}
	bool asyncsys::spliturl(const std::string &url, std::string &name, std::string &password, std::string &addr)
	{
		static std::regex e_url{R"((\S+):(\S+)@(\S+))"};
		static std::regex e_url2{R"((\S+)@(\S+))"};
		std::smatch sm;
		if (std::regex_search(url, sm, e_url))
		{
			name = sm[1];
			password = sm[2];
			addr = sm[3];
			return true;
		}
		if (std::regex_search(url, sm, e_url2))
		{
			name = sm[1];
			addr = sm[2];
			return true;
		}
		return false;
	}
	bool asyncsys::splitaddr(const std::string &addr, std::string &ip, int &port)
	{
		static std::regex e_port{R"(:(\d+))"};
		static std::regex e_ip_port{R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d+))"};
		static std::regex e_url{R"((\w+))"};
		static std::regex e_url_port{R"((\w+):(\d+))"};
		std::smatch sm;
		if (std::regex_search(addr, sm, e_ip_port))
		{
			ip = sm[1];
			port = std::stoi(sm[2]);
			return true;
		}
		if (std::regex_search(addr, sm, e_port))
		{
			ip = "127.0.0.1";
			port = std::stoi(sm[1]);
			return true;
		}
		if (std::regex_search(addr, sm, e_url))
		{
			auto url = sm[1];
			ip = parsedns(url);
			port = 80;
			return true;
		}
		if (std::regex_search(addr, sm, e_url_port))
		{
			auto url = sm[1];
			ip = parsedns(url);
			port = std::stoi(sm[2]);
			return true;
		}
		return false;
	}
	int asyncsys::start(const std::string &addr, asyncnet::fcb &&_cli, asyncnet::fcb &&_close)
	{
		std::string ip;
		int port;
		if (splitaddr(addr, ip, port))
		{
			auto fd = asyncnet::get().__bind(ip, port);
			LOG_DBG("start fd:%d %s", fd, addr.c_str());
			for (int i = 0; i < max_thread_count; ++i)
			{
				std::thread t(std::bind(&asyncsys::__start, this, fd, _cli, _close));
				t.detach();
				m_list.emplace_back(std::move(t));
			}
		}
		return INVALID;
	}
	int asyncsys::connect(const std::string &addr, asyncnet::fcb &&_cli, asyncnet::fcb &&_close)
	{
		std::string ip;
		int port;
		if (splitaddr(addr, ip, port))
		{
			LOG_DBG("connect %s", addr.c_str());
			int fd = asyncnet::get().connect(ip, port, std::move(_cli), std::move(_close));
			if (fd == INVALID)
			{
				LOG_DBG("connect failed %s", addr.c_str());
			}
			return fd;
		}
		return INVALID;
	}
	int asyncsys::__start(int fd, asyncnet::fcb _cli, asyncnet::fcb _close)
	{
		if (fd == INVALID)
		{
			exit(-1);
		}
		asyncnet::get().__start(fd, std::move(_cli), std::move(_close));
		asyncnet::get().wait();
		return 0;
	}
}