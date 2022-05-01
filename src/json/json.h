#ifndef __JSON_H__
#define __JSON_H__
#include <memory>
#include <type_traits>
#include <cjson/cJSON.h>

namespace ashan
{
	struct json final
	{
		cJSON *m_json = nullptr;
		cJSON *m_root = nullptr;
		json()
		{
			m_json = cJSON_CreateObject();
		}
		json(cJSON *_json, cJSON *_root) : m_json(_json), m_root(m_json)
		{
		}
		json(json &&j)
		{
			std::swap(m_json, j.m_json);
			std::swap(m_root, j.m_root);
		}
		~json()
		{
			if (m_root == nullptr && m_json != nullptr)
			{
				cJSON_Delete(m_json);
			}
		}
		std::unique_ptr<char> serialize()
		{
			return m_json != nullptr ? std::unique_ptr<char>(cJSON_Print(m_json)) : nullptr;
		}
		std::unique_ptr<char> print()
		{
			return m_json != nullptr ? std::unique_ptr<char>(cJSON_PrintUnformatted(m_json)) : nullptr;
		}
		bool print(char *buf, int len, bool formated = true)
		{
			return m_json != nullptr && cJSON_PrintPreallocated(m_json, buf, len, formated);
		}
		void parse(const char *_str)
		{
			if (m_root == nullptr && m_json != nullptr)
			{
				cJSON_Delete(m_json);
			}
			m_json = cJSON_Parse(_str);
		}
		void set(const char *_item, const char *_value)
		{
			if (m_json != nullptr)
			{
				cJSON_AddStringToObject(m_json, _item, _value);
			}
		}
		template <class _Value>
		void set(const char *_item, const _Value &_value)
		{
			static_assert(std::is_arithmetic<_Value>::value);
			if (m_json != nullptr)
			{
				cJSON_AddNumberToObject(m_json, _item, _value);
			}
		}
		json operator[](const char *_item)
		{
			auto obj = cJSON_GetObjectItem(m_json, _item);
			if (obj != nullptr)
			{
				return json(obj, m_json);
			}
			obj = cJSON_CreateObject();
			if (obj != nullptr)
			{
				cJSON_AddItemToObject(m_json, _item, obj);
			}
			return json(obj, m_json);
		}
		bool asBool()
		{
			return m_json != nullptr && cJSON_IsBool(m_json) ? cJSON_IsTrue(m_json) : false;
		}
		const char *asString()
		{
			static char null_string[] = "";
			return m_json != nullptr && cJSON_IsString(m_json) ? m_json->valuestring : null_string;
		}
		int asInt()
		{
			return m_json != nullptr && cJSON_IsNumber(m_json) ? m_json->valueint : 0;
		}
		double asDouble()
		{
			return m_json != nullptr && cJSON_IsNumber(m_json) ? m_json->valuedouble : 0;
		}
		const char *name()
		{
			static char null_string[] = "";
			return m_json != nullptr ? m_json->string : null_string;
		}
		int getArraySize()
		{
			return m_json != nullptr && cJSON_IsArray(m_json) ? cJSON_GetArraySize(m_json) : 0;
		}
		json operator[](int pos)
		{
			return m_json != nullptr && pos >= 0 && pos < cJSON_GetArraySize(m_json)
					   ? json(cJSON_GetArrayItem(m_json, pos), m_json)
					   : json(nullptr, m_json);
		}
		auto createArray()
		{
			if (m_json == nullptr)
			{
				m_json = cJSON_CreateArray();
			}
			else if (!cJSON_IsArray(m_json))
			{
				cJSON_Delete(m_json);
				m_json = cJSON_CreateArray();
			}
			return this;
		}
		auto createArray(const char *_item)
		{
			assert(_item != nullptr);
			if (m_json == nullptr)
			{
				m_json = cJSON_CreateObject();
			}
			auto obj = cJSON_CreateArray();
			cJSON_AddItemToObject(m_json, _item, obj);
			return this;
		}
		void push_back(json &j)
		{
			if (cJSON_IsArray(m_json))
			{
				cJSON_AddItemToArray(m_json, j.m_json);
				j.m_json = nullptr;
				j.m_root = nullptr;
			}
		}
		void push_back(const char *_d)
		{
			__push_back(cJSON_CreateString(_d));
		}
		void push_back(char *_d)
		{
			__push_back(cJSON_CreateString(_d));
		}
		void push_back(const std::string &_d)
		{
			__push_back(cJSON_CreateString(_d.c_str()));
		}
		template <class _CLS>
		void push_back(const _CLS &_d)
		{
			static_assert(std::is_arithmetic<_CLS>::value);
			__push_back(cJSON_CreateNumber(_d));
		}
		void __push_back(cJSON *j)
		{
			if (cJSON_IsArray(m_json))
			{
				cJSON_AddItemToArray(m_json, j);
			}
		}
	};
}
#endif