#include <hiredis.h>
#include <iostream>

using namespace std;

bool dump_reply(redisReply* pReply);

int main()
{
//	redisContext* pRedis = redisConnect("192.168.5.111",6379);
	redisContext* pRedis = redisConnect("192.168.5.155",6379);
	if (pRedis == NULL || pRedis->err != 0)
	{
		cout << "connect error" << endl;
		return -1;
	}

	// ÊÚÈ¨

// 	string strArray[] = {
// 
// 	};
// 
// 	redisReply* pReply = (redisReply*)redisCommand(pRedis,"auth 7k7kqipai?linux");
// 	if (pReply == NULL)
// 	{
// 		cout << "connect appear failed" << endl;
// 		redisFree(pRedis);
// 		return -2;
// 	}
// 
// 	if (!dump_reply(pReply))
// 	{
// 		freeReplyObject(pReply);
// 		return -3;
// 	}
// 
// 	freeReplyObject(pReply);
// 	// string test
// 	pReply = (redisReply*)redisCommand(pRedis,"KEYS *");
// 	if (!)
// 
// 	// lset test

	while (true)
	{
		char szCommand[1024];
		cout << "please input yours command#: ";
		cin.getline(szCommand,1024);
		redisReply* pReply = (redisReply*)redisCommand(pRedis,szCommand);
		if (pReply == NULL)
		{
			cout << "connect appear failed" << endl;
			redisFree(pRedis);
			return -2;
		}
		
		dump_reply(pReply);
		freeReplyObject(pReply);
	}

	redisFree(pRedis);

	return 0;
}

bool dump_reply(redisReply* pReply)
{
	bool bResult = true;
	switch (pReply->type)
	{
	case REDIS_REPLY_ERROR:
		{
			cout << "REDIS_REPLY_ERROR:" << pReply->str << endl;
			bResult = false;
		}break;
	case REDIS_REPLY_NIL:
		{
			cout << "REDIS_REPLY_NIL:" << endl;
			bResult = false;
		}break;
	case REDIS_REPLY_STRING:
		{
			cout << "REDIS_REPLY_STRING:" << pReply->str << endl;
		}break;
	case REDIS_REPLY_STATUS:
		{
			cout << "REDIS_REPLY_STATUS:" << pReply->str << endl;
		}break;
	case REDIS_REPLY_INTEGER:
		{
			cout << "REDIS_REPLY_INTEGER:" << pReply->integer << endl;
		}break;
	case REDIS_REPLY_ARRAY:
		{
			cout << "REDIS_REPLY_ARRAY:" << pReply->elements << endl;
			for (size_t i = 0; i < pReply->elements; ++i)
			{
				dump_reply(pReply->element[i]);
			}
		}break;
	default:
		cout << pReply->type << " match noting" << endl;
		break;
	}
	return bResult;
}
