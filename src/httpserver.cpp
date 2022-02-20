#include "asyncnet.h"
#include "asyncsys.h"
#include "httpserver.h"
#include "client.h"
#include "session.h"
#include "log.h"
#include "sp.h"

namespace ashan
{
	struct httpsession : sp<httpsession>
	{
		std::shared_ptr<session> m_session;
		std::shared_ptr<httpserver> m_server;
		http_parser m_parser;
		httpserver::request m_request;
		httpsession()
		{
			http_parser_init(&m_parser, HTTP_REQUEST);
			m_parser.data = this;
		}
	};
	int on_message_begin(http_parser *p)
	{
		LOG_DBG("%s", "***MESSAGE BEGIN***");
		httpsession *s = (httpsession *)(p->data);
		return 0;
	}
	int on_message_complete(http_parser *p)
	{
		LOG_DBG("%s", "***MESSAGE COMPLETE***");
		httpsession *s = (httpsession *)(p->data);
		assert(s->m_server != nullptr);
		s->m_server->__callback(s->shared_from_this());
		return 0;
	}
	int on_url(http_parser *p, const char *at, size_t length)
	{
		LOG_DBG("Url: %.*s", (int)length, at);
		httpsession *s = (httpsession *)(p->data);
		s->m_request.m_method.append(at, length);
		return 0;
	}
	int on_headers_complete(http_parser *p)
	{
		LOG_DBG("%s", "***HEADERS COMPLETE***");
		httpsession *s = (httpsession *)(p->data);
		return 0;
	}
	int on_header_field(http_parser *p, const char *at, size_t length)
	{
		LOG_DBG("Header field: %.*s", (int)length, at);
		httpsession *s = (httpsession *)(p->data);
		std::string t(at, length);
		s->m_request.t = std::move(t);
		return 0;
	}
	int on_header_value(http_parser *p, const char *at, size_t length)
	{
		LOG_DBG("Header value: %.*s", (int)length, at);
		httpsession *s = (httpsession *)(p->data);
		s->m_request.m_header.emplace(std::move(s->m_request.t), std::string(at, length));
		return 0;
	}
	int on_body(http_parser *p, const char *at, size_t length)
	{
		LOG_DBG("Body: %.*s", (int)length, at);
		httpsession *s = (httpsession *)(p->data);
		s->m_request.m_body.append(at, length);
		return 0;
	}
	http_parser_settings *http_parser_settings_init()
	{
		static http_parser_settings _;
		_.on_message_begin = on_message_begin;
		_.on_header_field = on_header_field;
		_.on_header_value = on_header_value;
		_.on_url = on_url;
		_.on_status = 0;
		_.on_body = on_body;
		_.on_headers_complete = on_headers_complete;
		_.on_message_complete = on_message_complete;
		return &_;
	}
	static http_parser_settings *settings_common = http_parser_settings_init();
	httpserver::httpserver(const std::string &url) : m_url(url)
	{
		__init();
	}
	void httpserver::__init()
	{
		asyncsys::get().start(
			m_url,
			[this](auto, auto fd)
			{
				assert(fd != INVALID);
				auto cli = client::make(fd)->init();
				auto s = httpsession::make();
				s->m_session = session::make()->set_client(cli);
				s->m_server = shared_from_this();
				cli->dispatch = [=](auto, auto buf, auto size)
				{
					auto parsed = http_parser_execute(&s->m_parser, settings_common, buf, size);
					if (s->m_parser.upgrade)
					{
						LOG_DBG("http_parser_execute upgrade %d:%ld", size, parsed);
					}
					else if (parsed != size)
					{
						LOG_ERR("http_parser_execute parsed %d:%ld", size, parsed);
					}
					return size;
				};
			});
	}
	void httpserver::set(const std::string &method, httpcb &&callback)
	{
		m_router.emplace(method, std::move(callback));
	}
	void httpserver::__callback(std::shared_ptr<httpsession> s)
	{
		static httpcb defhttpcb = [](auto s, auto &_request)
		{
			ashan::httpreponse rep;
			rep.set_error(404);
			auto _lst = rep.serialize();
			for (auto &it : _lst)
			{
				s->send(std::move(it));
			}
		};
		if (auto it = m_router.find(s->m_request.m_method); it != m_router.end())
		{
			it->second(s->m_session, s->m_request);
		}
		else
		{
			defhttpcb(s->m_session, s->m_request);
		}
	}
}