//class SendRecv
//{
//    // 문장 ﻿전송 (서버 <= 클라)
//    // 리턴: 성공 len, 실패 -1 // 매개변수: 소켓, flag, 문장, option
//    public int Send_(Socket socket, char flag, string str, char opt)
//    {
//        Console.WriteLine("#B.. 문장 전송 시작 (C++서버 << C#클라)");
//        int ret = -1, cnt;
//        opt = '\0';
//        byte[] BYTE_str = null;

//        // #B.. 문장과 헤더정보를 전송할 하나의 바이트 배열로 조합
//        ConvertString.ConvertString convert = new ConvertString.ConvertString();
//        short len = convert.str_to_byte(ref BYTE_str, flag, str, opt);

//        // #B.. 조합된 바이트 배열을 서버로 전송
//        cnt = socket.Send(BYTE_str, len, SocketFlags.None);
//        if (cnt != len)
//        {
//            Console.WriteLine("#B.. 문장 전송 실패 {0}", cnt);
//        }
//        else
//        {
//            // 잘 전송되었으면, 전송 함수를 종료한다.
//            ret = cnt;
//            Console.WriteLine("#B.. 전송 성공 크기: {0, 4} Byte, 목표 크기: {1, 4} Byte", cnt, len);
//        }
//        Console.WriteLine("------------------------------------------------------------------------\n");
//        return ret;
//    }

//    public string Recv_(Socket socket)
//    {
//        Console.WriteLine("#A.. 문장 수신 시작 (C++서버 >> C#클라)");

//        byte[] header = new byte[4];
//        short recv_size;
//        string return_str = null;

//        // #A1. 헤더 4Byte만 수신
//        int cnt = socket.Receive(header, 4, SocketFlags.None);
//        Console.WriteLine("1 {0}", cnt);

//        // #A2. 첫 1Byte flag 읽음
//        // 여기서 0이면 채팅 처리, 1이면 파일 정보 읽어 버퍼 생성, 그외는 식별자에 따라 버퍼에 쌓기 

//        // #A3. 다음 실제 문장 수신 크기 = 전체 전송체 - 헤더 4byte
//        recv_size = (short)(header[1] - 4);
//        Console.WriteLine("2 {0}", recv_size);

//        //return 0;
//        if (cnt != 4)
//        {
//            Console.WriteLine("#A1. 헤더 4Byte 수신 실패 XXXXX ");
//        }
//        else
//        {
//            // #A4. 크기 만큼 바이트 배열 생성
//            byte[] msg = new byte[recv_size];

//            // #A5. 수신
//            cnt = socket.Receive(msg, recv_size, SocketFlags.None);
//            Console.WriteLine("3 {0}", cnt);
//            if (cnt != recv_size)
//            {
//                return_str = Encoding.Unicode.GetString(msg);
//                Console.WriteLine("#A6. 문장 수신 실패  : {0, 3} Byte, 수신 문자 : {1, 3} ", cnt, return_str);
//            }
//            else
//            {
//                return_str = Encoding.Unicode.GetString(msg);
//                Console.WriteLine("#A. 문장 수신 성공 크기 : {0, 3} Byte, 수신 문자 : {1, 3} ", cnt, return_str);
//            }
//        }
//        Console.WriteLine("------------------------------------------------------------------------\n");
//        return return_str;
//    }

    class z_save_code
{



    ///* [패턴 C] 파일 수신 (서버 => 클라) */
    //// 성공 size, 실패 null매개변수: connected 이후 소켓, 경로, 아이디
    //int RecvFile_From_CPP_Serv(Socket socket, string addr, string id)
    //{
    //    Console.WriteLine("#C.. 통신 패턴 C 파일 수신 (C++서버 >> C#클라) 시작\n");

    //    //
    //    // 이 함수의 성공 여부, 성공 file size, 실패 null
    //    byte[] byte_recv_size = new byte[4];
    //    int ret = 0;

    //    Console.WriteLine("#C1.");
    //    Console.WriteLine("#C2.");
    //    // #C3. 전송받을 파일의 크기를 int 형으로 서버에서 먼저 받고  
    //    int recv_size_cnt = socket.Receive(byte_recv_size, 4, SocketFlags.None);
    //    if (recv_size_cnt != 4)
    //    {
    //        Console.WriteLine("\n#C3. 크기 수신 실패 XXXXX : {0} Byte", recv_size_cnt);
    //    }
    //    else
    //    {
    //        Console.WriteLine("#C3. 크기 수신 성공 : {0} Byte 정보 수신", recv_size_cnt);

    //        // #C4. 받은 4바이트를 int 형 변환
    //        int recv_size = BitConverter.ToInt32(byte_recv_size, 0);
    //        Console.WriteLine("#C4. 수신 크기 확인 : {0} 크기 받을 예정", recv_size);

    //        // #C5. 에코 (길이 + 2)
    //        int i_echo = recv_size + 2;
    //        byte[] byte_echo_size = BitConverter.GetBytes(i_echo);
    //        Console.WriteLine("#C5. 에코 길이 준비 : {0} 에코 전송 예정", i_echo);

    //        // #A6. 에코 전송
    //        int sent_size_cnt = socket.Send(byte_echo_size, 4, SocketFlags.None);
    //        if (sent_size_cnt != 4)
    //        {
    //            Console.WriteLine("\n#C6. 에코 전송 실패 XXXXX : {0} Byte", sent_size_cnt);
    //        }
    //        else
    //        {
    //            Console.WriteLine("#C6. 에코 전송 성공 : {0} Byte  {1} 전송", sent_size_cnt, i_echo);


    //            Console.WriteLine("#C7.");
    //            Console.WriteLine("#C8.");
    //            Console.WriteLine("#C9.");
    //            // #C9. 서버에서 오는 메시지를 받는다.
    //            byte[] byte_recv_msg = new byte[recv_size];
    //            recv_size_cnt = socket.Receive(byte_recv_msg, recv_size, SocketFlags.None);
    //            if (recv_size_cnt != recv_size)
    //            {
    //                Console.WriteLine("\n#C10 서버 수신 실패 XXXXX 목표:{0}, 실제:{1}", recv_size, recv_size_cnt);
    //            }
    //            else
    //            {
    //                //
    //                // 경로 설정해서 대입 필요
    //                Stream outStream = new FileStream("C:\\Users\\iot2122\\Downloads\\recieved_123.png", FileMode.Create);
    //                outStream.Write(byte_recv_msg, 0, byte_recv_msg.Length);
    //                Console.WriteLine("#C10.  저장 성공 : {0}", recv_size_cnt);
    //                outStream.Close();

    //                ret = recv_size_cnt;
    //            }
    //        }
    //    }
    //    Console.WriteLine("------------------------------------------------------------------------\n");

    //    return ret;
    //}

    ///* [패턴 D] 파일 전송 (서버 <= 클라) */
    //// 매개변수: connected 이후 소켓, 경로, 아이디
    //// 전송 크기, 실패 -1
    //int SendFile_To_CPP_Serv(Socket socket, string addr, string id)
    //{
    //    Console.WriteLine("\n#D..파일 전송 시작 (C++서버 << C#클라)");

    //    //
    //    // 경로 설정 필요
    //    string target_addr = "C:\\Users\\iot2122\\Downloads\\321.png";
    //    FileInfo target_file = new FileInfo(target_addr);

    //    // Byte 스트림 생성
    //    using (FileStream stream = new FileStream(target_file.FullName, FileMode.Open, FileAccess.Read))
    //    {
    //        Action<byte[]> Send = (file) =>
    //        {
    //            // 먼저 데이터 사이즈를 보내고.
    //            socket.Send(BitConverter.GetBytes(file.Length), 4, SocketFlags.None);
    //            // 에코가 데이터 사이즈 +2 라면,
    //            byte[] echo = new byte[4];
    //            socket.Receive(echo, 4, SocketFlags.None);
    //            if(BitConverter.ToInt32(echo) == (file.Length+2))
    //                // 데이터를 보낸다.
    //                socket.Send(file, file.Length, SocketFlags.None);
    //        };

    //        // 파일 Byte배열 준비
    //        byte[] data = new byte[target_file.Length];
    //        stream.Read(data, 0, data.Length);

    //        // 파일 Byte 배열을 보낸다.
    //        Send(data);

    //        // 서버로부터 byte=1 데이터가 오면 함수 종료
    //        byte[] ret = new byte[1];
    //        socket.Receive(ret, 1, SocketFlags.None);
    //        if (ret[0] == 1)
    //        {
    //            Console.WriteLine("Completed");
    //        }
    //    }

    //    return 0;
    //}
    //    string Recv_From_CPP(Socket socket)
    //    {
    //        string return_value = null;
    //        byte[] byte_arr = new byte[4];

    //        // #A3. 문자열의 크기를 C++ 서버에서 받고 
    //        int cnt = socket.Receive(byte_arr, 4, SocketFlags.None);
    //        Console.WriteLine("#A.. 문장 수신 시작 (C++서버 >> C#클라) \n");
    //        Console.WriteLine("#A1.");
    //        Console.WriteLine("#A2.");
    //        if (cnt != 4)
    //        {
    //            Console.WriteLine("\n#A3. 크기 수신 실패 XXXXX ");
    //        }
    //        else
    //        {
    //            Console.WriteLine("#A3. 문장 크기 수신 성공");

    //            // #A4. 받은 4바이트 byte배열을 int 형 변환
    //            int msg_size = BitConverter.ToInt32(byte_arr, 0);
    //            Console.WriteLine("#A4. 크기 변환 성공 : {0, 3} Byte", msg_size);

    //            // #A5. 크기 만큼 바이트 배열 변환
    //            byte[] byte_type_recv_msg = new byte[msg_size];
    //            Console.WriteLine("#A5. 수신 크기 준비 완료");
    //            Console.WriteLine("#A6.");

    //            // #A7. 크기 만큼 바이트 배열 변환
    //            cnt = socket.Receive(byte_type_recv_msg, msg_size, SocketFlags.None);
    //            if (cnt != msg_size)
    //            {
    //                Console.WriteLine("\n#A6. 문장 수신 실패 XXXXX ");
    //            }
    //            else
    //            {
    //                // #(통신 끝) 서버에서 전송받은 메시지 출력
    //                return_value = Encoding.Unicode.GetString(byte_type_recv_msg);
    //                Console.WriteLine("#A6. 문장 수신 성공 크기 : {0, 3} Byte, 수신 문자 : {1, 3} ", cnt, return_value);
    //            }
    //        }
    //        Console.WriteLine("------------------------------------------------------------------------\n");
    //        return return_value;
    //    }
    //int Send_To_CPP(Socket socket, string str)
    //{
    //    Console.WriteLine("\n#B.. 문장 전송 시작 (C++서버 << C#클라)\n");
    //    int ret = -1, send_size, cnt;

    //    // #B1. 플레그, ASCII, 1Byte // 아스키 char 전송
    //    byte[] BYTE_flag = Encoding.ASCII.GetBytes("A");

    //    // #B2. 문자열(str + "\0")
    //    byte[] BYTE_str = Encoding.Unicode.GetBytes(str + "\0");

    //    // #B3. 문자열의 크기만 계산한 byte 배열
    //    short len = (short)BYTE_str.Length;
    //    byte[] BYTE_size = BitConverter.GetBytes(len);

    //    // #B4. 옵션, ASCII, 1Byte 
    //    byte[] BYTE_opt = Encoding.ASCII.GetBytes("\0");

    //    // #B5. 플레그 + 문자열 크기 + 문자열 연결된 byte 배열 생성
    //    send_size = BYTE_flag.Length + BYTE_size.Length + BYTE_opt.Length + BYTE_str.Length;
    //    byte[] BYTE_send = new byte[send_size];

    //    // #B6. 전송할 배열에 flag, size, str 순서로 복사 
    //    Array.Copy(BYTE_flag, 0, BYTE_send, 0, BYTE_flag.Length);
    //    Array.Copy(BYTE_size, 0, BYTE_send, BYTE_flag.Length, BYTE_size.Length);
    //    Array.Copy(BYTE_opt, 0, BYTE_send, BYTE_flag.Length + BYTE_size.Length, BYTE_opt.Length);
    //    Array.Copy(BYTE_str, 0, BYTE_send, BYTE_flag.Length + BYTE_size.Length + BYTE_opt.Length, BYTE_str.Length);

    //    // #B6. 전송 배열을 서버로 전송
    //    cnt = socket.Send(BYTE_send, send_size, SocketFlags.None);
    //    if (cnt != send_size)
    //    {
    //        Console.WriteLine("#B5. 문장 전송 실패 {0}", cnt);
    //    }
    //    else
    //    {
    //        // 잘 전송되었으면, 전송 함수를 종료한다.
    //        ret = cnt;
    //        Console.WriteLine("#B.. 전송 성공 크기: {0, 4} Byte, 목표 크기: {1, 4} Byte", cnt, send_size);
    //    }
    //    Console.WriteLine("------------------------------------------------------------------------\n");
    //    return ret;
    //}



    //// [패턴 A] ﻿문장 수신 (서버 => 클라), 성공 string, 실패 null, 매개변수: connected 이후 소켓
    //string RecvMSG_From_CPP_Serv(Socket socket)
    //{
    //    string return_value = null;
    //    byte[] byte_arr = new byte[4];

    //    // #A3. 문자열의 크기를 C++ 서버에서 받고 
    //    int cnt = socket.Receive(byte_arr, 4, SocketFlags.None);
    //    Console.WriteLine("#A.. 문장 수신 시작 (C++서버 >> C#클라) \n");
    //    Console.WriteLine("#A1.");
    //    Console.WriteLine("#A2.");
    //    if (cnt != 4)
    //    {
    //        Console.WriteLine("\n#A3. 크기 수신 실패 XXXXX ");
    //    }
    //    else
    //    {
    //        Console.WriteLine("#A3. 문장 크기 수신 성공");

    //        // #A4. 받은 4바이트 byte배열을 int 형 변환
    //        int msg_size = BitConverter.ToInt32(byte_arr, 0);
    //        Console.WriteLine("#A4. 크기 변환 성공 : {0, 3} Byte", msg_size);

    //        // #A5. 크기 만큼 바이트 배열 변환
    //        byte[] byte_type_recv_msg = new byte[msg_size];
    //        Console.WriteLine("#A5. 수신 크기 준비 완료");
    //        Console.WriteLine("#A6.");

    //        // #A7. 크기 만큼 바이트 배열 변환
    //        cnt = socket.Receive(byte_type_recv_msg, msg_size, SocketFlags.None);
    //        if (cnt != msg_size)
    //        {
    //            Console.WriteLine("\n#A6. 문장 수신 실패 XXXXX ");
    //        }
    //        else
    //        {
    //            // #(통신 끝) 서버에서 전송받은 메시지 출력
    //            return_value = Encoding.Unicode.GetString(byte_type_recv_msg);
    //            Console.WriteLine("#A6. 문장 수신 성공 크기 : {0, 3} Byte, 수신 문자 : {1, 3} ", cnt, return_value);
    //        }
    //    }
    //    Console.WriteLine("------------------------------------------------------------------------\n");
    //    return return_value;
    //}
    // [패턴 B] 문장 ﻿전송 (서버 <= 클라), 성공 len, 실패 -1, 매개변수: 소켓과 string 문자열 전달
    //int SendMSG_To_CPP_Serv(Socket socket, string str)
    //{
    //    Console.WriteLine("\n#B.. 문장 전송 시작 (C++서버 << C#클라)\n");
    //    int ret = -1, str_size, cnt;

    //    // #B1. 전송할 (str + '\0')을 byte 배열로 변경
    //    byte[] BYTE_str = Encoding.Unicode.GetBytes(str + "\0");
    //    if(BYTE_str != null)
    //        Console.WriteLine("#B1. 문장 \"{0}\"을 Byte[]로 변경", str);

    //    // #B2. 전송할 Byte 배열 크기 측정 == str(각2Byte) + '\0' 
    //    str_size = BYTE_str.Length;
    //    if(str_size != 0)
    //        Console.WriteLine("#B2. 전송할 Byte[] 크기 : {0, 3} Byte", str_size);

    //    // #B3. 계산한 크기를 byte 배열로 변경
    //    byte[] BYTE_size = BitConverter.GetBytes(str_size);
    //    if (BYTE_str != null)
    //        Console.WriteLine("#B3. 문장 크기 \"{0}\"를 Byte[]로 변경", str_size);

    //    // #B4. 계산한 크기를 서버로 전송
    //    cnt = socket.Send(BYTE_size, 4, SocketFlags.None);
    //    if (cnt != 4)
    //    {
    //        Console.WriteLine("#B4. 문장 크기 전송 실패");
    //    }
    //    else
    //    {
    //        Console.WriteLine("#B4. 문장 크기 전송 성공 : {0, 3} Byte", str_size);

    //        // #B5. C++ 서버로 문자를 보낸다.(eg. Login^ID^PW^)
    //        cnt = socket.Send(BYTE_str, str_size, SocketFlags.None);
    //        if (cnt != str_size)
    //        {
    //            Console.WriteLine("#B5. 문장 전송 실패");
    //        }
    //        else
    //        {
    //            // 잘 전송되었으면, 전송 함수를 종료한다.
    //            ret = cnt;
    //            Console.WriteLine("#B6. 전송 크기 : {0, 3} Byte, 전송 문자 : {1, 3}", cnt, Encoding.Unicode.GetString(BYTE_str));

    //        }
    //    }
    //    Console.WriteLine("------------------------------------------------------------------------\n");
    //    return ret;
    //}

    


}

