#ifndef __CLIQUERY_H__
#define __CLIQUERY_H__
#include <string>
#include <stdio.h>
#include <stdlib.h>

namespace ashan
{
	template <class _CB>
	struct cliquery
	{
		std::string m_query;
		_CB m_callback;
		cliquery(std::string &&_query, _CB &&_cb) : m_query(std::move(_query)), m_callback(std::move(_cb))
		{
		}
	};
}
#endif