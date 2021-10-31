using System;
using System.Text;
using System.Net;
using System.Threading.Tasks;

namespace TCP
{
    class HttpJoin
    {
        public static HttpListener listener;
        public static string url = "http://localhost:7770/";
        public static string pageViews = "";
        public static string next = "";
        public static int requestCount = 0;
        public static string pageData =
            "<!DOCTYPE>" +
            "<html lang=\"ko\">" +
            "  <head>" +
            "   <meta charset=\"UTF-8\">" +
            "    <title>TalkTalk</title>" +
            "  </head>" +
            "  <body>" +
            "    <p>회원가입 인증입니다.</p>" +
            "    <p>이메일을 등록 하시려면,</p>" +
            "    <p>아래 버튼을 클릭하세요</p>" +
            "    <form method=\"post\" action=\"shutdown\">" +
            "      <input type=\"submit\" value=\"회원 가입 인증\" {1}>" +
            "    </form>" +
            "    <p>{0}</p>" +
            "  </body>" +
            "</html>";


        public async static Task<string> HandleIncomingConnections()
        {
            bool runServer = true;
            string ret = "";
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
                    Console.WriteLine("Shutdown requested");
                    pageViews = "<p>이메일 등록 완료.<p>로그인하세요.";
                    runServer = false;

                    ret = "인증 완료";
                }

                // Make sure we don't increment the page views counter if `favicon.ico` is requested
                //if (req.Url.AbsolutePath != "/favicon.ico")
                //pageViews = "인증 완료";

                // Write the response info
                string disableSubmit = !runServer ? "disabled" : "";
                byte[] data = Encoding.UTF8.GetBytes(String.Format(pageData, pageViews, disableSubmit));
                resp.ContentType = "text/html";
                resp.ContentEncoding = Encoding.UTF8;
                resp.ContentLength64 = data.LongLength;

                // Write out to the response stream (asynchronously), then close it
                await resp.OutputStream.WriteAsync(data, 0, data.Length);
                resp.Close();
            }
            return ret;
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

            string result = listenTask.Result.ToString();


            // Close the listener
            listener.Close();

            return result;
        }
    }
}