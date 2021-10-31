#include "WinSock.h"

std::map<uint32_t, unsigned char*> WinSock::Initialize_FileBuf()
{
    std::map<uint32_t, unsigned char*> INI_MAP;

    unsigned char* ini2 = nullptr;
    uint32_t ini3 = 0;
    INI_MAP[ini3] = ini2;

    return INI_MAP;
}

SOCKET WinSock::ListenClients(std::string host, int port, int backlog) // ���� ���� �غ� �Լ�
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

    return sock;  // (Ŭ���̾�Ʈ�� ���� �غ��) ���� ������ ��ȯ�Ѵ�.
}

void WinSock::Initialize_DB_Connection_History() // DB ���ӱ�� �ʱ�ȭ��
{
    Database db; 
    if (db.Initialize_DB() == 0)
        printf("������ ����۵˴ϴ�. DB ��� ���� ����� �ʱ�ȭ�մϴ�.\n");
    else
        puts("DB Connect Initialize Error");
}

int WinSock::RunServer() // ��ü���� ���� ��� ���� ���� �Լ�
{
    Initialize_DB_Connection_History(); // DB ���ӱ�� �ʱ�ȭ��

    sock_serv = ListenClients(host, port, backlog);  // ���� ���� socket ���� Port 9090

    while (1)// Ŭ���̾�Ʈ ���� ����
    {
        puts("Ŭ���̾�Ʈ�� ������ ����մϴ�.");

        sock_connected = NULL; // Ŭ���̾�Ʈ ���Ḷ�� �ʱ�ȭ
        sock_connected = accept(sock_serv, (SOCKADDR*)&addr, &len); // �� Ŭ���̾�Ʈ ���� ���� 

        WinSock thd; // ������ Ŭ���̾�Ʈ�� �����忡�� ���������� ��/���� �����ϵ��� ó���Ѵ�.
        clientlist.push_back(new std::thread(&WinSock::Clnt_, &thd, sock_connected, addr, &clientlist));

        // async ����
        // v_async.push_back(std::async(std::launch::async, &WinSock::Client, sock_connected, clnt_Addr, &clientlist));
    }

    if (clientlist.size() > 0) // ������ joins
    {
        for (auto ptr = clientlist.begin(); ptr < clientlist.end(); ptr++)
        {
            (*ptr)->join();
        }
    }

    closesocket(sock_serv); // ���� ���� ����
    
    WSACleanup(); // wsa ����

    return 0;
}

void WinSock::Clnt_(SOCKET clnt_sock, SOCKADDR_IN clnt_Addr, std::vector<std::thread*>* clnt_list)
{
    // 1. Ŭ���̾�Ʈ ���Ḷ�� DB conn, File buffer ����
    Database db; db.Connect(); // DB
    std::map<uint32_t, unsigned char*> file_buffer; // Buffer // ����� ���� �ʿ�

    std::cout // (info) ���ӵ� Ŭ���̾�Ʈ ���� ���
        << "\nConnected Client Socket Number = " << clnt_sock << ", IP:Port = " << inet_ntoa(clnt_Addr.sin_addr) << ":" << ntohs(clnt_Addr.sin_port) << std::endl;
    
    while (true)
    { 
        // 2. Recv Packet
        wchar_t recv_pkt[512] = { '\0', };  // ���� 1024 Byte (2byte X 512), �Ź� �ʱ�ȭ
        int recv_cnt = recv(clnt_sock, (char*)recv_pkt, 1024, 0);  printf("\n���� %d Byte ���̳ʸ�.\n", recv_cnt);

        // 3. Connection Fail Exit
        if (recv_cnt < 1) 
            break;

        // 4. Control Workflow 
        int flag = 0, control_workflow = 1, lock[3] = { 1, }; // ����(1) // ����(2)
        std::memcpy(&flag, &recv_pkt[0], sizeof(int));
        std::chrono::system_clock::time_point timeout[3] = { std::chrono::system_clock::now(), }; // Ÿ�̸�
        printf("  ===== [ flag : %d ] ===== \n", flag);

        // 5. �α��� �� ��� 
        if (0 != control_workflow) 
        {
            if (flag == 1 && lock[0] == 1) // 1) �α��� ��û�� ó���̶�� (lock[0] == 1)
            {
                lock[0] = 2; // 2) lock[0]�� 2�� ������ �α��� �õ���, ������ ���� �� ���·� �����ϰ�
                timeout[0] = std::chrono::system_clock::now() + std::chrono::minutes(1); // 3)  1�� Ÿ�̸Ӹ� �����Ѵ�. 
            }
            else if (flag == 1 && lock[0] == 2) // 4) �� ����, �ٽ� �α��� ��û�� �ߺ��ȴٸ�,
            {
                if (timeout[0] < std::chrono::system_clock::now()) // 5) 1���� �����ٸ�, �� ������ �ٽ� �����ϰ�
                {
                    lock[0] = 2; 
                    timeout[0] = std::chrono::system_clock::now() + std::chrono::minutes(1);
                }
                else  // 6) 1�� �̸��̶��, �̸��� ��� Ȯ�θ� ó���� �̸��� Ȯ���� �����ϴ� �Լ��� �����Ų��.
                    flag = 2; 
            }
            else if (flag == 3 && lock[0] == 2) // 7. OTP ���� ������, �α��� ������ Ȯ�ε� ���¿���
            {
                Converter cnvter;
                std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]);

                if (0 == db._CheckOTP(vec[0], vec[1])) // 8.  OTP ���� �´ٸ�, 
                {
                    control_workflow = 0; // 9. �α��� workflow �� ����
                    lock[0] = 1; // 10. lock ���� �ʱ�ȭ (�α׾ƿ�, �α��ν� �ʼ�)
                } // 11. OTP �� ��ġ, �α��� ���� ����
                else
                    flag = 4; // 12.  OTP �� ����ġ ����
            }
            else if (flag == 5) // �ڵ� �α���
                control_workflow = 0;
            else if (flag == 6 && lock[1] == 1) // ��й�ȣ ã��, ó�� ���μ���
            {
                lock[1] = 2;
                timeout[1] = std::chrono::system_clock::now() + std::chrono::minutes(1);
            }
            else if (flag == 6 && lock[1] == 2) // ��й�ȣ ã��, �ߺ� ��û, ���� �̸��� Ȯ�� ��û
                flag = 7;
            else if (flag == 9 && lock[2] == 1) // ȸ������, ó�� ���μ���
            {
                lock[2] = 2;
                timeout[1] = std::chrono::system_clock::now() + std::chrono::minutes(1);
            }
            else if (flag == 10 && lock[2] == 2) // ȸ������ �̸��� Ȯ�� ��û
                flag = 10;

            WinSock ws1;
            std::async(std::launch::async, &WinSock::Guest_Pkt_, &ws1, clnt_sock, flag, recv_pkt, &lock, &db, &file_buffer);
        }
        else if (0 == control_workflow) // �α��� �� ��� ���ɱ�� �з�
        {
            WinSock ws2;
            std::async(std::launch::async, &WinSock::Clnt_Pkt_, &ws2, clnt_sock, flag, recv_pkt, &db, &file_buffer);
        }
    }

    std::cout // ���� ���� �۾� ����
        << "\ndisconnected Socket Number = " << clnt_sock << ", IP:Port = " << inet_ntoa(clnt_Addr.sin_addr) << ":" << ntohs(clnt_Addr.sin_port) << std::endl;
        
    uint32_t dbsock = (uint32_t)clnt_sock; 
    db.RemoveSocket(dbsock);      // 1. DB ���� ����(���� ������) 0���� �ʱ�ȭ
    db.Disconnect();              // 2. DB ������ ���� �Ѵ�.
    closesocket(clnt_sock);       // 3. ����� Ŭ���̾�Ʈ ���� �����Ѵ�.

    for (auto iter = file_buffer.begin(); iter != file_buffer.end(); iter++) // 4. ���� �Ҵ�� ���� ���� ���� 
        delete (*iter).second;
    file_buffer.clear(); // Ű���� ����

    for (auto ptr = clnt_list->begin(); ptr < clnt_list->end(); ptr++) // 5. Ŭ���̾�Ʈ ���� ���� ��, vector �����̳� ����
    {
        if ((*ptr)->get_id() == std::this_thread::get_id())// thread ���̵� ���� ���� ã�Ƽ�
        {
            clnt_list->erase(ptr); // threadlist���� �����Ѵ�.
            break;
        }
    }
}

void WinSock::Guest_Pkt_(SOCKET clnt_sock, int flag, wchar_t* recv_pkt, int (*lock)[3], Database* db, std::map<uint32_t, unsigned char*>* f_buf) // ������ ��Ŷ�� ������ ó���Ѵ�.
{
    switch (flag)
    {

    case 1: // �α��� ���� ��û�� 1�а� �̸��� ������ ���� ���μ��� ����
        Guest_Pkt_Login_(clnt_sock, flag, recv_pkt, db);
        break;

    case 2: // 1�г� �α��� �ߺ� ��û��, �α��� ������ ��ġ ���θ� ����
        Guest_Pkt_Login_(clnt_sock, flag, recv_pkt, db);
        break;

    case 3: // OTP ��ġ, �α��� ���� (�ߺ� ��û�� ����� �� ����.)
        Guest_Pkt_Echo_(clnt_sock, flag);
        break;

    case 4: // OTP ����ġ ����
        Guest_Pkt_Echo_(clnt_sock, flag);
        break;

    case 5: // �ڵ� �α��� ���� ����
        Guest_Pkt_Echo_(clnt_sock, flag);
        break;

    case 6: // ��й�ȣ ã�� ���� ��û��, 1�а� �̸��� ������ ���μ��� ����
        Guest_Pkt_ChangePW_(clnt_sock, flag, recv_pkt, lock, db);
        break;

    case 7: // 1�г� ��й�ȣ ã�� �ߺ� ��û��, ���� ���� ��ġ ���θ� ����
        Guest_Pkt_ChangePW_(clnt_sock, flag, recv_pkt, lock, db);
        break;

    case 8: // �̸��� �ߺ� Ȯ�� 
        Guest_Pkt_CheckEmail_( clnt_sock,  recv_pkt,  db);
        break;

    case 9: // ȸ������
        Guest_Pkt_Join_(clnt_sock, recv_pkt, db);
        break;

    default:
        break;
    }
}

int WinSock::Clnt_Pkt_(SOCKET clnt_sock, int flag, wchar_t* recv_pkt, Database* db, std::map<uint32_t, unsigned char*>* f_buf) // ������ ��Ŷ�� ������ ó���Ѵ�.
{       
    switch (flag) // 2. Process Packet Request /w DB
    {

    case 13: // ���� ������ �����϶�� ��û
        Clnt_Pkt_Send_FData(clnt_sock, recv_pkt, f_buf);
        break;

    default:
        break;
    }

    return 0;
}

int WinSock::Guest_Pkt_Login_(SOCKET clnt_sock, int flag, wchar_t* recv_pkt, Database* db) // �α��� 
{
    // 1. db�� email, pw, sock ����, DB Ȯ�� ó��
    Converter cnvter; 
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]); // ���޹��� ���ڿ� email`pw`name`word`null -> vector ��ȯ
    int sock = (int)clnt_sock;// ���� Online �ĺ��� ���������� ���
    int login_res = (*db)._Login(vec[0], vec[1], sock);   // �α��� ���� db���� Ȯ��

    // 2. ���� �α��� ������ ������
    if (login_res == 0) 
    {
        // 3. OTP 2�� ���� �������� ���� Ű DB ����
        uint32_t otp_key; 
        std::memcpy(&otp_key, &recv_pkt[12], sizeof(uint32_t));
        int savekey_res = (*db)._SaveOTP(vec[0], otp_key);

        // 4. EMAIL ������ OTP Ȯ�� QR ���� ��, �� ȣ���� ��û�� flag == 1 ��츸, �ߺ� ��û�� X
        if(flag == 1)
        {
            const wchar_t* req = (const wchar_t*)cnvter._Str2WC(std::string(vec[0]) + "`" + std::to_string(otp_key) + "`");
            _ReqOTPEmail(3, 0, 0, 0, req);
        }

        // 5. Ŭ���̾�Ʈ�� �̸��ϰ� ��� ��ġ �� �̸��� Ȯ�� ����
        Guest_Pkt_Echo_(clnt_sock, 1);

        return 0;
    }
    else
    {
        // 6. ����ġ ����
        Guest_Pkt_Echo_(clnt_sock, 2);
        return -1;
    }
}

void WinSock::Guest_Pkt_Echo_(SOCKET clnt_sock, int flag)
{
    const wchar_t* msg = nullptr;

    if (flag == 1)
        msg = L"�α��� ����`�̸��� OTP Ȯ��`";
    if(flag == 2)
        msg = L"�α��� ����`�̸��� ��� ����ġ`";
    else if (flag == 3)
        msg = L"�α��� ����`OTP ��ġ`";
    else if (flag == 4)
        msg = L"�α��� ����`OTP ����ġ`";
    else if (flag == 5)
        msg = L"�α��� ����`�ڵ� �α���`";
    else if (flag == 6)
        msg = L"��й�ȣ ã�� ����`�̸��� Ȯ��`";
    else if (flag == 7)
        msg = L"��й�ȣ ã�� ����`�̸��� �̸� ����ġ`";

    Send_(clnt_sock, flag , 0, 0, 0, msg); // ���� ����
}

int WinSock::Guest_Pkt_ChangePW_(SOCKET clnt_sock, int flag, wchar_t* recv_pkt, int(*lock)[3], Database* db)
{
    Converter cnvter;
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]);

    if ((*db)._CheckEmailName(vec[0], vec[2]) == 0)
    {
        // 4. EMAIL ������ OTP Ȯ�� QR ���� ��, �� ȣ���� ��û
        const wchar_t* req = (const wchar_t*)cnvter._Str2WC(std::string(vec[0]) + "`");
        if (flag == 6)
        {
            std::string new_pw = _ReqPWEmail(6, 0, 0, 0, req);
            int pw_update = (*db)._SavePW(vec[0], (char*)new_pw.c_str());
            (*lock)[1] = 1; // ���ؽ� ���
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

    int email_duplicate = (*db)._CheckID(vec[0]); // email ã��, ������ 0

    const wchar_t* res = nullptr;

    if (email_duplicate == 0)
        res = L"���̵� �ߺ�`";
    else
        res = L"���̵� �ߺ� ����`";

    Guest_Pkt_Echo_(clnt_sock, 8);
}

void WinSock::Guest_Pkt_Join_(SOCKET clnt_sock, wchar_t* recv_pkt, Database* db)
{
    const wchar_t* res = nullptr;
    int sock = (int)clnt_sock; // DB�� ������ ���ڷ� ��ȯ�� �����Ѵ�.

    Converter cnvter;
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]); // ���޹��� ���ڿ�

    if ((*db)._CheckID(vec[0]) != 0) // ��ϵ� ȸ�� �̸����� ������
        if ((*db)._Join(vec, sock) == 0) // ȸ������ ������ 0, ���н� -1 ��ȯ
            if ((*db)._CheckID(vec[0]) == 0) // ȸ������ ������, ���̵� ã�� 0 ��ȯ
            {
                res = L"ȸ������ ����`�̸��� Ȯ��`";
                // ���� ����

                std::string email(vec[0]);
                std::string name(vec[2]);
                const wchar_t* req_email = (const wchar_t*)cnvter._Str2WC(email + "`" + name);

                // ������ �ƴ϶� �̸��� �۽� ��û, �� �ɱ�, Ŭ�󿡰� �̸��� ���� �ȳ�
                _ReqOTPEmail(9, 0, 0, 0, req_email);
            }
            else
                res = L"ȸ������ ����`";
        else
            res = L"ȸ������ ����`";
    else
        res = L"�̸��� �ߺ�";

    Send_(clnt_sock, 5, 0, 0, 0, res);

}

void WinSock::Send_(SOCKET clnt_sock, int flag, int recv, int me, uint32_t key, const wchar_t* wc_ptr)
{
    Time t;
    uint32_t time = t.Get_HHmmssfff();
    int len = (int)(wcslen(wc_ptr)) * 2; // byte num = wchar len * 2
    wchar_t wc_arr[512] = { '\0', };

    std::memcpy(&wc_arr[0], &flag, 4); // #B1. �÷���
    std::memcpy(&wc_arr[2], &recv, 4); // #B2. �޴� ���
    std::memcpy(&wc_arr[4], &me, 4); // #B3. ������ ��� or ���� ��
    std::memcpy(&wc_arr[6], &key, 4); // #B4. key, �ð�, ä�ù� ��ȣ
    std::memcpy(&wc_arr[8], wc_ptr, len); // #B5. ����, ���̳ʸ�

    send(clnt_sock, (char*)wc_arr, 1024, 0); // size_t ũ�⺯ȯ ���

#pragma region check

    printf("\n-------------- Ŭ���̾�Ʈ�� ���� ��Ŷ ���� Ȯ�� --------------\n");
    int reflag;
    int rerecv;
    int reme;
    uint32_t rekey;
    std::memcpy(&reflag, &wc_arr[0], 4);
    std::memcpy(&rerecv, &wc_arr[2], 4);
    std::memcpy(&reme, &wc_arr[4], 4);
    std::memcpy(&rekey, &wc_arr[6], 4); // �ĺ�
    printf("[send_wc[00] :%d]\n", reflag);
    printf("[send_wc[02] :%d]\n", rerecv);
    printf("[send_wc[04] :%d]\n", reme);
    printf("[send_wc[06] :%d]\n", rekey); // �ĺ�
    wprintf(L"[send_wc[08] :%s]\n", &wc_arr[8]); //������ �׳� ��� Ȯ��
    printf("-----------------------------------------------------------\n\n");

#pragma endregion

    return;
}

int WinSock::_ReqOTPEmail(int flag, int recver, int me, uint32_t key, const wchar_t* wc_ptr)
{
    int email_serv_port = 2323; // �̸��� ���� ��Ʈ

    WSADATA wsaData;  // ���� ���� ������ ����
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// ���� ���� ���.
        return -1;

    SOCKET email_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);    // Internet�� Stream ������� ���� ����

    SOCKADDR_IN email_adr;
    memset(&email_adr, 0, sizeof(email_adr)); // �ּ� ����ü �ʱ�ȭ
    email_adr.sin_family = AF_INET;  // ������ Internet Ÿ��
    email_adr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 127.0.0.1(localhost)�� �����ϱ� //inet_pton(AF_INET, host, &(email_adr.sin_addr.s_addr));
    email_adr.sin_port = htons(email_serv_port);// ��Ʈ 2323 ���� ����

    if (connect(email_sock, (SOCKADDR*)&email_adr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        puts("�̸��� ���� ���� ����.");
        return -1;
    }

    Time t;
    uint32_t time = t.Get_HHmmssfff();
    int req_len = (int)(wcslen(wc_ptr)) * 2; // byte num = wchar len * 2
    wchar_t wc_arr[512] = { '\0', };

    std::memcpy(&wc_arr[0], &flag, 4); // #B1. �÷���
    std::memcpy(&wc_arr[2], &recver, 4); // #B2. �޴� ���
    std::memcpy(&wc_arr[4], &me, 4); // #B3. ������ ��� or ���� ��
    std::memcpy(&wc_arr[6], &key, 4); // #B4. key, �ð�, ä�ù� ��ȣ
    std::memcpy(&wc_arr[8], wc_ptr, req_len); // #B5. ����, ���̳ʸ�

    send(email_sock, (char*)wc_arr, 1024, 0); // size_t ũ�⺯ȯ ���

#pragma region check

    printf("\n-------------- �̸��� ������ ���� ��Ŷ ���� Ȯ�� --------------\n");
    int reflag;
    int rerecv;
    int reme;
    uint32_t rekey;
    std::memcpy(&reflag, &wc_arr[0], 4);
    std::memcpy(&rerecv, &wc_arr[2], 4);
    std::memcpy(&reme, &wc_arr[4], 4);
    std::memcpy(&rekey, &wc_arr[6], 4); // �ĺ�
    printf("[send_wc[00] :%d]\n", reflag);
    printf("[send_wc[02] :%d]\n", rerecv);
    printf("[send_wc[04] :%d]\n", reme);
    printf("[send_wc[06] :%d]\n", rekey); // �ĺ�
    wprintf(L"[send_wc[08] :%s]\n", &wc_arr[8]); //������ �׳� ��� Ȯ��
    printf("----------------------------------------------------\n\n");

#pragma endregion

    // ���� ���� ����
    closesocket(sock);
    // ���� ����
    WSACleanup();

    return 0;
}


std::string WinSock::_ReqPWEmail(int flag, int recver, int me, uint32_t key, const wchar_t* wc_ptr)
{
    int email_serv_port = 2323; // �̸��� ���� ��Ʈ

    WSADATA wsaData;  // ���� ���� ������ ����
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// ���� ���� ���.
        return "��Ĺ ���� ���� ����";

    SOCKET email_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);    // Internet�� Stream ������� ���� ����

    SOCKADDR_IN email_adr;
    memset(&email_adr, 0, sizeof(email_adr)); // �ּ� ����ü �ʱ�ȭ
    email_adr.sin_family = AF_INET;  // ������ Internet Ÿ��
    email_adr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 127.0.0.1(localhost)�� �����ϱ� //inet_pton(AF_INET, host, &(email_adr.sin_addr.s_addr));
    email_adr.sin_port = htons(email_serv_port);// ��Ʈ 2323 ���� ����

    if (connect(email_sock, (SOCKADDR*)&email_adr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        puts("�̸��� ���� ���� ����.");
        return "�̸��� ���� ���� ����.";
    }

    Time t;
    uint32_t time = t.Get_HHmmssfff();
    int req_len = (int)(wcslen(wc_ptr)) * 2; // byte num = wchar len * 2
    wchar_t wc_arr[512] = { '\0', };

    std::memcpy(&wc_arr[0], &flag, 4); // #B1. �÷���
    std::memcpy(&wc_arr[2], &recver, 4); // #B2. �޴� ���
    std::memcpy(&wc_arr[4], &me, 4); // #B3. ������ ��� or ���� ��
    std::memcpy(&wc_arr[6], &key, 4); // #B4. key, �ð�, ä�ù� ��ȣ
    std::memcpy(&wc_arr[8], wc_ptr, req_len); // #B5. ����, ���̳ʸ�

    send(email_sock, (char*)wc_arr, 1024, 0); // size_t ũ�⺯ȯ ���

#pragma region check

    printf("\n-------------- �̸��� ������ ���� ��Ŷ ���� Ȯ�� --------------\n");
    int reflag;
    int rerecv;
    int reme;
    uint32_t rekey;
    std::memcpy(&reflag, &wc_arr[0], 4);
    std::memcpy(&rerecv, &wc_arr[2], 4);
    std::memcpy(&reme, &wc_arr[4], 4);
    std::memcpy(&rekey, &wc_arr[6], 4); // �ĺ�
    printf("[send_wc[00] :%d]\n", reflag);
    printf("[send_wc[02] :%d]\n", rerecv);
    printf("[send_wc[04] :%d]\n", reme);
    printf("[send_wc[06] :%d]\n", rekey); // �ĺ�
    wprintf(L"[send_wc[08] :%s]\n", &wc_arr[8]); //������ �׳� ��� Ȯ��
    printf("----------------------------------------------------\n\n");

#pragma endregion

    wchar_t recv_pkt[512] = { '\0', };
    int recv_cnt = recv(email_sock, (char*)recv_pkt, 1024, 0);  printf("\n���� %d Byte ���̳ʸ�.\n", recv_cnt);

    Converter cnvter; // 1. db�� email, pw, sock ����, DB Ȯ�� ó��
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]); // ���޹��� ���ڿ� email`pw`name`word`null -> vector ��ȯ


    // ���� ���� ����
    closesocket(sock);
    // ���� ����
    WSACleanup();

    return vec[0];
}




void WinSock::Clnt_Pkt_Send_MyProfile(SOCKET clnt_sock, wchar_t* recv_pkt, Database* db, std::map<uint32_t, unsigned char*>* f_buf) // �� ������ ����
{
    Converter cnvter;
    std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]); // ���޹��� ���ڿ�
    std::vector<std::string> friend_list = (*db)._FrndProfile(vec[0]); // ģ�� ������ DB���� �����´�.

    // 2. Ŭ���̾�Ʈ�� ������ ���� ����
    std::string MYPROFILE = (*db)._MyProfile(vec[0]); // �� ������ ������ ������
    wchar_t* W_MYPROFILE = cnvter._Str2WC(MYPROFILE); // wchar�� ��ȯ�� 
    Send_(clnt_sock, 2, 0, 0, 0, W_MYPROFILE); // ����

    // 3 �� ������ ���� ����
    Clnt_Pkt_PF_FReady(clnt_sock, vec[0], f_buf);
}

void WinSock::Clnt_Pkt_Send_FrndProfile(SOCKET clnt_sock, char* my_email, Database* db, std::map<uint32_t, unsigned char*>* f_buf) // ģ�� ������ ����
{
    Converter cnvter;
    //std::vector<char*> vec = cnvter._WC2VC(&recv_pkt[8]); // ���޹��� ���ڿ�
    std::vector<std::string> friend_list = (*db)._FrndProfile(my_email); // ģ�� ������ DB���� �����´�.

    for (std::string friend_info : friend_list) // ���Ϳ� ����� ģ�� ������ �ϳ��� �����´�.
    {
        wchar_t* friend_info_wchar = cnvter._Str2WC(friend_info); // char to wchar
        Send_(clnt_sock, 8, 0, 0, 0, friend_info_wchar);    // ģ�� ���� ����

        char* context = NULL;
        char* friend_info_cstr = cnvter._WC2C(friend_info_wchar);
        char* friend_picture_name = strtok_s(friend_info_cstr, "`", &context); //�ش� ���ڸ� �������� ���ڿ� �ڸ���

        Clnt_Pkt_PF_FReady(clnt_sock, friend_picture_name, f_buf);
    }
}

void WinSock::Clnt_Pkt_PF_FReady(SOCKET clnt_sock, char* email, std::map<uint32_t, unsigned char*>* f_buf)
{
    char dir[1024] = "C:\\Users\\iot2122\\Downloads\\img\\"; // 1. ���� �غ�
    strcat(dir, email);
    strcat(dir, ".png\0"); // �� �߰�

    FILE* fp = fopen(dir, "rb"); // 2. ���� ��������
    
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END); // 3. ������ ���� ���� ����
        int f_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        int frag_1008byte_cnt = f_size / 1008; // 1008byte ���� ��, ���� ���� 
        int total_frag_cnt = 0;
        if(f_size % 1008 != 0)
            total_frag_cnt = frag_1008byte_cnt + 1; // ��ü ���� ���� 
        uint32_t Last_frag_byte_cnt = f_size % 1008;
        int Last_frag_start_index = f_size - Last_frag_byte_cnt;
        unsigned char* data = new unsigned char[f_size]; // 4. ���� �����͸� ���̳ʸ��� ������ ���� �غ�

        fread(data, 1, f_size, fp); // 5. ������ ���� �ӽ� ���ۿ� ����

        fclose(fp); // 6. ���̳ʸ��� ����Ǿ��� ������ ���� ���ҽ��� �ݴ´�.

        Time t; // 7. ���� ���۸� �ĺ��� �ý� �к� ���� �и�������
        uint32_t id = t.Get_HHmmssfff(); 

        (*f_buf).insert(std::pair<uint32_t, unsigned char*>(id,  data)); // 8. ���� ���۸� �ʿ� ��Ƶд�. key:�ĺ��� value:����Ʈ�迭

        _FReady(clnt_sock, 12, 0, total_frag_cnt, id, email); // 9. Ŭ���̾�Ʈ�� ���� ������ ������ ������ �غ��Ų��.
    }
    else
        std::cout << "File open failed" << std::endl;
}

void WinSock::_FReady(SOCKET clnt_sock, int flag, int recv, int cnt, uint32_t id, char* file_name)
{
    wchar_t send_wchar[512] = { '\0', };
    Converter cnvter = Converter();
    wchar_t* f_name = cnvter._C2WC(file_name);

    std::memcpy(&send_wchar[0], &flag, 4); // 12�� ���� ���� �غ�
    std::memcpy(&send_wchar[2], &recv, 4); // �޴� ��� 0 (�������� ����������)
    std::memcpy(&send_wchar[4], &cnt, 4);  // ���� ���� ����
    std::memcpy(&send_wchar[6], &id, 4);   // ���� ���� �ĺ� ���̵� (uint32_t)
    std::memcpy(&send_wchar[8], f_name, len); // ���� �̸� ����

    send(clnt_sock, (char*)send_wchar, 1024, 0); // ����

    wprintf(L"ģ�� ���� ���� �̸�: \"%s\"\n", f_name);
    printf("���� ����:%u, ���� �ĺ� ��ȣ:%u\n", cnt, id);
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

    for (int i = 0; i < (total_frag_cnt-1); i++) // ������ ���� ������ ����, �����Ѵ�.
    {
        std::memcpy(&fdata[4], &i, 4); // index 8��(3��° �κ�) == ������ ����
        std::memcpy(&fdata[8], &((*f_buf)[id])[1008 * i], 1008);
        send(clnt_sock, (char*)&fdata, 1024, 0); // ���� ���� ����
    }
    Clnt_Pkt_Send_FLast(clnt_sock, 14, id, f_buf);  // ������ ���� ����
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
