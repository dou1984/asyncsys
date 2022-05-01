#ifndef __KAFKACONSUME_H__
#define __KAFKACONSUME_H__
#include <list>
#include <thread>
#include <string>
#include <functional>
#include "../sp.h"
#include "../myresults.h"
#include "kafkabase.h"
#include "kafkaconsume.h"

namespace ashan
{
	class kafkaconsume final : public kafkabase, public sp<kafkaconsume>
	{
	public:
		struct kfk_results final : myresults
		{
			uint64_t offset = 0;
			const char *key = 0;
			const char *payload = 0;			
		};
		using kafkacb = std::function<void(const kfk_results *)>;

		kafkaconsume(const std::string &addr);
		~kafkaconsume();
		void pull(const std::string &topic, int partition, kafkacb &&);
		void pull(const std::string &topic, kafkacb &&_callback) { pull(topic, 0, std::move(_callback)); }
		void stop() { m_state = e_state_stop; }

	private:
		enum e_state
		{
			e_state_stop,
			e_state_running,
		};
		int m_state = e_state_running;
		rd_kafka_topic_t *m_topic = nullptr;
	};
}
#endif