using System.Net.Mail;
using System.IO;
using System.Net.Sockets;
using System;
using System.Text;
using System.Threading;
using TalkTalk_EmailServ;
using ZXing;
using System.Net;
using System.Threading.Tasks;

class Program
{
    static void Main(string[] args)
    {
        // 서버로 동작시
        TCP.TCP TCP_ = new TCP.TCP();
        TCP_.RunAsyncSocketServer().Wait();

        /*// 클라이언트로서 연결 동작시
        TCP.TCP TCP = new TCP.TCP(); const string IP = "127.0.0.1"; const int PORT = 6363; // 이메일 서버와의 연결 포트
        NetworkStream stream;
        global::TCP.TCP.Connect(out TcpClient socket, out stream, IP, PORT);
        Thread SendEmail_TempHosting = new Thread(() => TCP.Read_Send_Email(stream));
        SendEmail_TempHosting.Start();
        SendEmail_TempHosting.Join();
        socket.Close();
        stream.Close(); */
    }
}

namespace TCP
{
    class TCP
    {
        public async Task RunAsyncSocketServer() // 서버로 동작시 연결 요청 대기
        {
            int MAX_SIZE = 1024;  // 가정

            // (1) 소켓 객체 생성 (TCP 소켓)
            Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            // (2) 포트에 바인드
            IPEndPoint ep = new IPEndPoint(IPAddress.Any, 2323);
            sock.Bind(ep);

            // (3) 포트 Listening 시작
            sock.Listen(5);

            Console.WriteLine("메인 서버의 요청을 대기합니다.");

            while (true)
            {
                // (4) 비동기 소켓 Accept
                Socket clientSock = await Task.Factory.FromAsync(sock.BeginAccept, sock.EndAccept, null);

                // (5) 비동기 소켓 수신
                var packet = new byte[MAX_SIZE];

                int nCount = await Task.Factory.FromAsync<int>(
                           clientSock.BeginReceive(packet, 0, packet.Length, SocketFlags.None, null, clientSock),
                           clientSock.EndReceive);

                if (nCount > 0)
                {
                    int flag = BitConverter.ToInt32(packet, 0); // 패킷 처리 판단
                    Console.Write("요청 수신>> 전달받은 패킷 분류: {0}\n", flag);

                    Thread running = new Thread(() => sorting(clientSock, packet, flag));
                    running.Start();
                    running.Join();
                }

                // (7) 소켓 닫기
                clientSock.Close();
            }
        }

   
        private void sorting(Socket sock, byte[] packet, int flag)
        {
            string res = "";

            if (flag == 3) // OTP 확인
            {
                string msg = Encoding.Unicode.GetString(packet, 16, 1008); // #B5. 문장, 바이너리
                Console.WriteLine("도착: {0}", msg);

                string[] info = msg.Split("`");

                // 이메일 전송
                string email = info[0];
                string otp = info[1];

                // instantiate a writer object
                var barcodeWriter = new BarcodeWriter();

                // set the barcode format
                barcodeWriter.Format = BarcodeFormat.QR_CODE;

                string ddir = @"C:\Users\iot2122\Downloads\OTP\" + email + ".bmp";

                IPHostEntry ipEntry = Dns.GetHostEntry(Dns.GetHostName());
                IPAddress[] addr = ipEntry.AddressList;
                
                Send_OTP_Email(email, otp);
                Console.WriteLine("{0}님께 OTP 확인 이메일을 전송했습니다.", email);

                HttpOTP hosting_otp = new HttpOTP();
                res = hosting_otp.run();

                // 반환 없다.
            }
            else if (flag == 6)
            {
                string msg = Encoding.Unicode.GetString(packet, 16, 1008); // #B5. 문장, 바이너리
                Console.WriteLine("도착: {0}", msg);

                string[] info = msg.Split("`");

                // 이메일 전송
                string email = info[0];
                string pw = info[1];

                Send_PW_Email(email, pw);
                Console.WriteLine("{0}님께 비밀번호 확인 이메일을 전송했습니다.", email);


                HttpPW hosting_pw = new HttpPW();
                res = hosting_pw.run();
                Console.WriteLine("새로운 패스워드는 {0}",  res);
                Send_Packet(sock, flag, 0, 0, 0, res, "", "", "");


            }
            else if (flag == 9)
            {
                string msg = Encoding.Unicode.GetString(packet, 16, 1008); // #B5. 문장, 바이너리
                Console.WriteLine("도착: {0}", msg);

                string[] info = msg.Split("`");

                // 이메일 전송
                string email = info[0];
                string name = info[1];

                Send_Join_Email(email, name);
                Console.WriteLine("{0}님께 회원가입 인증 이메일을 전송했습니다.", email);

                HttpJoin hj = new HttpJoin();
                res = hj.run();

                Send_Packet(sock, flag, 0, 0, 0, res, "", "", "");
            }
        }

        public void Print_Exception(Exception e)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine(e.ToString());
            Console.ResetColor();
        }

        public void Send_OTP_Email(string recv, string OTP)
        {
            MailMessage mail = new MailMessage();


            try
            {
                // 보내는 사람 메일, 이름, 인코딩(UTF-8)
                mail.From = new MailAddress("TalkTalkJoinServer@gmail.com", "TalkTalk 서버", System.Text.Encoding.UTF8);
                // 받는 사람 메일
                mail.To.Add(recv);
                // 참조 사람 메일
                //mail.CC.Add("nowonbun@gmail.com");
                // 비공개 참조 사람 메일
                //mail.Bcc.Add("nowonbun@gmail.com");
                // 메일 제목
                mail.Subject = recv + "님의 2차 인증 OTP 이메일 입니다.";
                // 본문 내용
                mail.Body = "<html><body>" + recv + "님의 OTP 는 " + OTP + " 입니다. <p> QR을 클릭하시면 OTP 확인 페이지로 이동합니다. </body></html>";
                // 본문 내용 포멧의 타입 (true의 경우 Html 포멧으로)
                mail.IsBodyHtml = true;
                // 메일 제목과 본문의 인코딩 타입(UTF-8)
                mail.SubjectEncoding = System.Text.Encoding.UTF8;
                mail.BodyEncoding = System.Text.Encoding.UTF8;
                // 첨부 파일 (Stream과 파일 이름)
                mail.Attachments.Add(new Attachment(new FileStream(@"C:\Users\iot2122\Downloads\OTP\" + recv + ".bmp", FileMode.Open, FileAccess.Read), recv + ".bmp"));
                //mail.Attachments.Add(new Attachment(new FileStream(@"D:\test2.zip", FileMode.Open, FileAccess.Read), "test2.zip"));
                // smtp 서버 주소
                SmtpClient SmtpServer = new SmtpClient("smtp.gmail.com");
                // smtp 포트
                SmtpServer.Port = 587;
                // smtp 인증
                SmtpServer.Credentials = new System.Net.NetworkCredential("TalkTalkJoinServer@gmail.com", "Talk1234!@#$");
                // SSL 사용 여부
                SmtpServer.EnableSsl = true;
                // 발송
                SmtpServer.Send(mail);
            }
            finally
            {
                Console.WriteLine("{0}에게 이메일 전송을 완료했습니다.", recv);
                // 첨부 파일 Stream 닫기
                foreach (var attach in mail.Attachments)
                {
                    attach.ContentStream.Close();
                }
            }
        }

        public void Send_PW_Email(string recv, string pw)
        {
            MailMessage mail = new MailMessage();
            try
            {
                // 보내는 사람 메일, 이름, 인코딩(UTF-8)
                mail.From = new MailAddress("TalkTalkJoinServer@gmail.com", "TalkTalk 서버", System.Text.Encoding.UTF8);
                // 받는 사람 메일
                mail.To.Add(recv);
                // 참조 사람 메일
                //mail.CC.Add("nowonbun@gmail.com");
                // 비공개 참조 사람 메일
                //mail.Bcc.Add("nowonbun@gmail.com");
                // 메일 제목
                mail.Subject = recv + "님의 비밀번호 확인 이메일 입니다.";
                // 본문 내용
                mail.Body = "<html><body>" + recv + "님의 비밀번호 변경 링크입니다. <p>링크를 클릭하시면 이메일 등록 페이지로 이동합니다." + "<p> http://localhost:7700/ </body></html>";
                // 본문 내용 포멧의 타입 (true의 경우 Html 포멧으로)
                mail.IsBodyHtml = true;
                // 메일 제목과 본문의 인코딩 타입(UTF-8)
                mail.SubjectEncoding = System.Text.Encoding.UTF8;
                mail.BodyEncoding = System.Text.Encoding.UTF8;
                // 첨부 파일 (Stream과 파일 이름)
                mail.Attachments.Add(new Attachment(new FileStream(@"C:\Users\iot2122\Downloads\img\password.png", FileMode.Open, FileAccess.Read), "password.png"));
                //mail.Attachments.Add(new Attachment(new FileStream(@"D:\test2.zip", FileMode.Open, FileAccess.Read), "test2.zip"));
                // smtp 서버 주소
                SmtpClient SmtpServer = new SmtpClient("smtp.gmail.com");
                // smtp 포트
                SmtpServer.Port = 587;
                // smtp 인증
                SmtpServer.Credentials = new System.Net.NetworkCredential("TalkTalkJoinServer@gmail.com", "Talk1234!@#$");
                // SSL 사용 여부
                SmtpServer.EnableSsl = true;
                // 발송
                SmtpServer.Send(mail);
            }
            finally
            {
                Console.WriteLine("{0}에게 이메일 전송을 완료했습니다.", recv);
                // 첨부 파일 Stream 닫기
                foreach (var attach in mail.Attachments)
                {
                    attach.ContentStream.Close();
                }
            }
        }

        public void Send_Join_Email(string recv, string name)
        {
            MailMessage mail = new MailMessage();
            try
            {
                // 보내는 사람 메일, 이름, 인코딩(UTF-8)
                mail.From = new MailAddress("TalkTalkJoinServer@gmail.com", "TalkTalk 서버", System.Text.Encoding.UTF8);
                // 받는 사람 메일
                mail.To.Add(recv);
                // 참조 사람 메일
                //mail.CC.Add("nowonbun@gmail.com");
                // 비공개 참조 사람 메일
                //mail.Bcc.Add("nowonbun@gmail.com");
                // 메일 제목
                mail.Subject = name + "님의 이메일을 등록해주세요.";
                // 본문 내용
                mail.Body = "<html><body<p>" + name + "님 환영합니다.<p>" + name + "님의 인증 이메일은 " + recv + " 입니다." + "<p> 링크를 클릭하시면 이메일 등록 페이지로 이동합니다." + "<p> http://localhost:7070/ </body></html>";
                // 본문 내용 포멧의 타입 (true의 경우 Html 포멧으로)
                mail.IsBodyHtml = true;
                // 메일 제목과 본문의 인코딩 타입(UTF-8)
                mail.SubjectEncoding = System.Text.Encoding.UTF8;
                mail.BodyEncoding = System.Text.Encoding.UTF8;
                // 첨부 파일 (Stream과 파일 이름)
                mail.Attachments.Add(new Attachment(new FileStream(@"C:\Users\iot2122\Downloads\img\welcome.png", FileMode.Open, FileAccess.Read), "welcome.png"));
                //mail.Attachments.Add(new Attachment(new FileStream(@"D:\test2.zip", FileMode.Open, FileAccess.Read), "test2.zip"));
                // smtp 서버 주소
                SmtpClient SmtpServer = new SmtpClient("smtp.gmail.com");
                // smtp 포트
                SmtpServer.Port = 587;
                // smtp 인증
                SmtpServer.Credentials = new System.Net.NetworkCredential("TalkTalkJoinServer@gmail.com", "Talk1234!@#$");
                // SSL 사용 여부
                SmtpServer.EnableSsl = true;
                // 발송
                SmtpServer.Send(mail);
            }
            finally
            {
                Console.WriteLine("{0}에게 이메일 전송을 완료했습니다.", recv);
                // 첨부 파일 Stream 닫기
                foreach (var attach in mail.Attachments)
                {
                    attach.ContentStream.Close();
                }
            }
        }

        public void Send_Packet(Socket sock, int flag, int recv, int me, UInt32 key, string email, string pw, string name, string nick)
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
            Array.Copy(b_flag, 0, packet, 0, 4);
            Array.Copy(b_recv, 0, packet, 4, 4);
            Array.Copy(b_send, 0, packet, 8, 4);
            Array.Copy(b_key, 0, packet, 12, 4);
            Array.Copy(b_msg, 0, packet, 16, b_msg.Length);

            // #B7. 전송
            sock.Send(packet);
            //Console.WriteLine((string)"\nㅡ".PadRight(40, 'ㅡ'));

        }

        /* b 소켓 생성, 서버 수신시 */
        public static void Connect(out TcpClient socket, out NetworkStream stream, string IP, int PORT)
        {
            socket = new TcpClient(IP, PORT);
            stream = socket.GetStream(); // TAP (Task-based Asynchronous Pattern) 

            if (socket != null && stream != null)
                Console.WriteLine("소캣 생성, 스트림 성공, 메인 서버 연결 성공");
            else
            {
                Console.WriteLine("소켓 생성 실패");
                socket.Close();
                stream.Close();
            }
        }
        public void Read_Send_Email(NetworkStream stream)
        {
            try
            {
                byte[] packet = new byte[1024];

                while (true)
                {

                    Console.WriteLine("메인 서버의 요청을 대기합니다.");
                    var nbytes = stream.Read(packet, 0, 1024);

                    int flag = BitConverter.ToInt32(packet, 0); // 패킷 처리 판단
                    Console.Write("요청 수신>> 전달받은 패킷 분류: {0}\n", flag);

                    //Thread running = new Thread(() => sorting( packet, flag));
                    //running.Start();
                    //running.Join();

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


    }
}