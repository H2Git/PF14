#pragma once

#include "Converter.h"

#define HAVE_STRUCT_TIMESPEC
#pragma comment(lib, "libmysql.lib")

//#include <my_global.h>
#include <iostream>
#include <mysql.h>
#include <vector>
#include <mutex>
#include <string>

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PSWD "1234"
#define DB_NAME "test1"

class Database
{
private:

	MYSQL* DB_CONN = NULL, DB_REAL;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;
	char query[1024] = {'\0',};
	char str1[1024] = { '\0', }, str2[1024] = { '\0', };
	std::mutex dbmtx;

public:

	void ConnTest(); // DB 연결 확인용 함수

	int Connect();
	int Connect(const char* host, const char* user, const char* pswd, const char* name);
	void Disconnect();

	int _SaveSock(char* email, int sock);
	int RemoveSocket(uint32_t socket);
	int Reset_All_Socket();

	int Initialize_DB();

	int _Login(char* email, char* pw, int sock);

	int _CheckID(char* email);

	int _SaveOTP(char* email, uint32_t key);

	int _CheckOTP(char* email, char* OTP);

	int _CheckEmailName(char* email, char* Name);

	int _SavePW(char* email, char* pw);

	std::string _FindEmail(char *email);

	std::string _FindPW(char* email, char* pw, char* name);

	uint32_t _FindCode(std::vector<char*> vec);

	int _NoConn(char* email);

	std::string _MyProfile(char* email);

	int _SamePW(char* email, char* pw);

	int _SameName(char* email, char* name);

	int _Join(std::vector<char*> vec, int sock);

	int _Code(std::vector<char*> vec, int sock);

	int _NewTable(char* my_email);

	std::vector<std::string> _FrndProfile(char* my_email);
};
