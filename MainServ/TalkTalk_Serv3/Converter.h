#pragma once
#include <vector>
#include <locale.h>
#include <wchar.h>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <WinSock2.h> // WideCharToMultiByte()

class Converter
{
public:

	char* _WC2C(wchar_t* wstr);
	wchar_t* _C2WC(char* cstr);
	void Split_CStr(char* str, char* sArr[]);
	std::vector<char*> _WC2VC(wchar_t* wctr);
	wchar_t* _Str2WC(std::string str);
	std::vector<std::string> _Str_Split(std::string input, char delimiter);


	__int16 str_to_byte(wchar_t* msg, unsigned char* flag, unsigned char* opt);
	wchar_t* concat_header(char* ch);
	std::vector<std::string> _Wchar_To_VectorString(wchar_t* msg);
	std::vector<std::string> split2(char* str, std::string delimiter);
};

