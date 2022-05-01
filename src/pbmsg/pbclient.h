#ifndef __PBCLIENT_H__
#define __PBCLIENT_H__
#include "../service/msgsession.h"
#include "../sp.h"
#include "pbrpc.h"
#include "pbdispatch.h"

namespace ashan
{
	/*
	 *	优先使用 rpc 方式
	 */
	struct pbclient : public msgsession, public sp<pbclient>
	{
		pbclient(const std::string &_url) : msgsession(_url)
		{
			static auto __oninit = heartbeat_init<msgsession, PBMSG>();
			msgsession::dispatch = pbdispatch::client;
		}
	};
	
	struct pbclientex : public msgsession, public sp<pbclientex>
	{
		pbclientex(const std::string &_url) : msgsession(_url)
		{
			static auto __oninit = heartbeat_init<msgsession, PBMSG>();
			msgsession::dispatch = pbdispatch::clientex;
		}
	};
}
#endif
