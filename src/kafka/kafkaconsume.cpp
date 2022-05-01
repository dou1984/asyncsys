#include "../log.h"
#include "../net/asyncsys.h"
#include "kafkaconsume.h"

namespace ashan
{
	kafkaconsume::kafkaconsume(const std::string &addr)
	{
		if (asyncsys::splitaddr(addr, m_ip, m_port))
		{
			__init();
		}
		else
		{
			m_brokers = addr;
		}
	}
	kafkaconsume::~kafkaconsume()
	{
		__clear();
		__destroy_topic(m_topic);
	}
	void kafkaconsume::pull(const std::string &topic, int partition, kafkacb &&_callback)
	{
		if ((m_topic = __init_topic(topic.c_str())) == nullptr)
		{
			LOG_ERR("create topic object:%s", rd_kafka_err2str(rd_kafka_last_error()));
			return;
		}
		while (m_state == e_state_running)
		{
			rd_kafka_poll(m_kafka, 0);
			auto _msg = rd_kafka_consume(m_topic, partition, m_timeout);
			if (_msg->err)
			{
				if (_msg->err != RD_KAFKA_RESP_ERR__PARTITION_EOF)
				{
					LOG_ERR("consume error %s %d offset:%ld: %s", rd_kafka_topic_name(_msg->rkt),
							_msg->partition, _msg->offset, rd_kafka_message_errstr(_msg));
				}
				kfk_results r;
				r.last_msg = rd_kafka_message_errstr(_msg);				
				r.last_error = rd_kafka_last_error();
				r.offset = _msg->offset;
				_callback(&r);
			}
			else
			{
				kfk_results r;
				r.key = (char *)_msg->key;
				r.payload = (char *)_msg->payload;
				r.offset = _msg->offset;
				_callback(&r);
			}
			rd_kafka_message_destroy(_msg);
		}
		rd_kafka_consume_stop(m_topic, partition);
		m_topic = nullptr;
	}	
}