#ifndef __KAFKABASE_H__
#define __KAFKABASE_H__
#include <string>
#include <librdkafka/rdkafka.h>

namespace ashan
{
	struct kafkabase
	{
		std::string m_brokers;
		std::string m_ip;
		int m_port = 0;
		int m_timeout = 100;
		
		rd_kafka_conf_t *m_conf = nullptr;
		rd_kafka_t *m_kafka = nullptr;

		void __init();
		void __clear();
		static void __logger(const rd_kafka_t *_kafka, int level, const char *fac, const char *buf);

		void __destroy_topic(rd_kafka_topic_t *m_topic);
		rd_kafka_topic_t *__init_topic(const char *topic);
	};
}

#endif