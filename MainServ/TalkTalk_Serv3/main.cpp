#include "WinSock.h"	

int main()
{
	// ���� �׽�Ʈ �ȳ� (���� ����)
	//for (size_t i = 0; i < 13; i++) printf("��");
	std::cout << "\n== server test start. ==\n" << std::endl;

	// wchar_t �ѱ� ���� ���� ������ ����
	_wsetlocale(LC_ALL, L"korean");


	WinSock ws;
	ws.RunServer();

	return 0;


}