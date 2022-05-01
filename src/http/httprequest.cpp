#include <list>
#include <string>
#include "httprequest.h"

namespace ashan
{
	void httprequest::set_header(std::string &&h, std::string &&v)
	{
		m_header.emplace(std::move(h), std::move(v));
	}
	void httprequest::set_body(std::string &&_body)
	{
		m_body = std::move(_body);
	}
	void httprequest::append(const char *buf, int size)
	{
		m_body.append(buf, size);
	}
	std::list<std::string> httprequest::serialize()
	{
		std::list<std::string> t;
		return t;
	}
}