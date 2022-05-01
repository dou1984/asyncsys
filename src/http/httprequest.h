#ifndef __HTTPREQUERT_H__
#define __HTTPREQUERT_H__
#include <map>
#include <string>

namespace ashan
{
	class httprequest
	{
		std::map<std::string, std::string> m_header;
		std::string m_body;

	public:
		void set_header(std::string &&, std::string &&);
		void set_body(std::string &&);
		void append(const char *, int);

		std::list<std::string> serialize();
	};
}
#endif