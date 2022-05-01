#include "jsontest.h"
#include <asys.h>

using namespace ashan;

int test_json()
{
	json j;

	json t;
	t.set("hello", "world");

	auto a = j.createArray();
	a->push_back(t);
	a->push_back(1);
	a->push_back(0.5);

	auto s = j.print();
	LOG_DBG("%s", s.get());
	return 0;
}