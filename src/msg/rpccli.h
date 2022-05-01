#ifndef __PRCCLI_H__
#define __PRCCLI_H__
#include <string.h>
#include "../sp.h"
#include "../msg/msg.h"
#include "../service/msgsession.h"
#include "mydispatch.h"

namespace ashan
{
	struct rpccli : public msgsession, public sp<rpccli>
	{
		rpccli(const std::string &url) : msgsession(url)
		{
			static auto __oninit = heartbeat_init<msgsession, MSG>();
			msgsession::dispatch = mydispatch<MSG>::client;
		}
	};
}
#endif