#ifndef __RPCSERVER_H__
#define __RPCSERVER_H__
#include <memory>
#include <string>
#include <unordered_map>
#include "client.h"
#include "session.h"
#include "sp.h"

namespace ashan
{
	class msgserver : public sp<msgserver>
	{
		using rpccb = std::function<void(std::shared_ptr<session>, const char *, int)>;
		std::unordered_map<int, rpccb> m_router;
		std::string m_url;

	public:
		msgserver(const std::string &url);
		void add_method(int, rpccb &&);		
	};
}

#endif