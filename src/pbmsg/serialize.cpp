#include "serialize.h"

namespace ashan
{
	void __serialize(MsgBase &_base, const std::string &_SRV, const std::string &_CMD, int error)
	{
		_base.set_server(_SRV);
		_base.set_method(_CMD);
		_base.set_error(error);
	}
	std::string serialize(MsgBase &_base)
	{
		auto byteSize = _base.ByteSize();
		std::string t;
		t.resize(sizeof(PBMSG) + byteSize);
		PBMSG *_msg = (PBMSG *)t.data();
		_msg->len = byteSize;
		_base.SerializeToArray(_msg->data(), _msg->size());
		return t;
	}
	std::string __serialize(const std::string &_SRV, const std::string &_CMD, int error, const std::string &code)
	{
		MsgBase _base;
		__serialize(_base, _SRV, _CMD, error);
		_base.set_code(code);
		return serialize(_base);
	}
	std::string __serialize(const std::string &_SRV, const std::string &_CMD, int error, const ::google::protobuf::Message &cls)
	{
		MsgBase _base;
		__serialize(_base, _SRV, _CMD, error);
		_base.mutable_data()->PackFrom(cls);
		return serialize(_base);
	}
	std::string __serialize(const std::string &_SRV, const std::string &_CMD, int error, const std::string &sess, const ::google::protobuf::Message &cls)
	{
		MsgBase _base;
		__serialize(_base, _SRV, _CMD, error);
		_base.set_session(sess);
		_base.mutable_data()->PackFrom(cls);
		return serialize(_base);
	}
	std::string __serialize(const std::string &_SRV, const std::string &_CMD, int error, u64 index, const std::string &sess, const ::google::protobuf::Message &cls)
	{
		MsgBase _base;
		__serialize(_base, _SRV, _CMD, error);
		_base.set_session(sess);
		_base.set_index(index);
		_base.mutable_data()->PackFrom(cls);
		return serialize(_base);
	}
	std::string __serialize(const std::string &_SRV, const std::string &_CMD, int error, u64 index, const ::google::protobuf::Message &cls)
	{
		MsgBase _base;
		__serialize(_base, _SRV, _CMD, error);
		_base.set_index(index);
		_base.mutable_data()->PackFrom(cls);
		return serialize(_base);
	}
}