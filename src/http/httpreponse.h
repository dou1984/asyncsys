#ifndef __HTTP_REPONSE_H__
#define __HTTP_REPONSE_H__
#include <list>
#include <map>
#include <string>
#include "../sp.h"

namespace ashan
{
	class httpreponse : public sp<httpreponse>
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

		enum error_code
		{
			error_ok = 200,
			error_bad_request = 400,
			error_frobidden = 403,
			error_not_found = 404,
			error_request_entity_too_large = 413,
			error_expectation_failed = 417,
			error_server_error = 500,
		};
	};
}

#endif