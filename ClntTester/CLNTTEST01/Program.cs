using System;
using System.IO;
using System.Net.Sockets;
namespace main
{
    class Program
    {

        static void Main(string[] args)
        {

            Console.WriteLine("a.. C# 클라이언트 테스트를 시작합니다.");

            /* a 연결 준비 */
            TCP.TCP TCP = new(); const string IP = "127.0.0.1"; const int PORT = 9090;
            TcpClient socket = null;
            NetworkStream stream = null;

            try
            {
                /* b 소켓 연결 */
                global::TCP.TCP.Connect(out socket, out stream, IP, PORT);

                /* c 수신  */
                int Thd_cnt = 1; // 수신 스레드 개수
                TCP.Read_(stream, Thd_cnt);
            }
            catch (SocketException se)
            {
                // 인터넷 접속이 안되는 경우에 대한 처리
                TCP.Print_Exception(se);
                socket.Close();
            }
            catch (EndOfStreamException ee)
            {
                TCP.Print_Exception(ee);
                stream.Close();
            }
            finally
            {
                socket.Close();
                stream.Close();
            }

            //return;
        }
    }
}
