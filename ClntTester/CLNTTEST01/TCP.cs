using System;
using System.IO;
using System.Threading;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Threading.Tasks;
using System.Text;

namespace TCP
{
    public enum Ret
    {
        SUCCESS,
        EXCEPTION,
        FAIl
    }

    public class Gorbal
    {
        public static Dictionary<string, int> to_MyCode;
        public static Dictionary<int, string> to_MyId;
    }

    class TCP
    {
        readonly object Lock = new();

        // ManualResetEvent instances signal completion.  
        private static ManualResetEvent connectDone = new ManualResetEvent(false);
        private static ManualResetEvent sendDone = new ManualResetEvent(false);
        private static ManualResetEvent receiveDone = new ManualResetEvent(false);

        public void Print_Exception(Exception e)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine(e.ToString());
            Console.ResetColor();
        }
        private static void ConnectCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the socket from the state object.  
                Socket client = (Socket)ar.AsyncState;

                // Complete the connection.  
                client.EndConnect(ar);

                Console.WriteLine("Socket connected to {0}", client.RemoteEndPoint.ToString());

                // Signal that the connection has been made.  
                connectDone.Set();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        /* b 소켓 생성, 서버 연결 */
        public static void Connect(out TcpClient socket, out NetworkStream stream, string IP, int PORT)
        {
            socket = new TcpClient(IP, PORT);
            stream = socket.GetStream(); // TAP (Task-based Asynchronous Pattern) 

            if (socket != null && stream != null)
                Console.WriteLine("소캣 생성, 스트림 성공, 서버 연결 성공");
            else
            {
                Console.WriteLine("소켓 생성 실패");
                socket.Close();
                stream.Close();
            }
        }

        /* c 수신, 여러 수신 쓰레드 실행 결과을 이번 프로젝트를 통해 실험 */
        public void Read_(NetworkStream stream, int Thd_cnt)
        {
            Dictionary<int, string[]> memb_list = new();
            Dictionary<int, List<string>> chat_save = new();
            Dictionary<UInt32, List<byte[]>> file_save = new();
            Dictionary<UInt32, string> file_name = new();
            Task[] OneD_Task = new Task[Thd_cnt];

            try
            {
                for (int i = 0; i < Thd_cnt; i++)
                {
                    OneD_Task[i] = new Task(() => Read_Save_(stream, memb_list, chat_save, file_save, file_name));
                    OneD_Task[i].Start();
                    OneD_Task[i].Wait();
                }
            }
            catch (AggregateException ae)
            {
                Print_Exception(ae);
                throw;
            }
        }

        // just once, read 1024byte & store it in 1D List 
        public void Read_Save_(NetworkStream stream, 
                            Dictionary<int, string[]>memb_list, Dictionary<int, List<string>> chat_save, 
                            Dictionary<UInt32, List<byte[]>> file_save, Dictionary<UInt32, string> file_name)
        {
            try
            {
                int check = Test_Tcp(stream);

                byte[] packet = new byte[1024];

                while (true)
                {

                    var nbytes = stream.Read(packet, 0, 1024);

                    // 테스트용
                    Read_Save_Sort_(packet, memb_list, chat_save, file_save, file_name, stream);
                    
                    // 테스트 동안 스레드 X
                    // packet을 독립적 쓰레드에서 처리한다.(C#에서는 배열 매개변수는 하드 카피 된다. 넘길때 데이터 신뢰)
                    //Thread process_ = new(() => Read_Save_Sort_(packet, memb_list, chat_save, file_save, file_name));
                    //process_.Start();
                    //process_.Join();

                    if (nbytes <= 0)
                    {
                        stream.Close();
                        break;
                    }

                    Array.Clear(packet, 0, packet.Length);

                }
            }
            catch (IOException ee)
            {
                Print_Exception(ee);
                stream.Close();
                return;
            }
        }

        public event EventHandler<string[]> Evnt_EmailPW_No;
        public event EventHandler<string[]> Evnt_EmailPW_OK_Check_OTP;
        public event EventHandler<string[]> Evnt_OTPOK_Login;
        public event EventHandler<string[]> Evnt_OTP_No;
        public event EventHandler<string[]> Evnt_EmailName_No;
        public event EventHandler<string[]> Evnt_MyProfile;
        public event EventHandler<string[]> Evnt_Join;
        public event EventHandler<string[]> Evnt_RNewFList;
        public event EventHandler<string> Evnt_Profile;
        public event EventHandler<string[]> Evnt_MyPw;

        public void Read_Save_Sort_(byte[] packet, 
            Dictionary<int, string[]> memb_list,     Dictionary<int, List<string>> chat_save,
            Dictionary<UInt32, List<byte[]>> file_save, Dictionary<UInt32, string> file_name, NetworkStream stream)
        {
            int flag = BitConverter.ToInt32(packet, 0); // 패킷 처리 판단
            Console.Write(">> 전달받은 패킷의 {0}\n", flag);
            switch (flag)
            {
                case 1:
                    Read_Save_Sort_Packet(packet, memb_list, Evnt_EmailPW_No); // 이메일 비번 불일치
                    break;

                case 2:
                    Read_Save_Sort_Packet(packet, memb_list, Evnt_EmailPW_OK_Check_OTP);  // 이메일 비번 일치, 이메일 OTP 확인
                    break;

                case 3:
                    Read_Save_Sort_Packet(packet, memb_list, Evnt_OTPOK_Login);  // OTP 성공, 로그인 성공
                    break;

                case 4:
                    Read_Save_Sort_Packet(packet, memb_list, Evnt_OTP_No); // OTP 실패
                    break;

                case 5:
                    Read_Save_Sort_Packet(packet, memb_list, Evnt_OTPOK_Login); // 자동 로그인 성공
                    break;

                case 6:
                    Read_Save_Sort_Packet(packet, memb_list, Evnt_EmailName_No); // 비밀번호 재설정을 위한 이메일, 이름 불일치
                    break;

                case 8:
                    Read_Save_Sort_Packet(packet, memb_list, Evnt_RNewFList);// remove frnd list in serv
                    break;
                case 9:
                    Read_Save_Sort_Packet(packet, memb_list, Evnt_RNewFList);// remove frnd list in serv
                    break;
                case 12:
                    Recv_FReady(packet, file_save, file_name, stream);
                    break;
                case 13:
                    Recv_FData(packet, file_save);
                    break;
                case 14:
                    Recv_FLast(packet, file_save, file_name, Evnt_Profile);
                    break;
                default:
                    Console.WriteLine("c.. 식별값이 없습니다.");
                    break;
            }
            return;
        }

        private void Read_Save_Sort_Packet(byte[] packet, Dictionary<int, string[]> memb_list, EventHandler<string[]> Event)
        {
            int mycode = BitConverter.ToInt32(packet, 4);  // #B2. 받는 사람
            int sender = BitConverter.ToInt32(packet, 8); // #B3. 보내는 사람 or 조각 수
            UInt32 key = BitConverter.ToUInt32(packet, 12); // #B4. key, 시간, 채팅방 번호
            string msg = Encoding.Unicode.GetString(packet, 16, 1008); // #B5. 문장, 바이너리
            Console.WriteLine("도착: {0}", msg);

            string[] info = msg.Split("`");
            //memb_list.Add(my_Code, msg);

            if (Event != null)
            {
                Console.WriteLine("이벤트 발생 내용: {0}", msg);
                Event(this, info);
            }
        }

        private void Recv_FReady(byte[] packet, Dictionary<UInt32, List<byte[]>> file_save, Dictionary<UInt32, string> file_name, NetworkStream stream)
        {
            int me = BitConverter.ToInt32(packet, 4); // 받는 사람(나) 4번재 인덱스부터 4byte(int)읽는다
            int total_frag_cnt = BitConverter.ToInt32(packet, 8); // 총 파일 조각 개수
            UInt32 code = BitConverter.ToUInt32(packet, 12); // 파일 버퍼를 식별할 아이디
            string[] recv_name = Encoding.Unicode.GetString(packet, 16, 1008).Split('\0');

            string name = recv_name[0] + ".png";


            lock (Lock)
            {
                file_name.Add(code, name); // 딕셔너리에 식별값을 키로, 파일 이름을 값으로 저장
            }

            // 2. 파일 조각을 저장 할 리스트 버퍼 생성
            // key:식별, value:파일 조각 만큼의 null 개수를 가진 리스트
            List<byte[]> file = new();
            //byte[] fragment = new byte[1008];
            for (int i = 0; i < total_frag_cnt; i++) // 총 조각의 갯수만큼 
            {
                file.Add(null); // 리스트 개수를 조각 개수만큼 만든다.
            }

            lock(Lock)
            {
                // 3. 생성한 널 리스트 버퍼를 딕셔너리에 추가
                file_save.Add(code, file);
            }

            Console.WriteLine("전송받은 파일 이름은: {0}\n", name);
            Console.WriteLine("생성한 리스트 조각 갯수는: {0}\n", file.Count);
            Console.WriteLine("식별 이름은: {0}\n", code);


            Send_Packet(stream, 13, 0, total_frag_cnt, code, "0", "0", "0", "0");
        }

        private void Recv_FData(byte[] packet, Dictionary<UInt32, List<byte[]>> file_save)
        {
            int index = BitConverter.ToInt32(packet, 8); // 전달 받은 파일의 조각이 들어갈 index
            UInt32 code = BitConverter.ToUInt32(packet, 12);// 파일 버퍼를 식별할 아이디
            Console.WriteLine("전송받은 파일 code:{0}, Index:{1}\n", code, index);
            byte[] fragment = new byte[1008];
            Array.Copy(packet, 16, fragment, 0, 1008);
            lock (Lock)
                (file_save[code])[index] = fragment;


        }

        private async void Recv_FLast(byte[] packet, Dictionary<UInt32, List<byte[]>> file_save, Dictionary<UInt32, string> file_name, EventHandler<string> evnt)
        {
            int fsize = BitConverter.ToInt32(packet, 4); // 전체 바이트 수
            int last_cnt = BitConverter.ToInt32(packet, 8); // 마지막 조각 바이트 갯수
            UInt32 code = BitConverter.ToUInt32(packet, 12);// 파일 버퍼를 식별할 아이디
            int cnt = 0;
            lock (Lock)
                cnt = (file_save[code]).Count; // byte[]이 담긴 리스트 버퍼의 개수

            //마지막 조각 추가
            byte[] fragment = new byte[1008];
            Array.Copy(packet, 16, fragment, 0, last_cnt);
            lock (Lock)
                (file_save[code])[cnt-1] = fragment;

            int check = 1008 * cnt;
            // 모든 바이트 수가 도착했는지 확인
            while (true)
            {
                int recv_byte = 0;
                lock (Lock)
                {
                    foreach (var item in file_save[code])
                    {
                        recv_byte += item.Length;
                        Console.WriteLine(recv_byte);
                    }

                }
                if (recv_byte == check)
                    break;

                await Task.Delay(500); // 마지막 조각을 받으면, 0.5초마다 모든 조각을 받았는지 확인한다.
            }

            ////파일 크기만큼 바이트 배열 준비

            int start = fsize - last_cnt;

            byte[] fbyte = new byte[fsize];  // 파일 크기만큼 바이너리 버퍼 준비

            Console.WriteLine("여긴가?3");
            lock (Lock)
            {
                for (int i = 0; i < (cnt - 1); i++) // 마지막 조각을 제외한 갯수 만큼 복사 반복
                {
                    Array.Copy(file_save[code][i], 0, fbyte, (1008 * i), 1008);
                }
            }
            //마지막 파일 조각 크기 복사
            Array.Copy(packet, 16, fbyte, start, last_cnt);

            //파일 생성
            string dir;
            lock (Lock)
                dir = "C:\\Users\\iot2122\\Downloads\\talktalk\\" + file_name[code];
            File.WriteAllBytes(dir, fbyte);


            if (evnt != null)
            {
                Console.WriteLine("이벤트 발생 내용: {0}", file_name[code]);
                evnt(this, file_name[code]);
            }

            //버퍼 딕셔너리 제거
            lock (Lock)
            {
                file_save.Remove(code);
                file_name.Remove(code);
            }
            Console.WriteLine(dir + " 파일 생성");
        }

        private void UpdateProfile(byte[] packet, int opt, EventHandler<string> Event)
        {
            string msg = Encoding.Unicode.GetString(packet, 16, 1008).TrimEnd('\0');
            string code = Convert.ToString(opt);

            if (Event != null)
            {
                Event(this, msg);
            }
        }











        public int Test_Tcp(NetworkStream stream)
        {
            int case_check = 0;

            Console.WriteLine((string)"\n\nㅡ".PadRight(40, 'ㅡ'));
            Console.WriteLine(" 1. 로그인 실패   2. 이메일확인    3. OTP성공    4.OTP 실패  ");
            Console.WriteLine(" 5. 자동로그인    6. PW재설정      7. 회원가입   ");
            Console.WriteLine(" 8. 친구목록요청  9. 친구 검색    10. 친구 추가    11. 친구 제거");
            Console.WriteLine("12. 채팅 전송    13. 채팅 나가기");
            Console.WriteLine("15. 다운준비     16. 다운종료    000. 테스트 종료\n");

            Console.WriteLine("21. 로그인2    22. 로그인3 \n");
            Console.WriteLine("31. 회원가입2    32. 회원가입3 \n");

            Console.Write("테스트 할 명령 번호 입력 >>");
            string input = Console.ReadLine();


            string str_key = DateTime.Now.ToString("HHmmssfff");
            UInt32 u_key = Convert.ToUInt32(str_key);


            switch (input)
            {
                case "1": // 로그인 정보 전송
                    Send_Packet(stream, 1, 0, 0, u_key, "talktalk.pj.user1@gmail.com", "Talk1234!@#$", "뉴비01", "복권당첨!");
                    Console.Write("OTP 입력 >> ");
                    string otp_input = Console.ReadLine();
                    Send_Packet(stream, 1, 0, 0, u_key, "talktalk.pj.user1@gmail.com", otp_input, "뉴비01", "복권당첨!");
                    break;

                //case "2": // 로그인 재전송시 이메일 확인하라는 애코 flag
                //    Send_Packet(stream, 1, 0, 0, u_key, "talktalk.pj.user1@gmail.com", "Talk1234!@#$", "뉴비01", "복권당첨!");
                //    break;

                case "3": // OTP 전송
                    Console.Write("OTP 입력 >> ");
                    string otp_input2 = Console.ReadLine();
                    Send_Packet(stream, 1, 0, 0, u_key, "talktalk.pj.user1@gmail.com", otp_input2, "뉴비01", "복권당첨!");
                    break;

                //case "4": //  OTP 재입력 확인
                //    Send_Packet(stream, 4, 0, 0, u_key, "talktalk.pj.user1@gmail.com", "Talk1234!@#$", "뉴비01", "복권당첨!");
                //    break;

                case "5": // 자동 로그인
                    Send_Packet(stream, 5, 0, 0, u_key, "talktalk.pj.user1@gmail.com", "Talk1234!@#$", "뉴비01", "복권당첨!");
                    break;

                case "6": // 비밀번호 재설정 요청
                    Send_Packet(stream, 6, 0, 0, u_key, "talktalk.pj.user1@gmail.com", "Talk1234!@#$", "뉴비01", "복권당첨!");
                    break;


                case "9": // 비밀번호 재설정 요청
                    Send_Packet(stream, 9, 0, 0, u_key, "talktalk.pj.user1@gmail.com", "Talk1234!@#$", "뉴비01", "복권당첨!");
                    break;

                case "21": // 로그인 2
                    Send_Packet(stream, 1, 0, 0, u_key, "ubiquitous6g@gmail.com", "Talk1234!@#$", "뉴비02", "복권당첨!");
                    break;
                case "22": // 로그인 3
                    Send_Packet(stream, 1, 0, 0, u_key, "ubiquitous7g@gmail.com", "Talk1234!@#$", "뉴비03", "여행중!");
                    break;

                case "31": // 로그인 2
                    Send_Packet(stream, 5, 0, 0, u_key, "ubiquitous6g@gmail.com", "Talk1234!@#$", "뉴비02", "복권당첨!");
                    break;
                case "32": // 로그인 3
                    Send_Packet(stream, 5, 0, 0, u_key, "ubiquitous7g@gmail.com", "Talk1234!@#$", "뉴비03", "여행중!");
                    break;
                case "000":
                    case_check = -1;
                    Console.WriteLine("종료합니다.");
                    break;
                default:
                    Console.WriteLine("명령어가 없습니다.");
                    break;
            }
            return case_check;
        }

        public void Send_Packet(NetworkStream stream, int flag, int recv, int me, UInt32 key, string email, string pw, string name, string nick)
        {
            byte[] packet = new byte[1024];
            string _l_ = "`";

            // #B1. 플레그
            byte[] b_flag = BitConverter.GetBytes(flag);

            // #B2. 받는 사람
            byte[] b_recv = BitConverter.GetBytes(recv);

            // #B3. 보내는 사람 or 조각 수
            byte[] b_send = BitConverter.GetBytes(me);

            // #B4. key, 시간, 채팅방 번호
            //string str_key = DateTime.Now.ToString("HHmmssfff");
            //UInt32 u_time = Convert.ToUInt32(key);
            byte[] b_key = BitConverter.GetBytes(key);

            // #B5. 문장
            byte[] b_msg = Encoding.Unicode.GetBytes(email + _l_ + pw + _l_ + name + _l_ + nick + _l_);

            // #B6. 바이트 배열 조합
            Array.Copy(b_flag, 0, packet,  0, 4);
            Array.Copy(b_recv, 0, packet,  4, 4);
            Array.Copy(b_send, 0, packet,  8, 4);
            Array.Copy(b_key,  0, packet, 12, 4);
            Array.Copy(b_msg,  0, packet, 16, b_msg.Length);

            // #B7. 전송
            stream.Write(packet, 0, 1024);
            //Console.WriteLine((string)"\nㅡ".PadRight(40, 'ㅡ'));

        }



        ///* 파일 ﻿전송 (서버 <= 클라)
        // * 리턴: 성공 0, 실패 -1
        // * 매개변수: socket, flag, string, option */
        //public int Upload_(Socket socket, string file_name, char flag, char opt, string directory)
        //{
        //    // #B1. 파일 관련 정보
        //    int ret = -1, cnt, rest, file_len, rest_index;
        //    byte flag_to_byte = Convert.ToByte(flag); // 전송체 식별 플레그

        //    // #B2. 파일 스트림 생성
        //    FileInfo target_file = new FileInfo(directory);
        //    FileStream stream = new FileStream(target_file.FullName, FileMode.Open, FileAccess.Read);

        //    // * long -> int 케스팅, 4G 이하의 파일 전송으로 제한한다. 
        //    // * Convert.ToInt32()는 반올림, 명시적 케스팅은 버림 동작한다.
        //    file_len = (int)target_file.Length; // 전체 길이
        //    cnt = file_len / 1020; // 1020씩 보낼때 반복 횟수
        //    rest = file_len % 1020; // 나머지 바이트 수
        //    rest_index = file_len - rest; // 나머지 시작 인덱스

        //    // #B3. 파일을 byte[] 형으로 준비
        //    byte[] file_byte = new byte[file_len];
        //    stream.Read(file_byte, 0, file_byte.Length);

        //    // 파일 정보를 보낸다. >>  이름^전체길이^반복횟수^식별코드
        //    string file_info = file_name + "^" + file_len + "^" + (cnt + 1) + "^" + (int)101 + "^";
        //    Send_(socket, file_info, (char)'d', opt); // 식별자는 ASCII 100 "파일 준비"다.

        //   // // 파일을 병렬로 보낸다. >> 
        //   // Parallel.For(0, cnt, i =>
        //   //{
        //   //     // 병렬로 각각 생성(==lock)
        //   //     byte[] byte1024 = new byte[1024];
        //   //    byte1024[0] = flag_to_byte;
        //   //    int start_index = 1020 * cnt;
        //   //    Buffer.BlockCopy(file_byte, start_index, byte1024, 4, 1020);
        //   //    socket.Send(file_byte, 1024, SocketFlags.None);
        //   //});

        //    //// 나머지 전송
        //    //if(rest != 0)
        //    //{
        //    //    byte[] byte_rest = new byte[rest_index];
        //    //    Buffer.BlockCopy(file_byte, rest_index, byte_rest, 4, rest);
        //    //    socket.Send(file_byte, 1024, SocketFlags.None);
        //    //}

        //    ////서버로부터 byte= 1 데이터가 오면 함수 종료
        //    ////사용 할 수 없다.
        //    //byte[] ret = new byte[1];
        //    //socket.Receive(ret, 1, SocketFlags.None);
        //    //if (ret[0] == 1)
        //    //{
        //    //    Console.WriteLine("Completed");
        //    //}

        //    ret = 0;
        //    Console.WriteLine((string)"\nㅡ".PadRight(40, 'ㅡ'));

        //    return ret;
        //}
    }
}
