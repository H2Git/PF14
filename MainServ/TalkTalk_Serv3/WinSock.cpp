#include "WinSock.h"

std::map<uint32_t, unsigned char*> WinSock::Initialize_FileBuf()
{
    std::map<uint32_t, unsigned char*> INI_MAP;

    unsigned char* ini2 = nullptr;
    uint32_t ini3 = 0;
    INI_MAP[ini3] = ini2;

    return INI_MAP;
}

SOCKET WinSock::ListenClients(std::string host, int port, int backlog) // 서버 연결 준비 함수
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

    return sock;  // (클라이언트와 연결 준비된) 서버 소켓을 반환한다.
}

void WinSock::Initialize_DB_Connection_History() // DB 접속기록 초기화용
{
    Database db; 
    if (db.Initialize_DB() == 0)
        printf("서버가 재시작됩니다. DB 모든 접속 기록을 초기화합니다.\n");
    else
        puts("DB Connect Initialize Error");
}

int WinSock::RunServer() // 전체적인 서버 기능 통합 실행 함수
{
    Initialize_DB_Connection_History(); // DB 접속기록 초기화용

    sock_serv = ListenClients(host, port, backlog);  // 메인 서버 socket 생성 Port 9090

    while (1)// 클라이언트 다중 연결
    {
        puts("클라이언트의 연결을 대기합니다.");

        sock_connected = NULL; // 클라이언트 연결마다 초기화
        sock_connected = accept(sock_serv, (SOCKADDR*)&addr, &len); // 새 클라이언트 연결 수락 

        WinSock thd; // 각각의 클라이언트가 스레드에서 독립적으로 송/수신 동작하도록 처리한다.
        clientlist.push_back(new std::thread(&WinSock::Clnt_, &thd, sock_connected, addr, &clientlist));

        // async 버전
        // v_async.push_back(std::async(std::launch::async, &WinSock::Client, sock_connected, clnt_Addr, &clientlist));
    }

    if (clientlist.size() > 0) // 스레드 joins
    {
        for (auto ptr = clientlist.begin(); ptr < clientlist.end(); ptr++)
        {
            (*ptr)->join();
        }
    }

    closesocket(sock_serv); // 서버 소켓 제거
    
    WSACleanup(); // wsa 종료

    return 0;
}

void WinSock::Clnt_(SOCKET clnt_sock, SOCKADDR_IN clnt_Addr, std::vector<std::thread*>* clnt_list)
{
    // 1. 클라이언트 연결마다 DB conn, File buffer 생성
    Database db; db.Connect(); // DB
    std::map<uint32_t, unsigned char*> file_buffer; // Buffer // 명시적 해제 필요

    std::cout // (info) 접속된 클라이언트 정보 출력
        << "\nConnected Client Socket Number = " << clnt_sock << ", IP:Port = " << inet_ntoa(clnt_Addr.sin_addr) << ":" << ntohs(clnt_Addr.sin_port) << std::endl;
    
    while (true)
    { 
        // 2. Recv Packet
        wchar_t recv_pkt[512] = { '\0', };  // 수신 1024 Byte (2byte X 512), 매번 초기화
        int recv_cnt = recv(clnt_sock, (char*)recv_pkt, 1024, 0);  printf("\n수신 %d Byte 바이너리.\n", recv_cnt);

        // 3. Connection Fail Exit
        if (recv_cnt < 1) 
            break;

        // 4. Control Workflow 
        int flag = 0, control_workflow = 1, lock[3] = { 1, }; // 최초(1) // 도중(2)
        std::memcpy(&flag, &recv_pkt[0], sizeof(int));
        std::chrono::system_clock::time_point timeout[3] = { std::chrono::system_clock::now(), }; // 타이머
        printf("  ===== [ flag : %d ] ===== \n", flag);

        // 5. 로그인 전 기능 
        if (0 != control_workflow) 
        {
            if (flag == 1 && lock[0] == 1) // 1) 로그인 요청이 처음이라면 (lock[0] == 1)
            {
                lock[0] = 2; // 2) lock[0]을 2로 변경해 로그인 시도중, 스레드 실행 중 상태로 변경하고
                timeout[0] = std::chrono::system_clock::now() + std::chrono::minutes(1); // 3)  1분 타이머를 세팅한다. 
            }
            else if (flag == 1 && lock[0] == 2) // 4) 그 사이, 다시 로그인 요청이 중복된다면,
            {
                if (timeout[0] < std::chrono::system_clock::now()) // 5) 1분이 지났다면, 위 과정을 다시 시작하고
                {
                    lock[0] = 2; 
                    timeout[0] = std::chrono::system_clock::now() + std::chrono::minutes(1);
                }
                else  // 6) 1분 미만이라면, 이메일 비번 확인만 처리해 이메일 확인을 유도하는 함수를 실행시킨다.
                    flag = 2; 
            }
            else if (flag == 3 && lock[0] == 2) // 7. OTP 값이 들어오고, 로그인 정보가 확인된 상태에서
            {
                Converter cnvter;
                std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]);

                if (0 == db._CheckOTP(vec[0], vec[1])) // 8.  OTP 값이 맞다면, 
                {
                    control_workflow = 0; // 9. 로그인 workflow 로 변경
                    lock[0] = 1; // 10. lock 상태 초기화 (로그아웃, 로그인시 필수)
                } // 11. OTP 값 일치, 로그인 성공 에코
                else
                    flag = 4; // 12.  OTP 값 불일치 에코
            }
            else if (flag == 5) // 자동 로그인
                control_workflow = 0;
            else if (flag == 6 && lock[1] == 1) // 비밀번호 찾기, 처리 프로세스
            {
                lock[1] = 2;
                timeout[1] = std::chrono::system_clock::now() + std::chrono::minutes(1);
            }
            else if (flag == 6 && lock[1] == 2) // 비밀번호 찾기, 중복 요청, 보낸 이메일 확인 요청
                flag = 7;
            else if (flag == 9 && lock[2] == 1) // 회원가입, 처리 프로세스
            {
                lock[2] = 2;
                timeout[1] = std::chrono::system_clock::now() + std::chrono::minutes(1);
            }
            else if (flag == 10 && lock[2] == 2) // 회원가입 이메일 확인 요청
                flag = 10;

            WinSock ws1;
            std::async(std::launch::async, &WinSock::Guest_Pkt_, &ws1, clnt_sock, flag, recv_pkt, &lock, &db, &file_buffer);
        }
        else if (0 == control_workflow) // 로그인 후 사용 가능기능 분류
        {
            WinSock ws2;
            std::async(std::launch::async, &WinSock::Clnt_Pkt_, &ws2, clnt_sock, flag, recv_pkt, &db, &file_buffer);
        }
    }

    std::cout // 접속 종료 작업 시작
        << "\ndisconnected Socket Number = " << clnt_sock << ", IP:Port = " << inet_ntoa(clnt_Addr.sin_addr) << ":" << ntohs(clnt_Addr.sin_port) << std::endl;
        
    uint32_t dbsock = (uint32_t)clnt_sock; 
    db.RemoveSocket(dbsock);      // 1. DB 접속 정보(소켓 고유값) 0으로 초기화
    db.Disconnect();              // 2. DB 연결을 해제 한다.
    closesocket(clnt_sock);       // 3. 연결된 클라이언트 소켓 해제한다.

    for (auto iter = file_buffer.begin(); iter != file_buffer.end(); iter++) // 4. 동적 할당된 맵의 버퍼 비우기 
        delete (*iter).second;
    file_buffer.clear(); // 키값도 제거

    for (auto ptr = clnt_list->begin(); ptr < clnt_list->end(); ptr++) // 5. 클라이언트 연결 해제 시, vector 컨테이너 해재
    {
        if ((*ptr)->get_id() == std::this_thread::get_id())// thread 아이디가 같은 것을 찾아서
        {
            clnt_list->erase(ptr); // threadlist에서 제거한다.
            break;
        }
    }
}

void WinSock::Guest_Pkt_(SOCKET clnt_sock, int flag, wchar_t* recv_pkt, int (*lock)[3], Database* db, std::map<uint32_t, unsigned char*>* f_buf) // 수신한 패킷을 스레드 처리한다.
{
    switch (flag)
    {

    case 1: // 로그인 최초 요청시 1분간 이메일 서버와 연결 프로세스 진행
        Guest_Pkt_Login_(clnt_sock, flag, recv_pkt, db);
        break;

    case 2: // 1분내 로그인 중복 요청시, 로그인 정보만 일치 여부만 에코
        Guest_Pkt_Login_(clnt_sock, flag, recv_pkt, db);
        break;

    case 3: // OTP 일치, 로그인 에코 (중복 요청의 경우의 수 없다.)
        Guest_Pkt_Echo_(clnt_sock, flag);
        break;

    case 4: // OTP 불일치 에코
        Guest_Pkt_Echo_(clnt_sock, flag);
        break;

    case 5: // 자동 로그인 성공 에코
        Guest_Pkt_Echo_(clnt_sock, flag);
        break;

    case 6: // 비밀번호 찾기 최초 요청시, 1분간 이메일 서버와 프로세스 진행
        Guest_Pkt_ChangePW_(clnt_sock, flag, recv_pkt, lock, db);
        break;

    case 7: // 1분내 비밀번호 찾기 중복 요청시, 수신 정보 일치 여부만 에코
        Guest_Pkt_ChangePW_(clnt_sock, flag, recv_pkt, lock, db);
        break;

    case 8: // 이메일 중복 확인 
        Guest_Pkt_CheckEmail_( clnt_sock,  recv_pkt,  db);
        break;

    case 9: // 회원가입
        Guest_Pkt_Join_(clnt_sock, recv_pkt, db);
        break;

    default:
        break;
    }
}

int WinSock::Clnt_Pkt_(SOCKET clnt_sock, int flag, wchar_t* recv_pkt, Database* db, std::map<uint32_t, unsigned char*>* f_buf) // 수신한 패킷을 스레드 처리한다.
{       
    switch (flag) // 2. Process Packet Request /w DB
    {

    case 13: // 파일 데이터 전송하라는 요청
        Clnt_Pkt_Send_FData(clnt_sock, recv_pkt, f_buf);
        break;

    default:
        break;
    }

    return 0;
}

int WinSock::Guest_Pkt_Login_(SOCKET clnt_sock, int flag, wchar_t* recv_pkt, Database* db) // 로그인 
{
    // 1. db에 email, pw, sock 전달, DB 확인 처리
    Converter cnvter; 
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]); // 전달받은 문자열 email`pw`name`word`null -> vector 변환
    int sock = (int)clnt_sock;// 계정 Online 식별용 고유값으로 사용
    int login_res = (*db)._Login(vec[0], vec[1], sock);   // 로그인 정보 db에서 확인

    // 2. 수신 로그인 정보가 맞으면
    if (login_res == 0) 
    {
        // 3. OTP 2차 인증 과정에서 사용될 키 DB 저장
        uint32_t otp_key; 
        std::memcpy(&otp_key, &recv_pkt[12], sizeof(uint32_t));
        int savekey_res = (*db)._SaveOTP(vec[0], otp_key);

        // 4. EMAIL 서버로 OTP 확인 QR 생성 및, 웹 호스팅 요청은 flag == 1 경우만, 중복 요청시 X
        if(flag == 1)
        {
            const wchar_t* req = (const wchar_t*)cnvter._Str2WC(std::string(vec[0]) + "`" + std::to_string(otp_key) + "`");
            _ReqOTPEmail(3, 0, 0, 0, req);
        }

        // 5. 클라이언트로 이메일과 비번 일치 및 이메일 확인 전달
        Guest_Pkt_Echo_(clnt_sock, 1);

        return 0;
    }
    else
    {
        // 6. 불일치 전달
        Guest_Pkt_Echo_(clnt_sock, 2);
        return -1;
    }
}

void WinSock::Guest_Pkt_Echo_(SOCKET clnt_sock, int flag)
{
    const wchar_t* msg = nullptr;

    if (flag == 1)
        msg = L"로그인 성공`이메일 OTP 확인`";
    if(flag == 2)
        msg = L"로그인 실패`이메일 비번 불일치`";
    else if (flag == 3)
        msg = L"로그인 성공`OTP 일치`";
    else if (flag == 4)
        msg = L"로그인 실패`OTP 불일치`";
    else if (flag == 5)
        msg = L"로그인 성공`자동 로그인`";
    else if (flag == 6)
        msg = L"비밀번호 찾기 성공`이메일 확인`";
    else if (flag == 7)
        msg = L"비밀번호 찾기 실패`이메일 이름 불일치`";

    Send_(clnt_sock, flag , 0, 0, 0, msg); // 실패 전송
}

int WinSock::Guest_Pkt_ChangePW_(SOCKET clnt_sock, int flag, wchar_t* recv_pkt, int(*lock)[3], Database* db)
{
    Converter cnvter;
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]);

    if ((*db)._CheckEmailName(vec[0], vec[2]) == 0)
    {
        // 4. EMAIL 서버로 OTP 확인 QR 생성 및, 웹 호스팅 요청
        const wchar_t* req = (const wchar_t*)cnvter._Str2WC(std::string(vec[0]) + "`");
        if (flag == 6)
        {
            std::string new_pw = _ReqPWEmail(6, 0, 0, 0, req);
            int pw_update = (*db)._SavePW(vec[0], (char*)new_pw.c_str());
            (*lock)[1] = 1; // 뮤텍스 고려
        }

        Guest_Pkt_Echo_(clnt_sock, 6);
        return 0;
    }
    else
    {
        Guest_Pkt_Echo_(clnt_sock, 7);
        return -1;
    }
}

void WinSock::Guest_Pkt_CheckEmail_(SOCKET clnt_sock, wchar_t* recv_pkt, Database* db)
{
    Converter cnvter;
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]);

    int email_duplicate = (*db)._CheckID(vec[0]); // email 찾기, 있으면 0

    const wchar_t* res = nullptr;

    if (email_duplicate == 0)
        res = L"아이디 중복`";
    else
        res = L"아이디 중복 없음`";

    Guest_Pkt_Echo_(clnt_sock, 8);
}

void WinSock::Guest_Pkt_Join_(SOCKET clnt_sock, wchar_t* recv_pkt, Database* db)
{
    const wchar_t* res = nullptr;
    int sock = (int)clnt_sock; // DB에 소켓을 숫자로 변환해 저장한다.

    Converter cnvter;
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]); // 전달받은 문자열

    if ((*db)._CheckID(vec[0]) != 0) // 등록된 회원 이메일이 없으면
        if ((*db)._Join(vec, sock) == 0) // 회원가입 성공시 0, 실패시 -1 반환
            if ((*db)._CheckID(vec[0]) == 0) // 회원가입 성공시, 아이디 찾기 0 반환
            {
                res = L"회원가입 성공`이메일 확인`";
                // 유저 전송

                std::string email(vec[0]);
                std::string name(vec[2]);
                const wchar_t* req_email = (const wchar_t*)cnvter._Str2WC(email + "`" + name);

                // 전송이 아니라 이메일 송신 요청, 락 걸기, 클라에게 이메일 전송 안내
                _ReqOTPEmail(9, 0, 0, 0, req_email);
            }
            else
                res = L"회원가입 실패`";
        else
            res = L"회원가입 실패`";
    else
        res = L"이메일 중복";

    Send_(clnt_sock, 5, 0, 0, 0, res);

}

void WinSock::Send_(SOCKET clnt_sock, int flag, int recv, int me, uint32_t key, const wchar_t* wc_ptr)
{
    Time t;
    uint32_t time = t.Get_HHmmssfff();
    int len = (int)(wcslen(wc_ptr)) * 2; // byte num = wchar len * 2
    wchar_t wc_arr[512] = { '\0', };

    std::memcpy(&wc_arr[0], &flag, 4); // #B1. 플레그
    std::memcpy(&wc_arr[2], &recv, 4); // #B2. 받는 사람
    std::memcpy(&wc_arr[4], &me, 4); // #B3. 보내는 사람 or 조각 수
    std::memcpy(&wc_arr[6], &key, 4); // #B4. key, 시간, 채팅방 번호
    std::memcpy(&wc_arr[8], wc_ptr, len); // #B5. 문장, 바이너리

    send(clnt_sock, (char*)wc_arr, 1024, 0); // size_t 크기변환 경고

#pragma region check

    printf("\n-------------- 클라이언트로 보낼 패킷 내용 확인 --------------\n");
    int reflag;
    int rerecv;
    int reme;
    uint32_t rekey;
    std::memcpy(&reflag, &wc_arr[0], 4);
    std::memcpy(&rerecv, &wc_arr[2], 4);
    std::memcpy(&reme, &wc_arr[4], 4);
    std::memcpy(&rekey, &wc_arr[6], 4); // 식별
    printf("[send_wc[00] :%d]\n", reflag);
    printf("[send_wc[02] :%d]\n", rerecv);
    printf("[send_wc[04] :%d]\n", reme);
    printf("[send_wc[06] :%d]\n", rekey); // 식별
    wprintf(L"[send_wc[08] :%s]\n", &wc_arr[8]); //문장은 그냥 출력 확인
    printf("-----------------------------------------------------------\n\n");

#pragma endregion

    return;
}

int WinSock::_ReqOTPEmail(int flag, int recver, int me, uint32_t key, const wchar_t* wc_ptr)
{
    int email_serv_port = 2323; // 이메일 서버 포트

    WSADATA wsaData;  // 소켓 정보 데이터 설정
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// 소켓 버전 명시.
        return -1;

    SOCKET email_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);    // Internet의 Stream 방식으로 소켓 생성

    SOCKADDR_IN email_adr;
    memset(&email_adr, 0, sizeof(email_adr)); // 주소 구조체 초기화
    email_adr.sin_family = AF_INET;  // 소켓은 Internet 타입
    email_adr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 127.0.0.1(localhost)로 접속하기 //inet_pton(AF_INET, host, &(email_adr.sin_addr.s_addr));
    email_adr.sin_port = htons(email_serv_port);// 포트 2323 으로 접속

    if (connect(email_sock, (SOCKADDR*)&email_adr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        puts("이메일 서버 연결 실패.");
        return -1;
    }

    Time t;
    uint32_t time = t.Get_HHmmssfff();
    int req_len = (int)(wcslen(wc_ptr)) * 2; // byte num = wchar len * 2
    wchar_t wc_arr[512] = { '\0', };

    std::memcpy(&wc_arr[0], &flag, 4); // #B1. 플레그
    std::memcpy(&wc_arr[2], &recver, 4); // #B2. 받는 사람
    std::memcpy(&wc_arr[4], &me, 4); // #B3. 보내는 사람 or 조각 수
    std::memcpy(&wc_arr[6], &key, 4); // #B4. key, 시간, 채팅방 번호
    std::memcpy(&wc_arr[8], wc_ptr, req_len); // #B5. 문장, 바이너리

    send(email_sock, (char*)wc_arr, 1024, 0); // size_t 크기변환 경고

#pragma region check

    printf("\n-------------- 이메일 서버로 보낸 패킷 내용 확인 --------------\n");
    int reflag;
    int rerecv;
    int reme;
    uint32_t rekey;
    std::memcpy(&reflag, &wc_arr[0], 4);
    std::memcpy(&rerecv, &wc_arr[2], 4);
    std::memcpy(&reme, &wc_arr[4], 4);
    std::memcpy(&rekey, &wc_arr[6], 4); // 식별
    printf("[send_wc[00] :%d]\n", reflag);
    printf("[send_wc[02] :%d]\n", rerecv);
    printf("[send_wc[04] :%d]\n", reme);
    printf("[send_wc[06] :%d]\n", rekey); // 식별
    wprintf(L"[send_wc[08] :%s]\n", &wc_arr[8]); //문장은 그냥 출력 확인
    printf("----------------------------------------------------\n\n");

#pragma endregion

    // 서버 소켓 종료
    closesocket(sock);
    // 소켓 종료
    WSACleanup();

    return 0;
}


std::string WinSock::_ReqPWEmail(int flag, int recver, int me, uint32_t key, const wchar_t* wc_ptr)
{
    int email_serv_port = 2323; // 이메일 서버 포트

    WSADATA wsaData;  // 소켓 정보 데이터 설정
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// 소켓 버전 명시.
        return "소캣 버전 설정 실패";

    SOCKET email_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);    // Internet의 Stream 방식으로 소켓 생성

    SOCKADDR_IN email_adr;
    memset(&email_adr, 0, sizeof(email_adr)); // 주소 구조체 초기화
    email_adr.sin_family = AF_INET;  // 소켓은 Internet 타입
    email_adr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 127.0.0.1(localhost)로 접속하기 //inet_pton(AF_INET, host, &(email_adr.sin_addr.s_addr));
    email_adr.sin_port = htons(email_serv_port);// 포트 2323 으로 접속

    if (connect(email_sock, (SOCKADDR*)&email_adr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        puts("이메일 서버 연결 실패.");
        return "이메일 서버 연결 실패.";
    }

    Time t;
    uint32_t time = t.Get_HHmmssfff();
    int req_len = (int)(wcslen(wc_ptr)) * 2; // byte num = wchar len * 2
    wchar_t wc_arr[512] = { '\0', };

    std::memcpy(&wc_arr[0], &flag, 4); // #B1. 플레그
    std::memcpy(&wc_arr[2], &recver, 4); // #B2. 받는 사람
    std::memcpy(&wc_arr[4], &me, 4); // #B3. 보내는 사람 or 조각 수
    std::memcpy(&wc_arr[6], &key, 4); // #B4. key, 시간, 채팅방 번호
    std::memcpy(&wc_arr[8], wc_ptr, req_len); // #B5. 문장, 바이너리

    send(email_sock, (char*)wc_arr, 1024, 0); // size_t 크기변환 경고

#pragma region check

    printf("\n-------------- 이메일 서버로 보낸 패킷 내용 확인 --------------\n");
    int reflag;
    int rerecv;
    int reme;
    uint32_t rekey;
    std::memcpy(&reflag, &wc_arr[0], 4);
    std::memcpy(&rerecv, &wc_arr[2], 4);
    std::memcpy(&reme, &wc_arr[4], 4);
    std::memcpy(&rekey, &wc_arr[6], 4); // 식별
    printf("[send_wc[00] :%d]\n", reflag);
    printf("[send_wc[02] :%d]\n", rerecv);
    printf("[send_wc[04] :%d]\n", reme);
    printf("[send_wc[06] :%d]\n", rekey); // 식별
    wprintf(L"[send_wc[08] :%s]\n", &wc_arr[8]); //문장은 그냥 출력 확인
    printf("----------------------------------------------------\n\n");

#pragma endregion

    wchar_t recv_pkt[512] = { '\0', };
    int recv_cnt = recv(email_sock, (char*)recv_pkt, 1024, 0);  printf("\n수신 %d Byte 바이너리.\n", recv_cnt);

    Converter cnvter; // 1. db에 email, pw, sock 전달, DB 확인 처리
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]); // 전달받은 문자열 email`pw`name`word`null -> vector 변환


    // 서버 소켓 종료
    closesocket(sock);
    // 소켓 종료
    WSACleanup();

    return vec[0];
}




void WinSock::Clnt_Pkt_Send_MyProfile(SOCKET clnt_sock, wchar_t* recv_pkt, Database* db, std::map<uint32_t, unsigned char*>* f_buf) // 내 프로필 전송
{
    Converter cnvter;
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]); // 전달받은 문자열
    std::vector<std::string> friend_list = (*db)._FrndProfile(vec[0]); // 친구 정보를 DB에서 가져온다.

    // 2. 클라이언트로 프로필 정보 전송
    std::string MYPROFILE = (*db)._MyProfile(vec[0]); // 내 프로필 정보를 가져와
    wchar_t* W_MYPROFILE = cnvter._Str2WC(MYPROFILE); // wchar로 변환해 
    Send_(clnt_sock, 2, 0, 0, 0, W_MYPROFILE); // 전송

    // 3 내 프로필 사진 전송
    Clnt_Pkt_PF_FReady(clnt_sock, vec[0], f_buf);
}

void WinSock::Clnt_Pkt_Send_FrndProfile(SOCKET clnt_sock, char* my_email, Database* db, std::map<uint32_t, unsigned char*>* f_buf) // 친구 프로필 전송
{
    Converter cnvter;
    //std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]); // 전달받은 문자열
    std::vector<std::string> friend_list = (*db)._FrndProfile(my_email); // 친구 정보를 DB에서 가져온다.

    for (std::string friend_info : friend_list) // 벡터에 저장된 친구 정보를 하나씩 가져온다.
    {
        wchar_t* friend_info_wchar = cnvter._Str2WC(friend_info); // char to wchar
        Send_(clnt_sock, 8, 0, 0, 0, friend_info_wchar);    // 친구 정보 전송

        char* context = NULL;
        char* friend_info_cstr = cnvter._WC2C(friend_info_wchar);
        char* friend_picture_name = strtok_s(friend_info_cstr, "`", &context); //해당 문자를 기준으로 문자열 자르기

        Clnt_Pkt_PF_FReady(clnt_sock, friend_picture_name, f_buf);
    }
}

void WinSock::Clnt_Pkt_PF_FReady(SOCKET clnt_sock, char* email, std::map<uint32_t, unsigned char*>* f_buf)
{
    char dir[1024] = "C:\\Users\\iot2122\\Downloads\\img\\"; // 1. 파일 준비
    strcat(dir, email);
    strcat(dir, ".png\0"); // 널 추가

    FILE* fp = fopen(dir, "rb"); // 2. 파일 가져오기
    
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END); // 3. 전달할 파일 정보 수집
        int f_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        int frag_1008byte_cnt = f_size / 1008; // 1008byte 나뉠 때, 조각 갯수 
        int total_frag_cnt = 0;
        if(f_size % 1008 != 0)
            total_frag_cnt = frag_1008byte_cnt + 1; // 전체 조각 갯수 
        uint32_t Last_frag_byte_cnt = f_size % 1008;
        int Last_frag_start_index = f_size - Last_frag_byte_cnt;
        unsigned char* data = new unsigned char[f_size]; // 4. 파일 데이터를 바이너리로 저장할 버퍼 준비

        fread(data, 1, f_size, fp); // 5. 전달할 파일 임시 버퍼에 저장

        fclose(fp); // 6. 바이너리로 저장되었기 때문에 파일 리소스는 닫는다.

        Time t; // 7. 파일 버퍼를 식별할 시시 분분 초초 밀리초초초
        uint32_t id = t.Get_HHmmssfff(); 

        (*f_buf).insert(std::pair<uint32_t, unsigned char*>(id,  data)); // 8. 파일 버퍼를 맵에 담아둔다. key:식별자 value:바이트배열

        _FReady(clnt_sock, 12, 0, total_frag_cnt, id, email); // 9. 클라이언트에 파일 정보를 보내서 공간을 준비시킨다.
    }
    else
        std::cout << "File open failed" << std::endl;
}

void WinSock::_FReady(SOCKET clnt_sock, int flag, int recv, int cnt, uint32_t id, char* file_name)
{
    wchar_t send_wchar[512] = { '\0', };
    Converter cnvter = Converter();
    wchar_t* f_name = cnvter._C2WC(file_name);

    std::memcpy(&send_wchar[0], &flag, 4); // 12번 파일 전송 준비
    std::memcpy(&send_wchar[2], &recv, 4); // 받는 사람 0 (서버에서 전송함으로)
    std::memcpy(&send_wchar[4], &cnt, 4);  // 파일 조각 개수
    std::memcpy(&send_wchar[6], &id, 4);   // 파일 버퍼 식별 아이디 (uint32_t)
    std::memcpy(&send_wchar[8], f_name, len); // 파일 이름 전송

    send(clnt_sock, (char*)send_wchar, 1024, 0); // 전송

    wprintf(L"친구 사진 파일 이름: \"%s\"\n", f_name);
    printf("조각 갯수:%u, 파일 식별 번호:%u\n", cnt, id);
}

void WinSock::Clnt_Pkt_Send_FData(SOCKET clnt_sock, wchar_t* recv_pkt, std::map<uint32_t, unsigned char*>* f_buf)
{
    int total_frag_cnt; // 1. Check recved packet Flag 
    uint32_t id;

    std::memcpy(&total_frag_cnt, &recv_pkt[4], sizeof(int));
    std::memcpy(&id, &recv_pkt[6], sizeof(uint32_t));

    wchar_t fdata[512] = { '\0', };
    
    int flag = 13, recver = 0;
    std::memcpy(&fdata[0], &flag, 4);
    std::memcpy(&fdata[2], &recver, 4);
    std::memcpy(&fdata[6], &id, 4);

    for (int i = 0; i < (total_frag_cnt-1); i++) // 마지막 조각 갯수를 제외, 전송한다.
    {
        std::memcpy(&fdata[4], &i, 4); // index 8번(3번째 부분) == 조각의 순서
        std::memcpy(&fdata[8], &((*f_buf)[id])[1008 * i], 1008);
        send(clnt_sock, (char*)&fdata, 1024, 0); // 파일 조각 전송
    }
    Clnt_Pkt_Send_FLast(clnt_sock, 14, id, f_buf);  // 마지막 조각 전송
}

void WinSock::Clnt_Pkt_Send_FLast(SOCKET clnt_sock, int f, uint32_t id, std::map<uint32_t, unsigned char*>* f_buf)
{
    int flag = f;
    //int recv = 0;
    int fsize = _msize((*f_buf)[id]);
    int last_cnt = fsize % 1008;
    int last_start = fsize - last_cnt;

    wchar_t flast[512] = { '\0', };

    std::memcpy(&flast[0], &flag, 4);
    std::memcpy(&flast[2], &fsize, 4);
    std::memcpy(&flast[4], &last_cnt, 4);
    std::memcpy(&flast[6], &id, 4);
    std::memcpy(&flast[8], &((*f_buf)[id])[last_start], last_cnt);

    send(clnt_sock, (char*)&flast, 1024, 0);
}
