#include "Database.h"

int Database::Connect()
{
    mysql_init(&DB_REAL);

    DB_CONN = mysql_real_connect(&DB_REAL, DB_HOST, DB_USER, DB_PSWD, DB_NAME, 3306, (char*)NULL, 0);
    mysql_query(DB_CONN, "set session character_set_connection=euckr;");
    mysql_query(DB_CONN, "set session character_set_results=euckr;"); // 한글 가져올때
    mysql_query(DB_CONN, "set session character_set_client=euckr;");  // 한글 저장할때

    if (DB_CONN != NULL)
        return 0;
    else
        return -1;
}

int Database::Connect(const char* host, const char* user, const char* pswd, const char* name)
{
    mysql_init(&DB_REAL);

    DB_CONN = mysql_real_connect(&DB_REAL, host, user, pswd, name, 3306, (char*)NULL, 0);
    mysql_query(DB_CONN, "set session character_set_connection=euckr;");
    mysql_query(DB_CONN, "set session character_set_results=euckr;"); // 한글 가져올때
    mysql_query(DB_CONN, "set session character_set_client=euckr;");  // 한글 저장할때

    if (DB_CONN != NULL)
        return 0;
    else
        return -1;
}
void Database::Disconnect()
{
    mysql_close(DB_CONN);
}

int Database::RemoveSocket(uint32_t sock)
{
    printf("remove taget sock %d\n", sock);
    snprintf(query, sizeof(query), "UPDATE test1.`mlist` SET sock=0 WHERE sock=%d", sock);

    if (0 == mysql_query(DB_CONN, query))
        return 0;
    else
        return -1;
}

int Database::Reset_All_Socket()
{
    snprintf(query, sizeof(query), "UPDATE test1.mlist SET sock = '0'");

    if (0 == mysql_query(DB_CONN, query))
        return 0;
    else
        return -1;
}

int Database::Initialize_DB()
{
    if (!Connect())
        if (!Reset_All_Socket())
        {
            Disconnect();
            return 0;
        }
        else
            return -1;
    else
        return -1;
}

void Database::ConnTest()
{
    int a = 0;
    std::cout << "아무 숫자 입력>> ";
    std::cin >> a;

    if (0 == mysql_query(DB_CONN, "INSERT INTO test1.`new` (c2, c3) VALUES('1', '2');"))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            std::cout << "TEST TEST TEST >>  " << std::endl;
        }
    }
}


int Database::_Login(char* email, char* pw, int sock)
{
    int check = -1;

    if (_CheckID(email) == 0) // 등록된 email 존재하고,
        if (_NoConn(email) == 0) // 해당 email이 서버와 연결된 상태가 아니고,
            if (_SamePW(email, pw) == 0) // 전달받은 비밀번호가 DB 비밀번호와 같으면, 
                if (_SaveSock(email, sock) == 0) // 소켓을 DB에 업데이트 하고, 로그인 성공한다.
                    check = 0;
                else
                    check = -1;
            else
                check = -2;
        else
            check = -3;
    else
        check = -4;

    // 확인용
    if (check == 0)
        puts("회원 확인, 비밀번호 일치, 소켓 저장 성공, Login 성공.");
    else if(check == -1)
        puts("쿼리 실행 실패 소켓 저장 실패");
    else if (check == -2)
        puts("비밀번호 불일지, Login 실패");
    else if (check == -3)
        puts("해당 이메일이 연결상태입니다. 로그인 불가");
    else if (check == -4)
        puts("해당 이메일이 없습니다. 로그인 불가");

    return check;
}

int Database::_CheckID(char* email) // ID 찾기
{
    char query[1024] = { '\0', };
    std::string recv_email(email); // 전달받은 이메일
    std::string qury_email;          // DB에서 가져온 이메일

    snprintf(query, sizeof(query), "SELECT email FROM test1.mlist WHERE email IN (\'%s\')", email); // 등록된 이메일 찾기

    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                qury_email += sql_row[0]; // DB에서 가져온 이메일

            mysql_free_result(sql_result); // 쿼리 실행 결과 초기화

            if (qury_email == recv_email) // 아이디 중복 함수, 아이디 중복의 경우 0
            {
                puts("\nID 중복, Yes 찾았습니다.");
                return 0;
            }
            else
            {
                puts("\nID 중복, No 없습니다.");
                return -1;
            }
        }
        else
            return -1;
    }
    else
    {
        mysql_free_result(sql_result);
        puts("\nID 찾기 쿼리 실행 실패 : DB 오류");
        return -1;
    }
}

int Database::_SaveOTP(char* email, uint32_t key) // 소켓값이 저장되면 0 반환
{
    printf("전달받은 %s의 키 값은 %lu\n", email, key);
    char query[1024] = { '\0', };
    snprintf(query, sizeof(query), "UPDATE test1.mlist SET `otp`='%d', lastconn = now() WHERE email='%s'", key, email);

    if (0 == mysql_query(DB_CONN, query))
        return 0;
    else
        return -1;
}

int Database::_CheckOTP(char* email, char* otp) // ID 찾기
{
    char query[1024] = { '\0', };
    std::string recv_otp(otp); // 전달받은 이메일
    std::string query_otp;          // DB에서 가져온 이메일

    snprintf(query, sizeof(query), "SELECT otp FROM test1.mlist WHERE email IN (\'%s\')", email); // 등록된 이메일 찾기

    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                query_otp += sql_row[0]; // DB에서 가져온 otp

            mysql_free_result(sql_result); // 쿼리 실행 결과 초기화

            if (query_otp == recv_otp) // 아이디 중복 함수, 아이디 중복의 경우 0
            {
                puts("\nOTP 일치, Yes 맞았습니다.");
                return 0;
            }
            else
            {
                puts("\nOTP 불일치, No 틀리립니다.");
                return -1;
            }
        }
        else
            return -1;
    }
    else
    {
        mysql_free_result(sql_result);
        puts("\nID 찾기 쿼리 실행 실패 : DB 오류");
        return -1;
    }
}
int Database::_CheckEmailName(char* email, char* Name)
{
    char query[1024] = { '\0', };
    std::string recv_Name(Name); // 전달받은 이메일
    std::string query_Name;          // DB에서 가져온 이메일

    snprintf(query, sizeof(query), "SELECT Name FROM test1.mlist WHERE email IN (\'%s\')", email); // 등록된 이메일 찾기

    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                query_Name += sql_row[0]; // DB에서 가져온 otp

            mysql_free_result(sql_result); // 쿼리 실행 결과 초기화

            if (query_Name == recv_Name) // 아이디 중복 함수, 아이디 중복의 경우 0
            {
                puts("\nName 일치, Yes 맞았습니다.");
                return 0;
            }
            else
            {
                puts("\nName 불일치, No 틀리립니다.");
                return -1;
            }
        }
        else
            return -1;
    }
    else
    {
        mysql_free_result(sql_result);
        puts("\nID 찾기 쿼리 실행 실패 : DB 오류");
        return -1;
    }
}

int Database::_SavePW(char* email, char* pw)
{
    printf("email 서버에서 전달받은 %s의 비밀번호 값은 %s\n", email, pw);
    char query[1024] = { '\0', };
    snprintf(query, sizeof(query), "UPDATE test1.mlist SET `pw`='%s', lastconn = now() WHERE email='%s'", pw, email);

    if (0 == mysql_query(DB_CONN, query))
        return 0;
    else
        return -1;
}

std::string Database::_FindEmail(char* email) // ID 찾기
{
    char query[1024] = { '\0', };
    std::string recv_email(email); // 수신한 이메일
    std::string qury_email;        // DB에서 가져온 이메일

    snprintf(query, sizeof(query), "SELECT email FROM test1.mlist WHERE email IN (\'%s\')", email);
    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);

            if (sql_row != nullptr)
                qury_email += sql_row[0]; // DB에서 가져온 이메일

            mysql_free_result(sql_result); // 쿼리 실행 결과 초기화

            return qury_email;
        }
        else
        {
            mysql_free_result(sql_result);
            puts("\nID 찾기 쿼리 실행 실패 : DB 오류");

            return "\0";
        }
    }
    else
        return "\0";
}

std::string Database::_FindPW(char* email, char* pw, char* name)
{
    char query[1024] = { '\0', };
    std::string pw_DB = "\0";

    if (_CheckID(email) == 0) // 등록된 email 존재하고,
    {
        if (_SameName(email, name) == 0) // email과 이름이 일치하면,
        {
            snprintf(query, sizeof(query), "SELECT pw FROM test1.mlist WHERE email IN ('%s')", email);

            if (0 == mysql_query(DB_CONN, query)) // pw 를 가져와서
            {
                sql_result = mysql_store_result(DB_CONN);
                if (0 < mysql_num_rows(sql_result))
                {
                    sql_row = mysql_fetch_row(sql_result);
                    if (sql_row != nullptr)
                        pw_DB = sql_row[0]; // 스트링에 담아

                    return pw; // 반환한다.
                }
                else
                {
                    puts("1비밀번호 찾기 실패");
                    return pw;
                }
            }
            else
            {
                puts("2비밀번호 찾기 실패");
                return pw;
            }
        }
        else
        {
            puts("3비밀번호 찾기 실패");
            return pw;
        }
    }
    else
    {
        puts("4비밀번호 찾기 실패");
        return pw;
    }
}

int Database::_SamePW(char* email, char* pw) // 같으면 0 반환
{

    char query[1024] = { '\0', };
    std::string PW;
    std::string recv_PW(pw);

    snprintf(query, sizeof(query), "SELECT pw FROM test1.mlist WHERE email IN ('%s')", email);
    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                PW += sql_row[0];

            std::cout << "[Recv PW: " << recv_PW << "]  [DB PW: " << PW << "]" << std::endl;

            if (PW == recv_PW)
                return 0;
            else
                return -1;
        }
        else
            return -1;
    }
    else
        return -1;

}

int Database::_SameName(char* email, char* name) // 같으면 0 반환
{

    char query[1024] = { '\0', };
    std::string recv_Name(name);
    sql_result = mysql_store_result(DB_CONN);
    snprintf(query, sizeof(query), "SELECT name FROM test1.mlist WHERE email IN ('%s')", email);
    mysql_query(DB_CONN, "set session character_set_results=euckr;");

    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
            {
                std::string Name(sql_row[0]);
                std::cout << "[Recv Name: " << recv_Name << "]  [DB Name: " << Name << "]" << std::endl;

                if (Name == recv_Name)
                    return 0;
                else
                    return -1;
            }
            else
                return -1;
        }
        else
            return -1;
    }
    else
        return - 1;
}

uint32_t Database::_FindCode(std::vector<char*> vec)// 해당 이메일 연결 상태인지 체크
{
    char query3[1024] = { '\0', };
    std::string user_code;
    uint32_t code = 0;

    snprintf(query3, sizeof(query), "SELECT sock FROM test1.mlist WHERE email IN ('%s')", vec[0]);
    if (0 == mysql_query(DB_CONN, query3))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                user_code += sql_row[0];

            code = strtoul(user_code.c_str(), NULL, 10);

            return code;
        }
        else
            return code;
    }
    else
        return code;
}

int Database::_NoConn(char* email)
{
    char query3[1024] = { '\0', };
    std::string sock_check;

    snprintf(query3, sizeof(query), "SELECT sock FROM test1.mlist WHERE email IN ('%s')", email);
    
    if (0 == mysql_query(DB_CONN, query3))
    {
        sql_result = mysql_store_result(DB_CONN);

        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                sock_check += sql_row[0];
            if (sock_check != "\0")
            {
                int i = std::stoi(sock_check); // 현재 접속 상태가 아니면 0이다.
                return i;
            }
            else
                return -1;
        }
        else
            return -1;
    }
    else
        return -1;
}

std::string Database::_MyProfile(char* email) // ID 찾기
{
    char query[1024] = { '\0', };
    std::string str;

    snprintf(query, sizeof(query), "SELECT email, name, word, code FROM test1.mlist WHERE email IN (\'%s\')", email); // 등록된 이메일 찾기
    mysql_query(DB_CONN, "set session character_set_results=euckr;");

    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        
        if ( 0 < mysql_num_rows(sql_result))
        {
            while (NULL != (sql_row = mysql_fetch_row(sql_result)))
            {
                str = std::string(sql_row[0]) + "`" + std::string(sql_row[1]) + "`" + std::string(sql_row[2]) + "`" + std::string(sql_row[3]) + "`";
            }
            puts("친구 리스트 준비 성공.");
            return str;
        }
        else
        {
            puts("친구 쿼리 실패.");
            return "\0";
        }
    }
    else
    {
        puts("친구 쿼리 실패.");
        return "\0";
    }
}

int Database::_SaveSock(char* email, int sock) // 소켓값이 저장되면 0 반환
{
    char query[1024] = { '\0', };
    snprintf(query, sizeof(query), "UPDATE test1.mlist SET sock='%d', lastconn = now() WHERE email='%s'", sock, email);

    if (0 == mysql_query(DB_CONN, query))
        return 0;
    else
        return -1;
}




int Database::_Code(std::vector<char*> vec, int sock)
{
    printf("\n회원가입 DB 처리를 시작합니다.\n");

    snprintf(query, sizeof(query), "INSERT INTO test1.mlist (email, pw, name, word, code, sock, lastconn) VALUES('%s', '%s', '%s', '%s', '%d', '%d', now())", vec[0], vec[1], vec[2], vec[3], 0, sock);
    mysql_query(DB_CONN, "set session character_set_client=euckr;");

    if (0 == mysql_query(DB_CONN, query))
    {
        puts("회원가입 성공.");

        if (0 == _NewTable(vec[0])) // 친구 리스트 테이블 DB 생성
        {
            puts("회원 테이블 생성 성공.");
            return 0;
        }
        else
        {
            puts("회원 테이블 생성 실패.");
            return -1;
        }
    }
    else
    {
        puts("회원가입 실패");
        return -1;
    }
}

int Database::_Join(std::vector<char*> vec, int sock)
{
    printf("\n회원가입 DB 처리를 시작합니다.\n");

    char query3[1024] = { '\0', };
    std::string code_check;

    snprintf(query3, sizeof(query3), "SELECT MAX(code) FROM mlist"); // 추가할 Code 값 가져오기

    if (0 == mysql_query(DB_CONN, query3))
    {
        sql_result = mysql_store_result(DB_CONN);

        if (0 < mysql_num_rows(sql_result)) // 가져온 값이 있으면
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                code_check += sql_row[0]; // 코드를 char*로

            int code = std::stoi(code_check) + 1; // char* 에서 int 로

            snprintf(query, sizeof(query), "INSERT INTO test1.mlist (email, pw, name, word, code, sock, lastconn) VALUES('%s', '%s', '%s', '%s', '%d', '%d', now())", vec[0], vec[1], vec[2], vec[3], 0, sock);
            mysql_query(DB_CONN, "set session character_set_client=euckr;");

            if (0 == mysql_query(DB_CONN, query))
            {
                puts("회원가입 성공.");

               if( 0 == _NewTable(vec[0])) // 친구 리스트 테이블 DB 생성
                    return 0;
                else
                    return -1;
            }
            else
            {
                puts("회원가입 실패");
                return -1;
            }
        }
        else
        {
            puts("1회원 Code 가져오기 실패");
            return -1;
        }
    }
    else
    {
        puts("2회원 Code 가져오기 실패");
        return -1;
    }
}

int Database::_NewTable(char* my_email)
{
    printf("\n친구 목록 DB를 추가합니다.\n");
    std::string email (my_email);

    Converter cc;
    std::string email_name = (cc._Str_Split(email, '@'))[0];

    std::string qry_str = "CREATE TABLE `"+ email_name + "`("
        "`email` varchar(100) NOT NULL," 
        "`pw` varchar(100) DEFAULT NULL," 
        "`name` varchar(100) DEFAULT NULL," 
        "`word` varchar(100) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL," 
        "`code` varchar(100) DEFAULT NULL," 
        "`sock` varchar(100) DEFAULT NULL," 
        "`lastconn` varchar(100) DEFAULT NULL," 
        "PRIMARY KEY(`email`)" 
        ") ENGINE = InnoDB DEFAULT CHARSET = utf8mb3;";

    std::cout << qry_str << std::endl;

    const char* query = qry_str.c_str();

    std::cout << query << std::endl;

    mysql_query(DB_CONN, "set session character_set_client=euckr;");
    if (0 == mysql_query(DB_CONN, query))
    {
        printf("\n쿼리성공:%d, 쿼리문:%s\n", 1, query);
        puts("친구 DB 생성 성공.");
        return 0;
    }
    else
    {
        puts("친구 DB 생성 실패");
        return -1;
    }

}

std::vector<std::string> Database::_FrndProfile(char* my_email)
{
    char query[1024] = { '\0', };
    std::vector<std::string> friend_list;
    std::string str_friend_info = "\0";

    printf("%s 친구 목록 테이블에서 친구 프로필 정보를 가져옵니다.\n ", my_email);
    snprintf(query, sizeof(query), "SELECT email, name, word, code FROM test1.`%s` WHERE email IS NOT NULL;", my_email);
    mysql_query(DB_CONN, "set session character_set_results=euckr;");
   
    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            while (NULL != (sql_row = mysql_fetch_row(sql_result)))
            {
                str_friend_info = std::string(sql_row[0]) + "`" 
                                + std::string(sql_row[1]) + "`" 
                                + std::string(sql_row[2]) + "`" 
                                + std::string(sql_row[3]) + "`";

                friend_list.push_back(str_friend_info);
            }
            puts("3 친구 리스트 준비 성공.");
            return friend_list;
        }
        else
        {

            puts("3 친구 리스트 준비 실패");
            return friend_list;
        }
    }
    else
    {
        puts("3 친구 프로필 쿼리 실패");
        return friend_list;
    }
}