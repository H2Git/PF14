#pragma once

#include <cstdio>
#include <sys/timeb.h>
#include <ctime>
#include <iostream>

class Time
{

private:

	time_t c_tm = time(NULL);
	struct tm tm = *localtime(&c_tm); // localtime(&msec.time);


	int year = tm.tm_year-100;
	int month = tm.tm_mon;
	int day = tm.tm_mday;
	int hour = tm.tm_hour;
	int min = tm.tm_min;
	int sec = tm.tm_sec;

	char ctime_char[16] = { '\0', }; //yyMMddHHmmssff + null

public:
	void Print_ctime();
	uint64_t Get_yyMMddHHmmssfff();
	uint32_t Get_HHmmssfff();
};

