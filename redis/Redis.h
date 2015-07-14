#pragma once

#include <hiredis/hiredis.h>
#include <string>
#include <map>
#include <vector>
#include <list>
using std::string;
using std::map;
using std::vector;
using std::list;
#include "json.hpp"
using nlohmann::json;

typedef struct ST_REDIS_INFO
{
	string ip;
	int port;
	int db;
}ST_REDIS_INFO;

typedef vector<string> redis_vec_str;
typedef map<string, string> redis_map_str;
typedef vector<string> redis_list_string;

class CRedis
{
public:
	CRedis();
	virtual ~CRedis();

	bool init(const ST_REDIS_INFO& info);
	bool select_db(int nDB = 0);									// 选择数据库
	int load_lua_script(const string& name, const string& content);
	int load_lua_script(const redis_map_str& rp);

	static json discard_json();
	static json null_json();

	// 通用操作
	json del(const char* key);
	json del(const string& key);
	json del(long long key);
	json del(const json& json_array);
	json expire(const string& key, int expire_sec);
	json expire_at(const string& key, time_t expire_ts);

	// string操作
	json string_get(const string& key);
	json string_get(long long key);
	json string_set(const string& key, const string& value);
	json string_set(const string& key, long long value);

	// list操作
	json list_rpush(const string& key, const char* value);
	json list_rpush(const string& key, const string& value);
	json list_rpush(const string& key, long long value);
	json list_rpush(const string& key, const json& json_array);
	json list_range(const string& key, long long begin, long long end);
	json list_lpop(const string& key);

	// set操作
	json set_add(const string& key, const char* value);
	json set_add(const string& key, const string& value);
	json set_add(const string& key, long long value);
	json set_add(const string& key, const json& json_array);
	json set_ismember(const string& key, const string& value);
	json set_ismember(const string& key, long long value);

	// hash操作
	json hash_get(const string& key, const string& field);
	json hash_get(const string& key, long long field);
	json hash_set(const string& key, const string& field, long long value);
	json hash_set(const string& key, const string& field, const string& value);
	json hash_mget_as_map(const string& key, const json& json_array);
	json hash_mget(const string& key, const json& json_array);
	json hash_mset(const string& key, const json& json_map);
	json hash_mset(const string& key, const json& k_array, const json& v_array);
	json hash_getall(const string& key);
	json hash_del(const string& key, const char* field);
	json hash_del(const string& key, long long field);
	json hash_del(const string& key, const string& field);
	json hash_del(const string& key, const json& json_array);
	json hash_incrby(const string& key, const string& field, long long incr = 1);
	json hash_incrby(const string& key, long long field, long long incr = 1);

	// eval
	json evalsha(const string& sha1, const json& keys = nullptr, const json& argvs = nullptr);
	json evalscript(const string& name, const json& keys = nullptr, const json& argvs = nullptr);
private:
	bool _connect(const char* ip, int port);						// 连接redis
	bool _auth(const string& pwd);									// 认证客户端
	void _disconnect();												// 断开redis
	bool _check_connect();											// 测试连接是否正常
private:
	redisContext* m_pRedis;			// 负责redis的各种操作
	int m_nDB;						// 当前正在使用的DB
	string m_strIp;
	int m_nPort;
	string m_strPasswd;
	redis_map_str m_scriptLua;
};
