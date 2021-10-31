using System;
using System.Text;
using System.Net;
using System.Threading.Tasks;

namespace TCP
{
    class HttpPW
    {
        public static HttpListener listener;
        public static string url = "http://localhost:7700/";
        public static string pageViews = "";
        public static string next = "";
        public static int requestCount = 0;
        public static string html_default =
            "<!DOCTYPE>" +
            "<html lang=\"ko\">" +
            "  <head>" +
            "   <meta charset=\"UTF-8\">" +
            "    <title>TalkTalk</title>" +
            "  </head>" +
            "  <body>" +
            "    <h3>비밀번호 재설정 페이지 입니다.</h3>" +
            "    <i>10자 이상의 비밀번호를 입력해주세요.</i><br><br>" +
            "    <form action=\"shutdown\" method=\"post\" >" +
            "       비밀번호 입력 : <input type = \"password\" placeholder=\"비밀번호를 입력하세요\" name=\"pw1\" id=\"pw11\" style=\" margin:4pt;\" ><br>" +
            "       비밀번호 확인 : <input type = \"password\" placeholder=\"비밀번호를 재입력하세요\" name=\"pw2\" id=\"pw22\" style=\" margin:4pt;\" ><br>" +
            "       <input type=\"submit\" value=\"비밀번호 재설정\" {1} style=\"width:224pt; margin-top:6px\" >" +
            "    </form>" +
            "    {0}" +
            "  </body>" +
            "</html>";



        public async static Task<string> HandleIncomingConnections()
        {
            bool runServer = true;
            string pw1 = "";
            string pw2 = "";

            // While a user hasn't visited the `shutdown` url, keep on handling requests
            while (runServer)
            {
                // Will wait here until we hear from a connection
                HttpListenerContext ctx = await listener.GetContextAsync();

                // Peel out the requests and response objects
                HttpListenerRequest req = ctx.Request;
                HttpListenerResponse resp = ctx.Response;

                // Print out some info about the request
                //Console.WriteLine("Request #: {0}", ++requestCount);
                Console.WriteLine(req.Url.ToString());
                Console.WriteLine(req.HttpMethod);
                Console.WriteLine(req.UserHostName);
                Console.WriteLine(req.UserAgent);
                Console.WriteLine();

                // If `shutdown` url requested w/ POST, then shutdown the server after serving the page
                if ((req.HttpMethod == "POST") && (req.Url.AbsolutePath == "/shutdown"))
                {
                    byte[] data2 = new byte[1024];
                    Console.WriteLine("읽어들임 {0}", req.InputStream.ReadAsync(data2, 0, data2.Length));

                    string res = Encoding.Default.GetString(data2);
                    Console.WriteLine("전달받은 문장:{0}", res);
                    pw1 = res.Substring(4, res.IndexOf("&") - 4);
                    pw2 = res.Substring(res.IndexOf("&") + 5, (res.IndexOf("\0") - (res.IndexOf("&") + 5)));
                    Console.WriteLine("pw1:{0}, {1}", pw1, pw1.Length);
                    Console.WriteLine("pw2:{0}, {1}", pw2, pw2.Length);

                    if (pw1 == pw2 && pw1.Length > 10)
                    {
                        Console.WriteLine("비밀번호를 재설정했습니다.");
                        html_default =
                                    "<!DOCTYPE>" +
                                    "<html lang=\"ko\">" +
                                    "  <head>" +
                                    "   <meta charset=\"UTF-8\">" +
                                    "    <title>TalkTalk</title>" +
                                    "  </head>" +
                                    "  <body>" +
                                    "    <p>비밀번호를 재설정했습니다.</p>" +
                                    "    <p>새로운 비밀번호로 로그인 하세요.</p>" +
                                    "    {0}" +
                                    "  </body>" +
                                    "</html>";
                        pageViews = "<script type=\"text/javascript\">" +
                                    "    alert(\"비밀번호를 재설정했습니다.\\n새로운 비밀번호로 로그인 하세요.\");" +
                                    "</script>";

                        runServer = false;
                    }
                    else if (pw1.Length < 10)
                    {
                        Console.WriteLine("입력된 비밀번호가 10자 이하 입니다.");
                        pageViews = "<script type=\"text/javascript\">" +
                                    "    alert(\"비밀번호를 10자 이상 입력해주세요.\");" +
                                    "</script>";
                    }
                    else if (pw1 != pw2)
                    {
                        Console.WriteLine("비밀번호가 일치하지 않습니다.");
                        pageViews = "<script type=\"text/javascript\">" +
                                    "    alert(\"비밀번호가 일치하지 않습니다.\");" +
                                    "</script>";
                    }
                }

                // Make sure we don't increment the page views counter if `favicon.ico` is requested
                //if (req.Url.AbsolutePath != "/favicon.ico")
                //pageViews += 1;

                // Write the response info
                string disableSubmit = !runServer ? "disabled" : "";
                byte[] data = Encoding.UTF8.GetBytes(String.Format(html_default, pageViews, disableSubmit));
                resp.ContentType = "text/html";
                resp.ContentEncoding = Encoding.UTF8;
                resp.ContentLength64 = data.LongLength;

                // Write out to the response stream (asynchronously), then close it
                await resp.OutputStream.WriteAsync(data, 0, data.Length);
                resp.Close();

            }

            return pw1;
        }

        public string run()
        {
            // Create a Http server and start listening for incoming connections
            listener = new HttpListener();
            listener.Prefixes.Add(url);
            listener.Start();
            Console.WriteLine("Listening for connections on {0}", url);

            // Handle requests
            Task<string> listenTask = HandleIncomingConnections();
            listenTask.GetAwaiter().GetResult();

            string new_pw = listenTask.Result.ToString();

            // Close the listener
            listener.Close();

            return new_pw;
        }
    }
}