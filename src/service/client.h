#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <functional>
#include "../ioBuffer.h"
#include "../invalid.h"
#include "../sp.h"

namespace ashan
{
	class client final : public sp<client>
	{
		ioBufferEx<char> m_iobuf;
		int m_fd = INVALID;
		int m_thdid = INVALID;

	public:
		std::function<int(client *, const char *, int)> dispatch;

		client(int _fd);
		std::shared_ptr<client> init();
		int send(const char *buf, int size) const;
		int send(std::string &&msg) const;
		int __realsend(std::string &&msg);
		bool valid() const { return m_fd != INVALID; }
		int thdid() const { return m_thdid; }
		int recv();
		void clear();		
	};
}

#endif