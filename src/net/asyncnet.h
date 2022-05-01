/*
 *	Copyright (c) 2019-2022 Zhao YunShan(93850592@qq.com)
 *	All rights reserved.
 */
#ifndef __ASYNCNET_H__
#define __ASYNCNET_H__
#include <cassert>
#include <vector>
#include <string>
#include <list>
#include <mutex>
#include <bitset>
#include <functional>
#include <unistd.h>
#include "../invalid.h"
#include "../thdlocal.h"

namespace ashan
{
#define MAX_THREAD_COUNT (0x10000)
#define MAX_RETRY_TIMEOUT (15 * 1000)
	class asyncnet final : public thdlocal<asyncnet>
	{
	public:
		enum STATE
		{
			e_state_invalid,
			e_state_accept,
			e_state_connect,
			e_state_connect_ex,
			e_state_client,
			e_state_client_ex,
			e_state_event,
			e_state_timer,
			e_state_rtimer,
		};
		enum FLAGS
		{
			e_flag_invalid,
			e_flag_close,
			e_flag_end,
		};
		using fcb = std::function<void(asyncnet *, int)>;
		using wbuffer = std::list<std::string>;

		asyncnet();
		virtual ~asyncnet();

		int wait();
		int get_epfd() const;
		int stop();
		static asyncnet *at(int _epfd);

	public:
		int start(const std::string &ip, int port, fcb &&_accept, fcb &&_close);
		int connect(const std::string &ip, int port, fcb &&_connect, fcb &&_close);
		int set_client(int fd, fcb &&_read, fcb &&_write = nullptr);
		int set_clientclose(int fd, fcb &&_close);
		int send(int fd, std::string &&_msg);
		int set_timer(int64_t itime, int64_t rtime, fcb &&_timer);
		int stop_timer(int fd);
		int64_t remaining_timer(int fd);
		int notify(fcb &&_event);
		static int notify_all(const fcb &_event);

	public:
		friend class asyncsys;
		friend class rediscli;
		friend class mysqlcli;

	private:
		int m_epfd = INVALID;
		int m_running = INVALID;
		std::vector<fcb> m_client;
		std::vector<fcb> m_close;
		std::vector<fcb> m_read;
		std::vector<fcb> m_write;
		std::vector<wbuffer> m_wbuffer;
		std::bitset<e_flag_end> m_flags;

		std::mutex m_mutex;
		std::list<fcb> m_trigger;
		int m_eventfd;
		static asyncnet *m_thdset[MAX_THREAD_COUNT];

	private:
		int __fd_add_event(int fd, int state, fcb &&_cli, fcb &&_close);
		int __start(int fd, fcb &&_cli, fcb &&_close);
		int __bind(const std::string &ip, int port);
		int __bind(int fd, const std::string &ip, int port);
		int __connect(int fd, const std::string &ip, int port);
		int __settimer(int fd, int64_t itime, int64_t rtime, fcb &&_timer);
		int __onconnect(int fd, int __events);
		int __onaccept(int fd, int __events);
		int __ontimer(int fd, int __events, int rtimer);
		int __onclient(int fd, int __events);
		int __onevent(int fd, int __events);
		uint64_t __merge(int fd, int state);
		int __wait();
		int __send(int fd);
		int __call(std::vector<fcb> &_vec, int, int);
		int __call(std::vector<fcb> &, int);
		int __clientclose(int fd);
		int __clientclosed_already(int fd);
		int __clear(std::vector<fcb> &_vec, int fd);

	private:
		int getsockerr(int __fd);
		void set_noblock(int __fd);
		void set_reuseaddr(int __fd);
		void add_event(int __fd, int __state, int __events);
		void mod_event(int __fd, int __state, int __events);
		void del_event(int __fd);
		void __emplace_back(std::vector<fcb> &vec, int fd, fcb &&obj);
		void __push_back(std::vector<fcb> &vec, int fd, const fcb &obj);
	};
}
#endif
