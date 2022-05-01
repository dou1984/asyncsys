#ifndef __RPC_H__
#define __RPC_H__
#include "../invalid.h"
#include "../sp.h"
#include "../mytype.h"
#include "../msg/msg.h"
#include "service/msgserver.h"
#include "service/msgsession.h"
#include "service/msgdispatch.h"

namespace ashan
{
	template <u32 _CMD, class _REQ, class _RES>
	struct rpc
	{
		struct controllor;
		using METHOD = std::function<void(std::shared_ptr<controllor>, _REQ *, _RES *)>;
		using QUERY = std::function<void(int, _RES *)>;
		template <class _CLS>
		static std::string serialize(const _CLS &cls, int error)
		{
			auto byteSize = cls.ByteSize();
			std::string t;
			t.resize(sizeof(MSG) + byteSize);
			MSG *_msg = (MSG *)t.data();
			_msg->len = byteSize;
			_msg->cmd = _CMD;
			_msg->error = error;
			if (byteSize != 0)
			{
				cls.SerializeToArray(_msg->data(), _msg->size());
			}
			return t;
		}
		struct controllor : sp<controllor>
		{
			std::shared_ptr<session> m_cli;
			int m_error = 0;
			_REQ m_req;
			_RES m_res;
			void set_error(int err) { m_error = err; }
			controllor(session *cli) : m_cli(cli->shared_from_this())
			{
			}
			~controllor()
			{
				m_cli->send(serialize(m_res, m_error));
			}
			bool valid() const { return ::valid(m_cli); }
			auto get() { return m_cli; }
		};
		static void add_method(const std::shared_ptr<msgserver<int>> &srv, const METHOD &_callback)
		{
			srv->add_method(
				_CMD,
				[=](auto s, auto buf, auto size)
				{
					auto c = controllor::make(s);
					c->m_req.ParseFromArray(buf, size);
					_callback(c, &c->m_req, &c->m_res);
				});
		}
		static void query(const std::shared_ptr<msgsession> &cli, const _REQ &_request, QUERY &&_callback)
		{
			cli->query(
				serialize(_request, 0),
				[_callback = std::move(_callback)](auto r)
				{
					__oncallback(_callback, r);
				});
		}
		template <class _CLI>
		static void request(_CLI cli, int _error, const _REQ &_res)
		{
			if (cli)
			{
				cli->send(serialize(_res, _error));
			}
		}
		template <class _CLI>
		static void response(_CLI cli, int _error, const _RES &_res)
		{
			if (cli)
			{
				cli->send(serialize(_res, _error));
			}
		}
		static void onevent(QUERY &&_callback)
		{
			msgdispatch<int>::get().onevent(
				_CMD,
				[_callback = std::move(_callback)](auto r)
				{
					__oncallback(_callback, r);
				});
		}
		static void __oncallback(const QUERY &_callback, const msgsession::msg_results *r)
		{
			if (r->len > 0)
			{
				_RES _response;
				_response.ParseFromArray(r->data, r->len);
				_callback(r->last_error, &_response);
			}
			else
			{
				_callback(r->last_error, nullptr);
			}
		}
	};
}
#endif