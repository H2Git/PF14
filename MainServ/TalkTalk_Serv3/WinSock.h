#pragma once

// winsock ���̺귯�� ��ũ (VS ���� or vscode �۾��� ����)
#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h> // winsock2 ���̺귯��
#include "Time.h"

// #define _CRT_SECURE_NO_WARNINGS�� ���� 
// �������� �ʴ� �Լ� ���
// inet_ntoa�� ipv4 ����, inet_ntop IPv4 �� IPv6 �ּ� ���ڿ� ��ȯ
#pragma warning(disable:4996)

#include <vector>
#include <future>
#include <mutex>
#include <iostream>
#include <map>

#include "Database.h"
#include "Converter.h"


class WinSock
{

private:

	// �⺻ ���ᰪ
	std::string host = "127.0.0.1"; //INADDR_ANY
	int port = 9090;
	int backlog = 5;

	SOCKET sock_serv = NULL;
	SOCKET sock_connected = NULL;

	SOCKADDR_IN addr;
	SOCKET sock = NULL;

	std::mutex mtx;

	int len = sizeof(SOCKADDR_IN);

	// ������ ����,  client list ����
	std::vector<std::thread*> clientlist;

	// Async ����,  client list ����
	//std::vector<std::future<void>> v_async;

public:

	std::map<uint32_t, unsigned char*> Initialize_FileBuf();

	SOCKET ListenClients(std::string host, int port, int backlog);

	void Initialize_DB_Connection_History();

	int RunServer();

	void Clnt_(SOCKET clnet_sock, SOCKADDR_IN clientAddr, std::vector<std::thread*>* clientlist);

	void Guest_Pkt_(SOCKET clnet_sock, int flag, wchar_t* recv_packet, int (*lock)[3], Database* db, std::map<uint32_t, unsigned char*>* f_buf);
	
	int Guest_Pkt_Login_(SOCKET clnet_sock, int flag, wchar_t* recv_packet, Database* db);

	void Guest_Pkt_Echo_(SOCKET clnet_sock, int flag);

	int _ReqOTPEmail(int flag, int recv, int me, uint32_t key, const wchar_t* wc_ptr);

	int Guest_Pkt_ChangePW_(SOCKET clnt_sock, int flag, wchar_t* recv_packet, int(*lock)[3], Database* db);

	std::string _ReqPWEmail(int flag, int recver, int me, uint32_t key, const wchar_t* wc_ptr);

	void Guest_Pkt_CheckEmail_(SOCKET clientSock, wchar_t* recv_packet, Database* db);

	void Guest_Pkt_Join_(SOCKET clientSock, wchar_t* recv_packet, Database* db);

	void Clnt_Pkt_Send_MyProfile(SOCKET clnt_sock, wchar_t* recv_pkt, Database* db, std::map<uint32_t, unsigned char*>* f_buf);

	void Clnt_Pkt_Send_FrndProfile(SOCKET clnet_sock, char* my_email, Database* db, std::map<uint32_t, unsigned char*>* f_buf);

	int Clnt_Pkt_(SOCKET clnet_sock, int flag, wchar_t* recv_packet, Database* db, std::map<uint32_t, unsigned char*>* f_buf);

	void Clnt_Pkt_PF_FReady(SOCKET clientSock, char* res_str, std::map<uint32_t, unsigned char*>* f_buf);

	void _FReady(SOCKET clnet_sock, int flag, int recver, int total_frag_cnt, uint32_t file_id, char* file_name);

	void Clnt_Pkt_Send_FData(SOCKET clnet_sock, wchar_t* recv_packet, std::map<uint32_t, unsigned char*>* f_buf);
	
	void Clnt_Pkt_Send_FLast(SOCKET clnet_sock, int flag, uint32_t id, std::map<uint32_t, unsigned char*>* f_buf);

	void Send_(SOCKET clnet_sock, int flag, int recv, int me, uint32_t key, const wchar_t* wstr);

};