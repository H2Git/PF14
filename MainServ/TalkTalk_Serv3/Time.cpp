#include "Time.h"


void Time::Print_ctime() 
{
	struct timeb msec;
	ftime(&msec);
	sprintf_s(ctime_char, 16, "%02d%02d%02d%02d%02d%02d%03d\0", year, month, day, hour, min, sec, msec.millitm);
	puts(ctime_char);

}

uint64_t Time::Get_yyMMddHHmmssfff()
{
	struct timeb msec;
	ftime(&msec);
	sprintf_s(ctime_char, 16, "%02d%02d%02d%02d%02d%02d%03d\0", year, month, day, hour, min, sec, msec.millitm); // null 필요
		
	uint64_t uint64_time = strtoull(ctime_char, NULL, 10); // 여기 null 필요

	return uint64_time;
}

uint32_t Time::Get_HHmmssfff()
{
	struct timeb msec;
	ftime(&msec);
	sprintf_s(ctime_char, 10, "%02d%02d%02d%03d\0", hour, min, sec, msec.millitm); // null 필요

	uint32_t uint32_time = strtoul(ctime_char, NULL, 10); // 여기 null 필요

	return uint32_time;
}