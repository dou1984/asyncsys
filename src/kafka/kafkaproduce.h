#ifndef __KAFKAPRODUCE_H__
#define __KAFKAPRODUCE_H__
#include <map>
#include <string>
#include "../sp.h"
#include "kafkabase.h"

namespace ashan
{
	class kafkaproduce final : public kafkabase, public sp<kafkaproduce>
	{
		std::map<std::string, rd_kafka_topic_t *> m_topic;
		rd_kafka_topic_t *__init(const std::string &);

	public:
		kafkaproduce(const std::string &addr);
		~kafkaproduce();

		int push(const std::string &topic, const std::string &msg);
		int push(const std::string &topic, const std::string &key, const std::string &msg);
		int push(const std::string &topic, int partition, const std::string &key, const std::string &msg);
		int push(const std::string &topic, int partition, const std::string &msg);
	};
}

#endif