#ifndef __PBRPC_H__
#define __PBRPC_H__
#include <string>
#include <functional>
#include "../service/msgserver.h"
#include "../service/msgsession.h"
#include "../service/msgdispatch.h"
#include "serialize.h"

namespace ashan
{
	template <const char *_SRV, const char *_CMD, class _REQ, class _RES>
	struct pbrpc
	{		
		struct controllor;
		using METHOD = std::function<void(std::shared_ptr<controllor>, _REQ *, _RES *)>;
		using QUERY = std::function<void(int, MsgBase *, _RES *)>;	
		template <class _CLS, class... _ARGS>
		static std::string serialize(_CLS &cls, _ARGS &&..._args)
		{
			return __serialize(_SRV, _CMD, std::forward(_args)..., cls);
		}
		struct controllor : sp<controllor>
		{
			std::shared_ptr<session> m_cli;
			std::string m_session;
			u64 m_index = 0;
			int m_error = 0;
			_REQ m_req;
			_RES m_res;
			void set_error(int err) { m_error = err; }
			void set_session(const std::string &_session) { m_session = _session; }
			void set_index(u64 index) { m_index = index; }
			controllor(session *cli) : m_cli(cli->shared_from_this()) {}
			~controllor()
			{
				auto s = serialize(m_res, m_error, m_session);
				m_cli->send(std::move(s));
			}
			bool valid() const { return ::valid(m_cli); }
			auto get() { return m_cli; }
		};
		static void add_method(const std::shared_ptr<msgserver<std::string>> &srv, const METHOD &_callback)
		{
			srv->add_method(
				_CMD,
				[=](auto s, auto buf, auto size)
				{
					auto c = controllor::make(s);
					auto _msg = (MsgBase *)buf;
					_msg->data().UnpackTo(&c->m_req);
					c->set_session(_msg->session());
					c->set_index(_msg->index());
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
		template <class _SESS>
		static void query(_SESS &cli, const _REQ &_request, QUERY &&_callback)
		{
			auto c = cli->obj();
			query(c.get(), _request, std::move(_callback));
		}
		template <class _SESS, class... _ARGS>
		static void send(_SESS &cli, const _REQ &_res, _ARGS &&..._args)
		{
			auto c = cli->obj();
			c->send(serialize(_res, std::forward<_ARGS>(_args)...));
		}
		template <class _CLI, class... _ARGS>
		static void request(_CLI &cli, const _REQ &_res, _ARGS &&..._args)
		{
			if (cli)
			{
				cli->send(serialize(_res, std::forward<_ARGS>(_args)...));
			}
		}
		template <class _CLI, class... _ARGS>
		static void response(_CLI &cli, const _RES &_res, _ARGS &&..._args)
		{
			if (cli)
			{
				cli->send(serialize(_res, std::forward<_ARGS>(_args)...));
			}
		}
		static void onevent(QUERY &&_callback)
		{
			msgdispatch<std::string>::get().onevent(
				_CMD,
				[_callback = std::move(_callback)](auto r)
				{
					__oncallback(_callback, r);
				});
		}
		static void __oncallback(const QUERY &_callback, const msgsession::msg_results *r)
		{
			_RES _response;
			auto _msg = (MsgBase *)r->data;
			if (r->data != nullptr)
			{
				_msg->data().UnpackTo(&_response);
			}
			_callback(r->last_error, _msg, &_response);
		}
	};
}
#endif