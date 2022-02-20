#ifndef __KAFKACLI_H__
#define __KAFKACLI_H__
#include <functional>
#include "clistate.h"
#include "cliquery.h"
namespace ashan
{
	class kafkacli
	{
	public:
		struct kfk_results
		{
			int last_error = 0;
			const char *last_msg = 0;
		};
		using kafkacb = std::function<void(const kfk_results *)>;

		kafkacli(const std::string &addr);
		void consume(const std::string &topic, kafkacb &&);
		void product(const std::string &topic, const std::string &msg);

	private:
		std::string m_ip;
		int m_port = 0;
	};
}
#endif