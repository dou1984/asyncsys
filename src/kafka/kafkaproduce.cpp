#include "../invalid.h"
#include "../net/asyncsys.h"
#include "kafkaproduce.h"

namespace ashan
{
	kafkaproduce::kafkaproduce(const std::string &addr)
	{
		if (asyncsys::splitaddr(addr, m_ip, m_port))
		{
			kafkabase::__init();
		}
		else
		{
			m_brokers = addr;
		}
	}
	kafkaproduce::~kafkaproduce()
	{
		__clear();
		for (auto it = m_topic.begin(); it != m_topic.end(); ++it)
		{
			__destroy_topic(it->second);
		}
	}
	rd_kafka_topic_t *kafkaproduce::__init(const std::string &topic)
	{
		if (auto it = m_topic.find(topic); it != m_topic.end())
		{
			return it->second;
		}
		auto __topic = __init_topic(topic.c_str());
		m_topic.emplace(topic, __topic);
		return __topic;
	}
	int kafkaproduce::push(const std::string &topic, const std::string &msg)
	{
		auto __topic = __init(topic);
		if (__topic != nullptr)
		{
			return rd_kafka_produce(
				__topic, 0, RD_KAFKA_MSG_F_COPY,
				(void *)msg.c_str(), msg.size(), NULL, 0, NULL);
		}
		return INVALID;
	}
	int kafkaproduce::push(const std::string &topic, const std::string &key, const std::string &msg)
	{
		auto __topic = __init(topic);
		if (__topic != nullptr)
		{
			return rd_kafka_produce(
				__topic, 0, RD_KAFKA_MSG_F_COPY,
				(void *)msg.c_str(), msg.size(), key.c_str(), key.size(), NULL);
		}
		return INVALID;
	}
	int kafkaproduce::push(const std::string &topic, int partition, const std::string &key, const std::string &msg)
	{
		auto __topic = __init(topic);
		if (__topic != nullptr)
		{
			return rd_kafka_produce(
				__topic, partition, RD_KAFKA_MSG_F_COPY,
				(void *)msg.c_str(), msg.size(), key.c_str(), key.size(), NULL);
		}
		return INVALID;
	}
	int kafkaproduce::push(const std::string &topic, int partition, const std::string &msg)
	{
		auto __topic = __init(topic);
		if (__topic != nullptr)
		{
			return rd_kafka_produce(
				__topic, partition, RD_KAFKA_MSG_F_COPY,
				(void *)msg.c_str(), msg.size(), NULL, 0, NULL);
		}
		return INVALID;
	}
}