#include "Database.h"

int Database::Connect()
{
    mysql_init(&DB_REAL);

    DB_CONN = mysql_real_connect(&DB_REAL, DB_HOST, DB_USER, DB_PSWD, DB_NAME, 3306, (char*)NULL, 0);
    mysql_query(DB_CONN, "set session character_set_connection=euckr;");
    mysql_query(DB_CONN, "set session character_set_results=euckr;"); // �ѱ� �����ö�
    mysql_query(DB_CONN, "set session character_set_client=euckr;");  // �ѱ� �����Ҷ�

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
    mysql_query(DB_CONN, "set session character_set_results=euckr;"); // �ѱ� �����ö�
    mysql_query(DB_CONN, "set session character_set_client=euckr;");  // �ѱ� �����Ҷ�

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
    std::cout << "�ƹ� ���� �Է�>> ";
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

    if (_CheckID(email) == 0) // ��ϵ� email �����ϰ�,
        if (_NoConn(email) == 0) // �ش� email�� ������ ����� ���°� �ƴϰ�,
            if (_SamePW(email, pw) == 0) // ���޹��� ��й�ȣ�� DB ��й�ȣ�� ������, 
                if (_SaveSock(email, sock) == 0) // ������ DB�� ������Ʈ �ϰ�, �α��� �����Ѵ�.
                    check = 0;
                else
                    check = -1;
            else
                check = -2;
        else
            check = -3;
    else
        check = -4;

    // Ȯ�ο�
    if (check == 0)
        puts("ȸ�� Ȯ��, ��й�ȣ ��ġ, ���� ���� ����, Login ����.");
    else if(check == -1)
        puts("���� ���� ���� ���� ���� ����");
    else if (check == -2)
        puts("��й�ȣ ������, Login ����");
    else if (check == -3)
        puts("�ش� �̸����� ��������Դϴ�. �α��� �Ұ�");
    else if (check == -4)
        puts("�ش� �̸����� �����ϴ�. �α��� �Ұ�");

    return check;
}

int Database::_CheckID(char* email) // ID ã��
{
    char query[1024] = { '\0', };
    std::string recv_email(email); // ���޹��� �̸���
    std::string qury_email;          // DB���� ������ �̸���

    snprintf(query, sizeof(query), "SELECT email FROM test1.mlist WHERE email IN (\'%s\')", email); // ��ϵ� �̸��� ã��

    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                qury_email += sql_row[0]; // DB���� ������ �̸���

            mysql_free_result(sql_result); // ���� ���� ��� �ʱ�ȭ

            if (qury_email == recv_email) // ���̵� �ߺ� �Լ�, ���̵� �ߺ��� ��� 0
            {
                puts("\nID �ߺ�, Yes ã�ҽ��ϴ�.");
                return 0;
            }
            else
            {
                puts("\nID �ߺ�, No �����ϴ�.");
                return -1;
            }
        }
        else
            return -1;
    }
    else
    {
        mysql_free_result(sql_result);
        puts("\nID ã�� ���� ���� ���� : DB ����");
        return -1;
    }
}

int Database::_SaveOTP(char* email, uint32_t key) // ���ϰ��� ����Ǹ� 0 ��ȯ
{
    printf("���޹��� %s�� Ű ���� %lu\n", email, key);
    char query[1024] = { '\0', };
    snprintf(query, sizeof(query), "UPDATE test1.mlist SET `otp`='%d', lastconn = now() WHERE email='%s'", key, email);

    if (0 == mysql_query(DB_CONN, query))
        return 0;
    else
        return -1;
}

int Database::_CheckOTP(char* email, char* otp) // ID ã��
{
    char query[1024] = { '\0', };
    std::string recv_otp(otp); // ���޹��� �̸���
    std::string query_otp;          // DB���� ������ �̸���

    snprintf(query, sizeof(query), "SELECT otp FROM test1.mlist WHERE email IN (\'%s\')", email); // ��ϵ� �̸��� ã��

    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                query_otp += sql_row[0]; // DB���� ������ otp

            mysql_free_result(sql_result); // ���� ���� ��� �ʱ�ȭ

            if (query_otp == recv_otp) // ���̵� �ߺ� �Լ�, ���̵� �ߺ��� ��� 0
            {
                puts("\nOTP ��ġ, Yes �¾ҽ��ϴ�.");
                return 0;
            }
            else
            {
                puts("\nOTP ����ġ, No Ʋ�����ϴ�.");
                return -1;
            }
        }
        else
            return -1;
    }
    else
    {
        mysql_free_result(sql_result);
        puts("\nID ã�� ���� ���� ���� : DB ����");
        return -1;
    }
}
int Database::_CheckEmailName(char* email, char* Name)
{
    char query[1024] = { '\0', };
    std::string recv_Name(Name); // ���޹��� �̸���
    std::string query_Name;          // DB���� ������ �̸���

    snprintf(query, sizeof(query), "SELECT Name FROM test1.mlist WHERE email IN (\'%s\')", email); // ��ϵ� �̸��� ã��

    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                query_Name += sql_row[0]; // DB���� ������ otp

            mysql_free_result(sql_result); // ���� ���� ��� �ʱ�ȭ

            if (query_Name == recv_Name) // ���̵� �ߺ� �Լ�, ���̵� �ߺ��� ��� 0
            {
                puts("\nName ��ġ, Yes �¾ҽ��ϴ�.");
                return 0;
            }
            else
            {
                puts("\nName ����ġ, No Ʋ�����ϴ�.");
                return -1;
            }
        }
        else
            return -1;
    }
    else
    {
        mysql_free_result(sql_result);
        puts("\nID ã�� ���� ���� ���� : DB ����");
        return -1;
    }
}

int Database::_SavePW(char* email, char* pw)
{
    printf("email �������� ���޹��� %s�� ��й�ȣ ���� %s\n", email, pw);
    char query[1024] = { '\0', };
    snprintf(query, sizeof(query), "UPDATE test1.mlist SET `pw`='%s', lastconn = now() WHERE email='%s'", pw, email);

    if (0 == mysql_query(DB_CONN, query))
        return 0;
    else
        return -1;
}

std::string Database::_FindEmail(char* email) // ID ã��
{
    char query[1024] = { '\0', };
    std::string recv_email(email); // ������ �̸���
    std::string qury_email;        // DB���� ������ �̸���

    snprintf(query, sizeof(query), "SELECT email FROM test1.mlist WHERE email IN (\'%s\')", email);
    if (0 == mysql_query(DB_CONN, query))
    {
        sql_result = mysql_store_result(DB_CONN);
        if (0 < mysql_num_rows(sql_result))
        {
            sql_row = mysql_fetch_row(sql_result);

            if (sql_row != nullptr)
                qury_email += sql_row[0]; // DB���� ������ �̸���

            mysql_free_result(sql_result); // ���� ���� ��� �ʱ�ȭ

            return qury_email;
        }
        else
        {
            mysql_free_result(sql_result);
            puts("\nID ã�� ���� ���� ���� : DB ����");

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

    if (_CheckID(email) == 0) // ��ϵ� email �����ϰ�,
    {
        if (_SameName(email, name) == 0) // email�� �̸��� ��ġ�ϸ�,
        {
            snprintf(query, sizeof(query), "SELECT pw FROM test1.mlist WHERE email IN ('%s')", email);

            if (0 == mysql_query(DB_CONN, query)) // pw �� �����ͼ�
            {
                sql_result = mysql_store_result(DB_CONN);
                if (0 < mysql_num_rows(sql_result))
                {
                    sql_row = mysql_fetch_row(sql_result);
                    if (sql_row != nullptr)
                        pw_DB = sql_row[0]; // ��Ʈ���� ���

                    return pw; // ��ȯ�Ѵ�.
                }
                else
                {
                    puts("1��й�ȣ ã�� ����");
                    return pw;
                }
            }
            else
            {
                puts("2��й�ȣ ã�� ����");
                return pw;
            }
        }
        else
        {
            puts("3��й�ȣ ã�� ����");
            return pw;
        }
    }
    else
    {
        puts("4��й�ȣ ã�� ����");
        return pw;
    }
}

int Database::_SamePW(char* email, char* pw) // ������ 0 ��ȯ
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

int Database::_SameName(char* email, char* name) // ������ 0 ��ȯ
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

uint32_t Database::_FindCode(std::vector<char*> vec)// �ش� �̸��� ���� �������� üũ
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
                int i = std::stoi(sock_check); // ���� ���� ���°� �ƴϸ� 0�̴�.
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

std::string Database::_MyProfile(char* email) // ID ã��
{
    char query[1024] = { '\0', };
    std::string str;

    snprintf(query, sizeof(query), "SELECT email, name, word, code FROM test1.mlist WHERE email IN (\'%s\')", email); // ��ϵ� �̸��� ã��
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
            puts("ģ�� ����Ʈ �غ� ����.");
            return str;
        }
        else
        {
            puts("ģ�� ���� ����.");
            return "\0";
        }
    }
    else
    {
        puts("ģ�� ���� ����.");
        return "\0";
    }
}

int Database::_SaveSock(char* email, int sock) // ���ϰ��� ����Ǹ� 0 ��ȯ
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
    printf("\nȸ������ DB ó���� �����մϴ�.\n");

    snprintf(query, sizeof(query), "INSERT INTO test1.mlist (email, pw, name, word, code, sock, lastconn) VALUES('%s', '%s', '%s', '%s', '%d', '%d', now())", vec[0], vec[1], vec[2], vec[3], 0, sock);
    mysql_query(DB_CONN, "set session character_set_client=euckr;");

    if (0 == mysql_query(DB_CONN, query))
    {
        puts("ȸ������ ����.");

        if (0 == _NewTable(vec[0])) // ģ�� ����Ʈ ���̺� DB ����
        {
            puts("ȸ�� ���̺� ���� ����.");
            return 0;
        }
        else
        {
            puts("ȸ�� ���̺� ���� ����.");
            return -1;
        }
    }
    else
    {
        puts("ȸ������ ����");
        return -1;
    }
}

int Database::_Join(std::vector<char*> vec, int sock)
{
    printf("\nȸ������ DB ó���� �����մϴ�.\n");

    char query3[1024] = { '\0', };
    std::string code_check;

    snprintf(query3, sizeof(query3), "SELECT MAX(code) FROM mlist"); // �߰��� Code �� ��������

    if (0 == mysql_query(DB_CONN, query3))
    {
        sql_result = mysql_store_result(DB_CONN);

        if (0 < mysql_num_rows(sql_result)) // ������ ���� ������
        {
            sql_row = mysql_fetch_row(sql_result);
            if (sql_row != nullptr)
                code_check += sql_row[0]; // �ڵ带 char*��

            int code = std::stoi(code_check) + 1; // char* ���� int ��

            snprintf(query, sizeof(query), "INSERT INTO test1.mlist (email, pw, name, word, code, sock, lastconn) VALUES('%s', '%s', '%s', '%s', '%d', '%d', now())", vec[0], vec[1], vec[2], vec[3], 0, sock);
            mysql_query(DB_CONN, "set session character_set_client=euckr;");

            if (0 == mysql_query(DB_CONN, query))
            {
                puts("ȸ������ ����.");

               if( 0 == _NewTable(vec[0])) // ģ�� ����Ʈ ���̺� DB ����
                    return 0;
                else
                    return -1;
            }
            else
            {
                puts("ȸ������ ����");
                return -1;
            }
        }
        else
        {
            puts("1ȸ�� Code �������� ����");
            return -1;
        }
    }
    else
    {
        puts("2ȸ�� Code �������� ����");
        return -1;
    }
}

int Database::_NewTable(char* my_email)
{
    printf("\nģ�� ��� DB�� �߰��մϴ�.\n");
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
        printf("\n��������:%d, ������:%s\n", 1, query);
        puts("ģ�� DB ���� ����.");
        return 0;
    }
    else
    {
        puts("ģ�� DB ���� ����");
        return -1;
    }

}

std::vector<std::string> Database::_FrndProfile(char* my_email)
{
    char query[1024] = { '\0', };
    std::vector<std::string> friend_list;
    std::string str_friend_info = "\0";

    printf("%s ģ�� ��� ���̺��� ģ�� ������ ������ �����ɴϴ�.\n ", my_email);
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
            puts("3 ģ�� ����Ʈ �غ� ����.");
            return friend_list;
        }
        else
        {

            puts("3 ģ�� ����Ʈ �غ� ����");
            return friend_list;
        }
    }
    else
    {
        puts("3 ģ�� ������ ���� ����");
        return friend_list;
    }
}