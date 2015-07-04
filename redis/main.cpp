#include "Redis.h"
#include <iostream>
using namespace std;

#define print(j) cout << " " #j  << ": " << j  << "\n" << endl

int main(int argc, char** argv)
{
	auto t = CRedis();
	ST_REDIS_INFO info = {"127.0.0.1", 6379, 0};
	if (!t.init(info))
	{
		return -1;
	}

	// string²Ù×÷²âÊÔ
	print(t.string_set("dddd", "dddd"));
	print(t.string_set("mmmm", "mmmm"));
	print(t.string_set("tttt", 1111111));
	print(t.string_set("11", 1111111));
	print(t.string_get("dddd"));
	print(t.string_get("tttt"));
	print(t.del("dddd"));
	print(t.del(json::array({"dddd", "mmmm", 11, "454545", 8888, json::array(), "tttt", json::object()})));

	// list²Ù×÷²âÊÔ
	print(t.list_rpush("test", "first"));
	print(t.list_rpush("test", 111));
	print(t.list_rpush("test", json::array({ "dddd", "mmmm", 11, })));
	print(t.list_range("t", 0, -1));
	print(t.list_range("test", 0, -1));
	print(t.list_range("test", 0, 2));
	print(t.list_lpop("t"));
	print(t.list_lpop("test"));
	print(t.list_range("test", 0, -1));
	print(t.del("test"));

	// set²Ù×÷²âÊÔ
	print(t.set_add("set", "first"));
	print(t.set_add("set", 123));
	print(t.set_add("set", json::array({ "dddd", "mmmm", 11 })));
	print(t.set_ismember("set", "first"));
	print(t.set_ismember("set", 568));
	print(t.set_ismember("set", 123));
	print(t.set_ismember("setss", 123));
	print(t.del("set"));

	// push²Ù×÷²âÊÔ
	print(t.hash_set("hash", "name", "ysl"));
	print(t.hash_set("hash", "age", 18));
	print(t.hash_set("hash", "111", 18));
	print(t.hash_set("hash", "112", 18));
	print(t.hash_set("hash", "sex", 9527));
	print(t.hash_get("hash", "name"));
	print(t.hash_get("hash", "111"));
	print(t.hash_get("hash", 111));
	print(t.hash_get("hash", "ysl"));
	print(t.hash_get("hashddd", "nononono"));
	print(t.hash_getall("hashddd"));
	print(t.hash_getall("hash"));
	print(t.hash_del("hash", 111));
	print(t.hash_del("hashddd", 111));
	print(t.hash_del("hash", 122));
	print(t.hash_del("hash", "age"));
	print(t.hash_del("hashddd", "age"));
	print(t.hash_del("hash", "fuck"));
	print(t.hash_del("hash", json::array({ "name", 112, "fuck" })));
	print(t.hash_del("hashddd", json::array({ "name", 112, "fuck" })));
	print(t.hash_getall("hash"));

	print(t.hash_mget_as_map("hash", json::array({"ysl", "name", 111})));
	print(t.hash_mget_as_map("hashddd", json::array({ "ysl", "name", 111 })));

	print(t.hash_mget("hash", json::array({ "ysl", "name", 111 })));
	print(t.hash_mget("hashddd", json::array({ "ysl", "name", 111 })));

	print(t.hash_mset("hash", json::object({
		{ "lname", "ysl" },
		{ "lage", 18 },
	})));
	print(t.hash_getall("hash"));

	print(t.hash_incrby("hash", 9527));
	print(t.hash_incrby("hash", 9527, 3));

	print(t.hash_incrby("hash", "9528"));
	print(t.hash_incrby("hash", "9528", 3));
	print(t.hash_getall("hash"));

	print(t.del("hash"));

	// eval²Ù×÷²âÊÔ
	print(t.evalsha("a3d7ac8b61b5b5f31433c8dcd2dd07312612f9a1"));
	print(t.evalsha("1c29500335297cb27a2d7e6723165011d568d6ed"));
	print(t.evalsha("e1cab9b688ae75e72e1d2e382c67bb7a7bfb33c5"));
	print(t.evalsha("c77644bb4b658b48dbe17f3135f7085e99b4c1b7"));
	print(t.evalsha("7aa1f86330d99925bfe411f2c9cf6735dba339a0"));
	print(t.evalsha("27439e5ea0fb94e55533c1e55e70a3e81ac002b1"));
	print(t.evalsha("922ae6b0b255e3fee8de6cad9b70aadc7d46453c"));
	print(t.evalsha("5924dd70b5114e67f1837b415600b26dccebda4e"));
	print(t.evalsha("5924dd70b5114e67f1837b415600b26dccebda4e", json::array({"test"})));
	print(t.evalsha("5924dd70b5114e67f1837b415600b26dccebda4e", json::array({ "test", "show" })));

	return 0;
}