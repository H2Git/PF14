#include "Converter.h"

char* Converter::_WC2C(wchar_t* wc)
{
    char* pStr;
    int strSize = WideCharToMultiByte(CP_ACP, 0, wc, -1, NULL, 0, NULL, NULL);
    pStr = new char[strSize];
    WideCharToMultiByte(CP_ACP, 0, wc, -1, pStr, strSize, 0, 0);

    return pStr;
}

wchar_t* Converter::_C2WC(char* str)
{
    wchar_t* pStr;
    int strSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL);
    pStr = new WCHAR[strSize];
    MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, pStr, strSize);

    return pStr;
}

void Converter::Split_CStr(char* str, char* cArr[])
{
    char* context = NULL;
    char* token = strtok_s(str, "`", &context); //�ش� ���ڸ� �������� ���ڿ� �ڸ���

    for (int i = 0; i < 3; i++)
    {
        cArr[i] = token;
        //printf("Convert���� �ڸ� ���ڿ� [%d]: %s\n", i, (cArr[i])); // ���
        token = strtok_s(NULL, "`", &context); // �ٽ� �ڸ���
    }
}

std::vector<char*> Converter::_WC2VC(wchar_t* wc_ptr)
{
    puts("���۹��� ���ڿ��� ���ͷ� �����մϴ�.\n");
    char* c_ptr;
    int strSize = WideCharToMultiByte(CP_ACP, 0, wc_ptr, -1, NULL, 0, NULL, NULL);
    c_ptr = new char[strSize];
    WideCharToMultiByte(CP_ACP, 0, wc_ptr, -1, c_ptr, strSize, 0, 0);

    //char* c_ptr_arr[3] = { NULL, };       // ��ȯ�� char ���ڿ��� ������ 3��������.
    std::vector<char*> c_ptr_vec;

    char* context = NULL;
    char* token = strtok_s(c_ptr, "`", &context); //�ش� ���ڸ� �������� ���ڿ� �ڸ���

    for (int i = 0; i < 4; i++)
    {
        c_ptr_vec.push_back(token);
        //c_ptr_arr[i] = token;
        printf("���� [%d]: %s\n", i, token); // ���

        token = strtok_s(NULL, "`", &context); // �ٽ� �ڸ���
    }

    return c_ptr_vec;
}

wchar_t* Converter::_Str2WC(std::string str)
{
    std::vector<char> writable(str.begin(), str.end());
    writable.push_back('\0');
    char* c_ptr = &writable[0]; // �̸����� char �����ͷ� ��ȯ
    wchar_t* wc_ptr = _C2WC(c_ptr);

    return wc_ptr;
}

std::vector<std::string> Converter::_Str_Split(std::string input, char delimiter) {
    std::vector<std::string> answer;
    std::stringstream ss(input);
    std::string temp;

    while (getline(ss, temp, delimiter)) {
        answer.push_back(temp);
    }

    return answer;
}


/* �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� */

__int16 Converter::str_to_byte(wchar_t* msg, unsigned char* flag, unsigned char* opt)
{
    // #B3. ���ڿ��� ũ�� ���(����*2)
    __int16 len = ((__int16)wcslen(msg)) * 2;

    unsigned char* ptr_msg = (unsigned char*)msg;
    unsigned char* ptr_size = (unsigned char*)&len;

    // #B5. ����ü�� ��� �߰� ���� �߰�
    ptr_msg[0] = *flag;
    ptr_msg[1] = (unsigned char)*ptr_size;
    ptr_msg[2] = (unsigned char)*(ptr_size + 1);
    ptr_msg[3] = *opt;

    ////test
    //cout << "flag   " << *flag << endl;
    //cout << "flag   " << ptr_msg[0] << endl;
    //cout << "c > " << (__int16)*ptr_size << endl;
    std::cout << "size : " << (__int16)ptr_msg[1] << std::endl;
    //cout << "c > " << len << endl;
    //cout << "d > " << *opt << endl;

    return len;
}

wchar_t* Converter::concat_header(char* ch)
{
    wchar_t err[] = { 'e', 'r', 'r', 'o', 'r', '\0' };
    wchar_t* wc_request = err;
    int len = strlen(ch) + 4;

    // 4byte null �߰� + ��ü 1024byte ���ϸ�
    if (len > 1024)
    {
        wc_request = NULL;
        return wc_request;
    }
    else
    {
        // �� ���� �ٽ� ����
        int strSize = MultiByteToWideChar(CP_ACP, 0, ch, -1, NULL, NULL);
        wc_request = new WCHAR[len];
        MultiByteToWideChar(CP_ACP, 0, ch, strlen(ch) + 1, wc_request + 2, len);
    }
    return wc_request;
}
std::vector<std::string> Converter::split2(char* str, std::string delimiter)
{
    std::vector<std::string> ret;
    char* temp_string = NULL;
    char* token = strtok_s(str, delimiter.c_str(), &temp_string);
    while (token)
    {
        ret.push_back(token);
        token = strtok_s(NULL, delimiter.c_str(), &temp_string);
    }
    return ret;
}

std::vector<std::string> Converter::_Wchar_To_VectorString(wchar_t* msg)
{
    std::wstring wstr(msg);
    std::string str(wstr.begin(), wstr.end());
    std::vector<std::string> vector_string = split2(const_cast<char*>(str.c_str()), "^");
    vector_string.shrink_to_fit();

    return vector_string;
}

