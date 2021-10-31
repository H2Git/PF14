#include "EmailServ.h"



int EmailServ::Run_Email_Server() // 전체적인 서버 기능 통합 실행 함수
{
    sock_email_serv = ListenClients(host, port, backlog); // 서버 socket 생성
    puts("이메일 서버 연결 준비");
    sock_email = accept(sock_email_serv, (SOCKADDR*)&addr, &len); // 새 클라이언트 연결 수락 
    puts("이메일 서버 연결 완료");

    int cnt = 1;

    while (1)
    {
        puts("이메일 서버 패킷 수신중\n"); // 확인 에코 수신 가능하지만, 이메일 전송 요청은 단방향 요청이다. 구현 필요X, 
        wchar_t recv_packet[512] = { '\0', }; // 수신 패킷 1024 Byte (2byte X 512), 매번 초기화
        cnt = recv(sock_email, (char*)recv_packet, 1024, 0); //연결 지속용 수신 블럭
       
        if (cnt <= 0)
        {
            break;
        }
    }

    closesocket(sock_email_serv); // 서버 소켓 제거
    WSACleanup(); // wsa 종료

    return 0;
}

SOCKET EmailServ::ListenClients(std::string host, int port, int backlog) // 서버 연결 준비 함수
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // WSAS 버전을 설정한다.
        puts(" - WSAStartup error\n"); // 실패하면, 에러 콘솔 출력

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);// 서버 소켓을 생성한다.
    memset(&addr, 0, sizeof(addr)); // 구조체 초기화
    addr.sin_family = AF_INET; // 소켓은 Internet 타입
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 서버local 설정
    addr.sin_port = htons(port);// 서버 포트 설정 9090

    if (bind(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) // 서버 소켓에 수신에 필요한 정보를 바인딩한다.
        puts(" - bind error\n"); // 실패하면, 에러 출력


    if (listen(sock, backlog) == SOCKET_ERROR) // 서버 소켓을 클라이언트 연결 요청 대기시킨다.
        puts(" - listen error\n"); // 실패하면, 에러 출력

    return sock;  // 클라이언트와 연결 준비된 서버 소켓을 반환한다.
}


