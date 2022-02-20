#ifndef __RPCFUNC_H__
#define __RPCFUNC_H__
#include "msgserver.h"
#include "msgsession.h"
#include "sp.h"

namespace ashan
{
	template <int _CMD, class _CLS>
	std::string msgassemble(const _CLS *cls, int error)
	{
		auto byteSize = cls->ByteSize();
		std::string t;
		t.resize(sizeof(MSG) + byteSize);
		MSG *_msg = (MSG *)t.data();
		_msg->len = byteSize;
		_msg->cmd = _CMD;
		_msg->error = error;
		if (byteSize != 0)
		{
			cls->SerializeToArray(_msg->data, _msg->len);
		}
		return t;
	}
	template <int _CMD, class _REQ, class _RES>
	struct rpc
	{
		struct controllor;
		using METHOD = std::function<void(std::shared_ptr<controllor>, _REQ *, _RES *)>;
		using QUERY = std::function<void(int, _RES *)>;
		struct controllor : sp<controllor>
		{
			std::shared_ptr<session> m_cli;
			int m_error = 0;
			_REQ m_req;
			_RES m_res;
			void set_error(int err) { m_error = err; }
			controllor(std::shared_ptr<session> cli) : m_cli(cli) {}
			~controllor()
			{
				m_cli->send(msgassemble<_CMD, _RES>(&m_res, m_error));
			}
		};
		static void add_method(std::shared_ptr<msgserver> srv, const METHOD &_callback)
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
		static void query(std::shared_ptr<msgsession> cli, const _REQ &_request, QUERY &&_callback)
		{
			cli->query(
				msgassemble<_CMD, _REQ>(&_request, 0),
				[_callback = std::move(_callback)](auto r)
				{
					if (r->last_error == 0)
					{
						_RES _response;
						_response.ParseFromArray(r->data, r->len);
						_callback(r->last_error, &_response);
					}
					else
					{
						_callback(r->last_error, nullptr);
					}
				});
		}
		static void request(std::shared_ptr<session> cli, int _error, const _RES &_res)
		{
			if (cli)
			{
				cli->send(msgassemble<_CMD, _RES>(&_res, _error));
			}
		}
	};
}
#endif