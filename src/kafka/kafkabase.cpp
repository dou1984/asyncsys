#include <assert.h>
#include "../log.h"
#include "kafkabase.h"

namespace ashan
{
	void kafkabase::__logger(const rd_kafka_t *_kafka, int level, const char *fac, const char *buf)
	{
		LOG_ERR("%d-%s-%s:%s", level, fac, _kafka ? rd_kafka_name(_kafka) : "", buf);
	}
	void kafkabase::__init()
	{
		assert(m_conf == nullptr);
		if ((m_conf = rd_kafka_conf_new()) != nullptr)
		{
			LOG_ERR("kafka conf new %s", m_brokers.c_str());
			return;
		}		
		char errstr[1024];
		if (rd_kafka_conf_set(m_conf, "bootstrap.servers", m_brokers.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
		{
			LOG_ERR("Set kafka brokers: %s fail: %s", m_brokers.c_str(), errstr);
			return;
		}
		rd_kafka_conf_set_log_cb(m_conf, kafkabase::__logger);

		if ((m_kafka = rd_kafka_new(RD_KAFKA_CONSUMER, m_conf, errstr, sizeof(errstr))) != nullptr)
		{
			LOG_ERR("kafka new %s", errstr);
		}
	}
	void kafkabase::__clear()
	{
		if (m_conf != nullptr)
		{
			rd_kafka_conf_destroy(m_conf);
			m_conf = nullptr;
		}
		if (m_kafka != nullptr)
		{
			rd_kafka_destroy(m_kafka);
			m_kafka = nullptr;
		}
	}
	void kafkabase::__destroy_topic(rd_kafka_topic_t *_topic)
	{
		if (_topic != nullptr)
		{
			rd_kafka_topic_destroy(_topic);
		}
	}
	rd_kafka_topic_t *kafkabase::__init_topic(const char *topic)
	{
		rd_kafka_topic_t *_topic;
		if ((_topic = rd_kafka_topic_new(m_kafka, topic, NULL)) == nullptr)
		{
			LOG_ERR("create topic object:%s", rd_kafka_err2str(rd_kafka_last_error()));
		}
		return _topic;
	}
}
