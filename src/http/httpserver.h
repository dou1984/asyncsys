#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__
#include <functional>
#include <string>
#include <map>
#include <http_parser.h>
#include "httpreponse.h"
#include "../sp.h"
#include "../service/session.h"

namespace ashan
{
	struct httpsession;
	class httpserver : public sp<httpserver>
	{
	public:
		struct request
		{
			std::string m_url;
			std::string m_method;
			std::string m_body;
			std::map<std::string, std::string> m_header;
			std::string t;
		};
		using httpcb = std::function<void(std::shared_ptr<session>& s, const request &)>;
		httpserver(const std::string &);
		void add_method(const std::string &, httpcb &&);

	private:
		friend int on_message_complete(http_parser *);
		void __init();
		void __callback(std::shared_ptr<httpsession>);
		std::string m_url;
		std::map<std::string, httpcb> m_router;		
	};
}
#endif