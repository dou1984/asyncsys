#include "kafkacli.h"
#include "asyncsys.h"

namespace ashan
{
	kafkacli::kafkacli(const std::string &addr)
	{
		if (asyncsys::splitaddr(addr, m_ip, m_port))
		{
		}
	}
	void kafkacli::consume(const std::string &topic, kafkacb &&)
	{
	}
	void kafkacli::product(const std::string &topic, const std::string &msg)
	{
	}
}