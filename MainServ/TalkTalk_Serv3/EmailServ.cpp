#include "EmailServ.h"



int EmailServ::Run_Email_Server() // ��ü���� ���� ��� ���� ���� �Լ�
{
    sock_email_serv = ListenClients(host, port, backlog); // ���� socket ����
    puts("�̸��� ���� ���� �غ�");
    sock_email = accept(sock_email_serv, (SOCKADDR*)&addr, &len); // �� Ŭ���̾�Ʈ ���� ���� 
    puts("�̸��� ���� ���� �Ϸ�");

    int cnt = 1;

    while (1)
    {
        puts("�̸��� ���� ��Ŷ ������\n"); // Ȯ�� ���� ���� ����������, �̸��� ���� ��û�� �ܹ��� ��û�̴�. ���� �ʿ�X, 
        wchar_t recv_packet[512] = { '\0', }; // ���� ��Ŷ 1024 Byte (2byte X 512), �Ź� �ʱ�ȭ
        cnt = recv(sock_email, (char*)recv_packet, 1024, 0); //���� ���ӿ� ���� ��
       
        if (cnt <= 0)
        {
            break;
        }
    }

    closesocket(sock_email_serv); // ���� ���� ����
    WSACleanup(); // wsa ����

    return 0;
}

SOCKET EmailServ::ListenClients(std::string host, int port, int backlog) // ���� ���� �غ� �Լ�
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // WSAS ������ �����Ѵ�.
        puts(" - WSAStartup error\n"); // �����ϸ�, ���� �ܼ� ���

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);// ���� ������ �����Ѵ�.
    memset(&addr, 0, sizeof(addr)); // ����ü �ʱ�ȭ
    addr.sin_family = AF_INET; // ������ Internet Ÿ��
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // ����local ����
    addr.sin_port = htons(port);// ���� ��Ʈ ���� 9090

    if (bind(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) // ���� ���Ͽ� ���ſ� �ʿ��� ������ ���ε��Ѵ�.
        puts(" - bind error\n"); // �����ϸ�, ���� ���


    if (listen(sock, backlog) == SOCKET_ERROR) // ���� ������ Ŭ���̾�Ʈ ���� ��û ����Ų��.
        puts(" - listen error\n"); // �����ϸ�, ���� ���

    return sock;  // Ŭ���̾�Ʈ�� ���� �غ�� ���� ������ ��ȯ�Ѵ�.
}


