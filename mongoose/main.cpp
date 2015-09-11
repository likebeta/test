#include "httpClient.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
	json j = { {"name", "yisilong"} };
	httpClient http;
	json t = http.web_get(argv[1], j);
	if (t.is_object())
	{
		cout << t << endl;
	}

	t = http.web_post(argv[1], j);
	if (t.is_object())
	{
		cout << t << endl;
	}
	return 0;
}
