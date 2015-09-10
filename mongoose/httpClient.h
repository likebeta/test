#pragma once

#include <string>
using std::string;

#include "json.hpp"
using nlohmann::json;

class httpClient
{
public:
	httpClient();
	~httpClient();
	void set_data(const string& response);
	void set_end();
public:
	json web_get(const string& url, const json& query = nullptr, int timeout = 3);
private:
	static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);
	json m_data;
	bool m_bEnd;
};
