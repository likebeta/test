#include "Redis.h"
//#include "ILog.h"
#include <string.h>
//#include "Tools.h"
#include <sstream>

// extern ILog* g_pLog;
// #define WriteDebugLog(_fmt, ...) g_pLog->Debug("CTest::%s, " _fmt, __FUNCTION__, ##__VA_ARGS__)
// #define WriteLog(_fmt, ...) g_pLog->Info("CTest::%s, " _fmt , __FUNCTION__, ##__VA_ARGS__)
// #define WriteWarnLog(_fmt, ...) g_pLog->Warn("CTest::%s, " _fmt, __FUNCTION__, ##__VA_ARGS__)
// #define WriteErrorLog(_fmt, ...) g_pLog->Error("CTest::%s, " _fmt, __FUNCTION__, ##__VA_ARGS__)
// #define WriteNetwork(_fmt, ...) g_pLog->Network("CTest::%s, " _fmt, __FUNCTION__, ##__VA_ARGS__)

#define WriteDebugLog(_fmt, ...) printf("CTest::%s, " _fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define WriteLog(_fmt, ...) printf("CTest::%s, " _fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define WriteWarnLog(_fmt, ...) printf("CTest::%s, " _fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define WriteErrorLog(_fmt, ...) printf("CTest::%s, " _fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define WriteNetwork(_fmt, ...) printf("CTest::%s, " _fmt "\n", __FUNCTION__, ##__VA_ARGS__)

using std::stringstream;
template<typename T>
string integer2str(T n)
{
	stringstream strStream;
	strStream << n;
	return strStream.str();
}

CRedis::CRedis()
{
	m_pRedis = NULL;
	m_strIp = "127.0.0.1";
	m_nPort = 6379;
	m_nDB = 0;
}

bool CRedis::init(const ST_REDIS_INFO& info)
{
	m_strIp = info.ip;
	m_nPort = info.port;
	m_nDB = info.db;
	WriteLog("init connect %s:%d:%d", info.ip.c_str(), info.port, info.db);

	if (!_connect(info.ip.c_str(), info.port))
	{
		return false;
	}

	if (!_auth(m_strPasswd))
	{
		WriteErrorLog("init, redis passwd not correct!");
		return false;
	}

	if (!select_db(info.db))
	{
		WriteErrorLog("init, select db failed!");
		return false;
	}

	return true;
}

int CRedis::load_lua_script(const string& name, const string& content)
{
	if (!_check_connect())
	{
		return -1;
	}

	WriteLog("SCRIPT LOAD %s", content.c_str());
	redisAppendCommand(m_pRedis, "SCRIPT LOAD %s", content.c_str());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return -1;
	}

	// 返回值脚本的sha1
	if (reply->type == REDIS_REPLY_STRING)
	{
		m_scriptLua[name] = string(reply->str, reply->str + reply->len);
		freeReplyObject(reply);
		return 0;
	}
	else
	{
		freeReplyObject(reply);
		return -1;
	}
}

int CRedis::load_lua_script(const redis_map_str& rp)
{
	for (auto it = rp.begin(); it != rp.end(); ++it)
	{
		WriteLog("load lua script: %s -- %s", it->first.c_str(), it->second.c_str());
		m_scriptLua[it->first] = it->second;
	}

	return 0;
}

CRedis::~CRedis()
{
	_disconnect();
}

static json __discard_json = json(json::value_t::discarded);
static json __null_json = json(json::value_t::null);

json CRedis::discard_json()
{
	return __discard_json;
}

json CRedis::null_json()
{
	return __null_json;
}

static string __dump_json(const json& j)
{
	if (j.is_string())
	{
		return j.get<string>();
	}
	else
	{
		return j.dump();
	}
}

json CRedis::del(const char* key)
{
	if (key == NULL)
	{
		return __discard_json;
	}
	return del(string(key));
}

json CRedis::del(const string& key)
{
	if (key.empty() || !_check_connect())
	{
		return __discard_json;
	}
	WriteDebugLog("DEL %s", key.c_str());
	redisAppendCommand(m_pRedis, "DEL %b", key.data(), key.size());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	// 返回值为删除的个数
	if (reply->type == REDIS_REPLY_INTEGER)
	{
		auto tmp = reply->integer;
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::del(long long key)
{
	string _key = integer2str(key);
	return del(_key);
}

json CRedis::del(const json& json_array)
{
	if (!json_array.is_array() || json_array.empty())
	{
		return __discard_json;
	}

	auto it = json_array.begin();
	string str_cmd = *it;
	while ((++it) != json_array.end())
	{
		if (it->is_discarded())
		{
			return __discard_json;
		}
		str_cmd += " " + __dump_json(*it);
	}

	return del(str_cmd);
}

json CRedis::expire(const string& key, int expire_sec)
{
	if (key.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("EXPIRE %s %d", key.c_str(), expire_sec);
	redisAppendCommand(m_pRedis, "EXPIRE %s %d", key.c_str(), expire_sec);
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		auto tmp = reply->integer;
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::expire_at(const string& key, time_t expire_ts)
{
	if (key.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("EXPIREAT %s %lld", key.c_str(), (long long)expire_ts);
	redisAppendCommand(m_pRedis, "EXPIREAT %s %lld", key.c_str(), (long long)expire_ts);
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		auto tmp = reply->integer;
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::string_get(const string& key)
{
	if (key.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("GET %s", key.c_str());
	redisAppendCommand(m_pRedis, "GET %s", key.c_str());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_STRING)	// 存在
	{
		string tmp(reply->str, reply->str + reply->len);
		freeReplyObject(reply);
		return tmp;
	}
	else if (reply->type == REDIS_REPLY_NIL) // 不存在
	{
		freeReplyObject(reply);
		return __null_json;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::string_set(const string& key, long long value)
{
	string _value = integer2str(value);
	return string_set(key, _value);
}

json CRedis::string_get(long long key)
{
	string _key = integer2str(key);
	return string_get(_key);
}

json CRedis::set_add(const string& key, const char* value)
{
	if (value == NULL)
	{
		return __discard_json;
	}
	return set_add(key, string(value));
}

json CRedis::string_set(const string& key, const string& value)
{
	if (key.empty() || value.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("SET %s %s", key.c_str(), value.c_str());
	redisAppendCommand(m_pRedis, "SET %s %b", key.c_str(), value.data(), value.size());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	// 返回值为状态
	if (reply->type == REDIS_REPLY_STATUS)
	{
		string tmp(reply->str, reply->str + reply->len);
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::list_rpush(const string& key, const char* value)
{
	if (value == NULL)
	{
		return __discard_json;
	}
	return list_rpush(key, string(value));
}

json CRedis::list_rpush(const string& key, long long value)
{
	string _value = integer2str(value);
	return list_rpush(key, _value);
}

json CRedis::list_rpush(const string& key, const string& value)
{
	if (key.empty() || value.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("RPUSH %s %s", key.c_str(), value.c_str());
	redisAppendCommand(m_pRedis, "RPUSH %s %b", key.c_str(), value.data(), value.size());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	// 返回为list长度
	if (reply->type == REDIS_REPLY_INTEGER)
	{
		auto tmp = reply->integer;
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::list_rpush(const string& key, const json& json_array)
{
	if (!json_array.is_array() || json_array.empty())
	{
		return __discard_json;
	}

	auto it = json_array.begin();
	string str_cmd = *it;
	while ((++it) != json_array.end())
	{
		if (it->is_discarded())
		{
			return __discard_json;
		}
		str_cmd += " " + __dump_json(*it);
	}

	return list_rpush(key, str_cmd);
}

json CRedis::list_range(const string& key, long long begin, long long end)
{
	if (key.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("LRANGE %s %lld %lld", key.c_str(), begin, end);
	redisAppendCommand(m_pRedis, "LRANGE %s %lld %lld", key.c_str(), begin, end);
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		json j = json::array();
		for (size_t i = 0; i < reply->elements; ++i)
		{
			if (reply->element[i]->type == REDIS_REPLY_STRING)	// 只返回存在的field
			{
				j.push_back(string(reply->element[i]->str, reply->element[i]->str + reply->element[i]->len));
			}
		}
		freeReplyObject(reply);
		return j;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::list_lpop(const string& key)
{
	if (key.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("LPOP %s", key.c_str());
	redisAppendCommand(m_pRedis, "LPOP %s", key.c_str());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	// 返回为list长度
	if (reply->type == REDIS_REPLY_STRING)
	{
		string tmp(reply->str, reply->str + reply->len);
		freeReplyObject(reply);
		return tmp;
	}
	else if (reply->type == REDIS_REPLY_NIL)
	{
		freeReplyObject(reply);
		return __null_json;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::set_add(const string& key, const string& value)
{
	if (key.empty() || value.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("SADD %s %s", key.c_str(), value.c_str());
	redisAppendCommand(m_pRedis, "SADD %s %b", key.c_str(), value.data(), value.size());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	// 添加成功数量
	if (reply->type == REDIS_REPLY_INTEGER)
	{
		auto tmp = reply->integer;
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::set_add(const string& key, long long value)
{
	string _value = integer2str(value);
	return set_add(key, _value);
}

json CRedis::set_add(const string& key, const json& json_array)
{
	if (!json_array.is_array() || json_array.empty())
	{
		return __discard_json;
	}

	auto it = json_array.begin();
	string str_cmd = *it;
	while ((++it) != json_array.end())
	{
		if (it->is_discarded())
		{
			return __discard_json;
		}
		str_cmd += " " + __dump_json(*it);
	}

	return set_add(key, str_cmd);
}

json CRedis::set_ismember(const string& key, const string& value)
{
	if (key.empty() || value.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("SISMEMBER %s %s", key.c_str(), value.c_str());
	redisAppendCommand(m_pRedis, "SISMEMBER %s %b", key.c_str(), value.data(), value.size());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		auto tmp = reply->integer;
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::set_ismember(const string& key, long long value)
{
	string _value = integer2str(value);
	return set_ismember(key, _value);
}

json CRedis::hash_get(const string& key, const string& field)
{
	if (key.empty() || field.empty())
	{
		return __discard_json;
	}

	if (!_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("HGET %s %s", key.c_str(), field.c_str());
	redisAppendCommand(m_pRedis, "HGET %s %b", key.c_str(), field.data(), field.size());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_STRING)	// 存在
	{
		string tmp(reply->str, reply->str + reply->len);
		freeReplyObject(reply);
		return tmp;
	}
	else if (reply->type == REDIS_REPLY_NIL) // 不存在
	{
		freeReplyObject(reply);
		return __null_json;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::hash_get(const string& key, long long field)
{
	string _field = integer2str(field);
	return hash_get(key, _field);
}

json CRedis::hash_set(const string& key, const string& field, long long value)
{
	string _value = integer2str(value);
	return hash_set(key, field, _value);
}

json CRedis::hash_set(const string& key, const string& field, const string& value)
{
	if (key.empty() || field.empty() || value.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
	redisAppendCommand(m_pRedis, "HSET %s %b %b", key.c_str(), field.data(), field.size(), value.data(), value.size());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	// 返回设置后增加的数量
	if (reply->type == REDIS_REPLY_INTEGER)
	{
		auto tmp = reply->integer;
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::hash_mget_as_map(const string& key, const json& json_array)
{
	if (key.empty() || !json_array.is_array() || json_array.empty() || !_check_connect())
	{
		return __discard_json;
	}

	auto it = json_array.begin();
	if (it->is_discarded())
	{
		return __discard_json;
	}
	string str_cmd = __dump_json(*it);
	while ((++it) != json_array.end())
	{
		if (it->is_discarded())
		{
			return __discard_json;
		}
		str_cmd += " " + __dump_json(*it);
	}
	string cmd = "HMGET " + key + " " + str_cmd;
	WriteDebugLog("%s", cmd.c_str());
	redisAppendCommand(m_pRedis, cmd.c_str());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		if (reply->elements != json_array.size())
		{
			return __discard_json;
		}

		json j = json::object();
		for (size_t i = 0; i < reply->elements; ++i)
		{
			string s = __dump_json(json_array[i]);
			if (reply->element[i]->type == REDIS_REPLY_STRING)	// 只返回存在的field
			{
				j[s] = string(reply->element[i]->str, reply->element[i]->len);
			}
			else if (reply->element[i]->type == REDIS_REPLY_NIL)
			{
				//j[s] = nullptr;
			}
			else
			{
				return __discard_json;
			}
		}
		freeReplyObject(reply);
		return j;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::hash_mget(const string& key, const json& json_array)
{
	if (key.empty() || !json_array.is_array() || json_array.empty() || !_check_connect())
	{
		return __discard_json;
	}

	auto it = json_array.begin();
	if (it->is_discarded())
	{
		return __discard_json;
	}
	string str_cmd = __dump_json(*it);
	while ((++it) != json_array.end())
	{
		if (it->is_discarded())
		{
			return __discard_json;
		}
		str_cmd += " " + __dump_json(*it);
	}

	string cmd = "HMGET " + key + " " + str_cmd;
	WriteDebugLog("%s", cmd.c_str());
	redisAppendCommand(m_pRedis, cmd.c_str());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		if (reply->elements != json_array.size())
		{
			return __discard_json;
		}

		json j = json::array();
		for (size_t i = 0; i < reply->elements; ++i)
		{
			string s = __dump_json(json_array[i]);
			if (reply->element[i]->type == REDIS_REPLY_STRING)
			{
				j.push_back(string(reply->element[i]->str, reply->element[i]->len));
			}
			else if (reply->element[i]->type == REDIS_REPLY_NIL)
			{
				j.push_back(nullptr);
			}
			else
			{
				return __discard_json;
			}
		}

		freeReplyObject(reply);
		return j;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::hash_getall(const string& key)
{
	if (key.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("HGETALL %s", key.c_str());
	redisAppendCommand(m_pRedis, "HGETALL %b", key.data(), key.size());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		json j = json::object();
		for (size_t i = 0; i < reply->elements; i += 2)
		{
			if (reply->element[i]->type == REDIS_REPLY_STRING && reply->element[i + 1]->type == REDIS_REPLY_STRING)
			{
				j[reply->element[i]->str] = string(reply->element[i + 1]->str, reply->element[i + 1]->len);
			}
		}

		freeReplyObject(reply);
		return j;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::hash_mset(const string& key, const json& json_map)
{
	if (key.empty() || !json_map.is_object() || json_map.empty() || !_check_connect())
	{
		return __discard_json;
	}

	auto it = json_map.begin();
	if (it.value().is_discarded())
	{
		return __discard_json;
	}
	string str_cmd = it.key() + " " + __dump_json(it.value());
	while ((++it) != json_map.end())
	{
		str_cmd += " " + it.key();
		auto& value = it.value();
		if (value.is_discarded())
		{
			return __discard_json;
		}
		else
		{
			str_cmd += " " + __dump_json(value);
		}
	}

	string cmd = "HMSET " + key + " " + str_cmd;
	WriteDebugLog("%s", cmd.c_str());
	redisAppendCommand(m_pRedis, cmd.c_str());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_STATUS)
	{
		string tmp(reply->str, reply->str + reply->len);
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::hash_mset(const string& key, const json& k_array, const json& v_array)
{
	if (key.empty() || !k_array.is_array() || k_array.empty() || !v_array.is_array() || v_array.empty())
	{
		return __discard_json;
	}

	if (k_array.size() != v_array.size() || !_check_connect())
	{
		return __discard_json;
	}

	if (k_array[0].is_discarded() || v_array[0].is_discarded())
	{
		return __discard_json;
	}
	string str_cmd = __dump_json(k_array[0]) + " " + __dump_json(v_array[0]);
	size_t index = 0;
	while ((++index) < k_array.size())
	{
		if (k_array[index].is_discarded() || v_array[index].is_discarded())
		{
			return __discard_json;
		}
		str_cmd += " " + __dump_json(k_array[index]) + " " + __dump_json(v_array[index]);
	}

	string cmd = "HMSET " + key + " " + str_cmd;
	WriteDebugLog("%s", cmd.c_str());
	redisAppendCommand(m_pRedis, cmd.c_str());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_STATUS)
	{
		string tmp(reply->str, reply->str + reply->len);
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::hash_del(const string& key, long long field)
{
	string _field = integer2str(field);
	return hash_del(key, _field);
}

json CRedis::hash_del(const string& key, const char* field)
{
	if (field == NULL)
	{
		return __discard_json;
	}
	return hash_del(key, string(field));
}

json CRedis::hash_del(const string& key, const string& field)
{
	if (key.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("HDEL %s %s", key.c_str(), field.c_str());
	redisAppendCommand(m_pRedis, "HDEL %s %b", key.c_str(), field.data(), field.size());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	// 返回删除数量
	if (reply->type == REDIS_REPLY_INTEGER)
	{
		auto tmp = reply->integer;
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::hash_del(const string& key, const json& json_array)
{
	if (json_array.empty() || !json_array.is_array())
	{
		return __discard_json;
	}

	auto it = json_array.begin();
	string str_cmd = *it;
	while ((++it) != json_array.end())
	{
		if (it->is_discarded())
		{
			return __discard_json;
		}
		str_cmd += " " + __dump_json(*it);
	}

	return hash_del(key, str_cmd);
}

json CRedis::hash_incrby(const string& key, const string& field, long long incr /* = 1 */)
{
	if (key.empty() || field.empty() || !_check_connect())
	{
		return __discard_json;
	}

	WriteDebugLog("HINCRBY %s %s %lld", key.c_str(), field.c_str(), incr);
	redisAppendCommand(m_pRedis, "HINCRBY %s %b %lld", key.c_str(), field.data(), field.size(), incr);
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		auto tmp = reply->integer;
		freeReplyObject(reply);
		return tmp;
	}
	else
	{
		freeReplyObject(reply);
		return __discard_json;
	}
}

json CRedis::hash_incrby(const string& key, long long field, long long incr /* = 1 */)
{
	string _field = integer2str(field);
	return hash_incrby(key, _field, incr);
}

static json __assign_json(redisReply* reply)
{
	if (reply == NULL)
	{
		return __discard_json;
	}
	if (reply->type == REDIS_REPLY_ERROR)
	{
		return __discard_json;
	}
	else if (reply->type == REDIS_REPLY_STRING)
	{
		return string(reply->str, reply->str + reply->len);
	}
	else if (reply->type == REDIS_REPLY_ARRAY)
	{
		json j = json::array();
		for (size_t i = 0; i < reply->elements; ++i)
		{
			j.push_back(__assign_json(reply->element[i]));
		}
		return j;
	}
	else if (reply->type == REDIS_REPLY_INTEGER)
	{
		return reply->integer;
	}
	else if (reply->type == REDIS_REPLY_NIL)
	{
		return __null_json;
	}
	else// if (reply->type == REDIS_REPLY_STATUS)
	{
		return string(reply->str, reply->str + reply->len);
	}
}

json CRedis::evalsha(const string& sha1, const json& keys /* = nullptr */, const json& argvs /* = nullptr */)
{
	if (sha1.empty() || !_check_connect())
	{
		return __discard_json;
	}

	if (!keys.is_array() && !keys.is_null())
	{
		return __discard_json;
	}

	if (!argvs.is_array() && !argvs.is_null())
	{
		return __discard_json;
	}

	size_t len = 0;
	if (keys.is_array())
	{
		len = keys.size();
	}

	string str_cmd = integer2str(len);
	if (keys.is_array() && !keys.empty())
	{
		auto it = keys.begin();
		str_cmd += " " + __dump_json(*it);
		while ((++it) != keys.end())
		{
			if (it->is_discarded())
			{
				return __discard_json;
			}
			str_cmd += " " + __dump_json(*it);
		}
	}

	if (argvs.is_array() && !argvs.empty())
	{
		auto it = argvs.begin();
		str_cmd += " " + __dump_json(*it);
		while ((++it) != argvs.end())
		{
			if (it->is_discarded())
			{
				return __discard_json;
			}
			str_cmd += " " + __dump_json(*it);
		}
	}

	string cmd = "EVALSHA " + sha1 + " " + str_cmd;
	WriteLog("%s", cmd.c_str());
	redisAppendCommand(m_pRedis, cmd.c_str());
	redisReply* reply = NULL;
	redisGetReply(m_pRedis, (void**)&reply);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return __discard_json;
	}

	json j = __assign_json(reply);
	freeReplyObject(reply);
	return j;
}

json CRedis::evalscript(const string& name, const json& keys /* = nullptr */, const json& argvs /* = nullptr */)
{
	auto it = m_scriptLua.find(name);
	if (it == m_scriptLua.end())
	{
		WriteErrorLog("evalscript, no find name!");
		return __discard_json;
	}

	return evalsha(it->second, keys, argvs);
}

bool CRedis::select_db(int nDB)
{
	if (m_pRedis == NULL)
	{
		return -1;
	}

	redisReply *reply = (redisReply*)redisCommand(m_pRedis, "SELECT %d", nDB);
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0)
	{
		freeReplyObject(reply);
		return true;
	}
	else
	{
		freeReplyObject(reply);
		return false;
	}
}

bool CRedis::_connect(const char* ip, int port)
{
	redisContext *c = redisConnect(ip, port);
	if (c == NULL)
	{
		WriteErrorLog("_connect, Connection error: can't allocate redis context");
		return false;
	}
	else if (c->err != 0)
	{
		WriteErrorLog("_connect, Connection error: %s", c->errstr);
		redisFree(c);
		return false;
	}

	m_pRedis = c;
	return true;
}

void CRedis::_disconnect()
{
	if (m_pRedis == NULL)
	{
		return;
	}

	redisReply *reply = (redisReply*)redisCommand(m_pRedis, "QUIT");
	if (reply != NULL)
	{
		freeReplyObject(reply);
	}

	redisFree(m_pRedis);
	m_pRedis = NULL;
}


bool CRedis::_auth(const string& pwd)
{
	if (pwd.empty())
	{
		return true;
	}

	redisReply *reply = (redisReply*)redisCommand(m_pRedis, "AUTH %b", pwd.data(), pwd.size());
	if (reply == NULL)
	{
		redisFree(m_pRedis);
		m_pRedis = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0)
	{
		freeReplyObject(reply);
		return true;
	}
	else
	{
		freeReplyObject(reply);
		return false;
	}
}

bool CRedis::_check_connect()
{
	if (m_pRedis == NULL)
	{
		WriteNetwork("_check_connect, try to reconnect");

		if (!_connect(m_strIp.c_str(), m_nPort))
		{
			return false;
		}

		if (!_auth(m_strPasswd.c_str()))
		{
			WriteErrorLog("_check_connect, redis passwd not correct!");
			return false;
		}

		if (!select_db(m_nDB))
		{
			WriteErrorLog("_check_connect, select db failed!");
			return false;
		}
	}

	return true;
}
