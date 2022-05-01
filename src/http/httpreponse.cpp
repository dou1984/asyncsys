#include <sstream>
#include <unordered_map>
#include "../joint.h"
#include "httpreponse.h"

namespace ashan
{
	static std::unordered_map<int, std::string> g_strerror;
	static int g_init = []() -> int
	{
		g_strerror.emplace(200, "200 OK");
		g_strerror.emplace(400, "400 Bad Request");
		g_strerror.emplace(403, "403 Frobidden");
		g_strerror.emplace(404, "404 Not Found");
		g_strerror.emplace(413, "413 Request Entity Too Large");
		g_strerror.emplace(417, "417 Expectation Failed");
		g_strerror.emplace(500, "500 Server Error");
		return g_strerror.size();
	}();
	const char *http_strerror(int err)
	{
		if (auto it = g_strerror.find(err); it != g_strerror.end())
		{
			return it->second.c_str();
		}
		static char deferr[] = "505 Not Supported";
		return deferr;
	}

	void httpreponse::set_error(int err)
	{
		m_error = err;
	}
	void httpreponse::set_header(std::string &&h, std::string &&v)
	{
		m_header.emplace(std::move(h), std::move(v));
	}
	void httpreponse::set_body(std::string &&_body)
	{
		m_body = std::move(_body);
	}
	void httpreponse::append(const char *buf, int size)
	{
		m_body.append(buf, size);
	}
	std::list<std::string> httpreponse::serialize()
	{
		std::list<std::string> lst;
		int body_size = 0;
		{
			ostring oss;
			oss.joint("<html>\r\n",
					  "<head>\r\n",
					  R"(<meta http-equiv="Content-Type" context="text/html;charset=UTF-8">)",
					  "\r\n");
			for (auto it : m_header)
			{
				oss.joint("<meta ", it.first, "=\"", it.second, "\">\r\n");
			}
			oss.joint("</head>\r\n");
			if (!m_body.empty())
			{
				oss.joint("<body>", m_body, "</body>\r\n");
			}
			oss.joint("</html>\r\n");
			auto body = oss.data();
			body_size = body.size();
			lst.emplace_back(std::move(body));
		}
		ostring oss;
		lst.emplace_front(
			oss.joint("HTTP/1.1 ", http_strerror(m_error), "\r\n",
					  "Server: asyncsys/1.0.0\r\n",
					  "Content-Type: text/html\r\n",
					  "Content-Length: ", body_size, "\r\n",
					  "Connection: keep-alive\r\n",
					  "\r\n"));
		return lst;
	}
}