#include "WinSock.h"	

int main()
{
	// 서버 테스트 안내 (추후 제거)
	//for (size_t i = 0; i < 13; i++) printf("ㅡ");
	std::cout << "\n== server test start. ==\n" << std::endl;

	// wchar_t 한글 에러 방지 로케일 설정
	_wsetlocale(LC_ALL, L"korean");


	WinSock ws;
	ws.RunServer();

	return 0;


}