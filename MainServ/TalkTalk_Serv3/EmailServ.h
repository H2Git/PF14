#pragma once

// winsock ���̺귯�� ��ũ (VS ���� or vscode �۾��� ����)
#pragma comment(lib, "ws2_32")
#include <WinSock2.h> // winsock2 ���̺귯��
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
	// �����μ� ��ɽ� ���� ���� �Լ�
	int Run_Email_Server();
	SOCKET ListenClients(std::string host, int port, int backlog); // ���� ���� �غ� �Լ�

	// Ŭ���̾�Ʈ�μ� ���۽�
	//int Send_ReqEmail(int flag, int recv, int me, uint32_t key, const wchar_t* wc_ptr);
};

