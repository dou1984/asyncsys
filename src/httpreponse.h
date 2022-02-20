#ifndef __HTTP_REPONSE_H__
#define __HTTP_REPONSE_H__
#include <list>
#include <map>
#include <string>

namespace ashan
{
	class httpreponse
	{
		int m_error = 200;
		std::string m_body;
		std::map<std::string, std::string> m_header;

	public:
		void set_error(int err);
		void set_header(std::string &&, std::string &&);
		void set_body(std::string &&);
		void append(const char*, int );
		std::list<std::string> serialize();
	};
}

#endif