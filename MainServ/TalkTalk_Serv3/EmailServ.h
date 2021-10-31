#pragma once

// winsock 라이브러리 링크 (VS 설정 or vscode 작업시 유용)
#pragma comment(lib, "ws2_32")
#include <WinSock2.h> // winsock2 라이브러리
#include "Time.h"


class EmailServ
{

private:
	SOCKET sock_email_serv = NULL;
	SOCKET sock_email = NULL;
	std::string host = "127.0.0.1"; //INADDR_ANY
	int port = 2323;
	int backlog = 1;

	SOCKADDR_IN addr;
	SOCKET sock = NULL;

	int len = sizeof(SOCKADDR_IN);

public:
	// 서버로서 기능시 통합 실행 함수
	int Run_Email_Server();
	SOCKET ListenClients(std::string host, int port, int backlog); // 서버 연결 준비 함수

	// 클라이언트로서 전송시
	//int Send_ReqEmail(int flag, int recv, int me, uint32_t key, const wchar_t* wc_ptr);
};

