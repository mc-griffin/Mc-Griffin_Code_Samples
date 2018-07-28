// UploadArchive

using System;
using System.Text; // StringBuilder
using System.IO; // File
using System.Net.Mail;
// requires references AWSSDK.Core and AWSSDK.Glacier
using Amazon.Glacier;
using Amazon.Glacier.Transfer;
using Amazon.Runtime;


namespace UploadArchive
{
    class Program
    {
        static string s_senderAccount = "auto@company.com";
        static string s_senderPassword = Credentials.GetPassword();
        static string s_recipientAccount = "auto@company.com";

        // Amazon.RegionEndpoint.USEast1 is Virginia
        // Amazon.RegionEndpoint.USWest2 is Oregon

        static string s_vaultName = "Company_Backup";

        static StringBuilder s_log = new StringBuilder();


        static void Main(string[] args)
        {
            Amazon.RegionEndpoint region = null;
            if (!GetDataCenter(out region))
                return;

            WriteLogConsole("To data center: " + region.ToString());

            string filePath;
            if (!GetFilePath(out filePath))
                return;

            int indexLastBackslash = filePath.LastIndexOf(@"\");
            if (indexLastBackslash < 0)
            {
                Console.WriteLine("Can't find the file name in \"{0}\"", filePath);
                Console.WriteLine("Press a key to EXIT");
                Console.ReadKey();
                return;
            }
            string description = filePath.Substring(indexLastBackslash + 1);

            int attempt = 1;
            bool error = false; 
            do
            {
                DateTime startDateTime = DateTime.Now;
                error = false;

                try
                {
                    double archiveSizeInGigabytes = new System.IO.FileInfo(filePath).Length / 1024.0 / 1024.0 / 1024.0;
                    WriteLogConsole(string.Format(
                        "Upload \"{0}\" with size {1:N2} GB", filePath, archiveSizeInGigabytes));
                    WriteLogConsole("Start time: " + startDateTime.ToString("yyyy-MM-dd hh:mm:ss"));

                    var manager = new ArchiveTransferManager(region);
                    string archiveId = manager.Upload(s_vaultName, description, filePath).ArchiveId;
                    WriteLogConsole("Save the following Archive ID: " + archiveId);
                }
                catch (AmazonGlacierException e)
                {
                    WriteLogConsole("AmazonGlacierException: " + e.Message);
                    error = true;
                }
                catch (AmazonServiceException e)
                {
                    WriteLogConsole("AmazonServiceException" + e.Message);
                    error = true;
                }
                catch (Exception e)
                {
                    WriteLogConsole("Exception" + e.Message);
                    error = true;
                }

                DateTime endDateTime = DateTime.Now;
                WriteLogConsole("End time: " + endDateTime.ToString("yyyy-MM-dd hh:mm:ss"));

                TimeSpan timeTaken = endDateTime - startDateTime;
                WriteLogConsole("Time taken: " + timeTaken.ToString());

                Email((error ? "Error: " : "") + "Amazon Upload " + description + " (" + region.ToString() + ")",
                    s_log.ToString());

                if (error)
                {
                    WriteLogConsole(string.Format("Try again.  Attempt number {0}", ++attempt));
                    WriteLogConsole("");
                }
            }
            while (error);

            Console.WriteLine("Press a key to EXIT");
            Console.ReadKey();
        }

        static void Email(string subject, string body)
        {
            try
            {
                MailMessage mail = new MailMessage();
                SmtpClient smtpServer = new SmtpClient("smtp.smarshconnect.com");

                mail.From = new MailAddress(s_senderAccount);
                mail.To.Add(s_recipientAccount);
                mail.Subject = subject;
                mail.Body = body;

                SmtpClient SmtpServer = new SmtpClient("smtp.smarshconnect.com");
                SmtpServer.Port = 587;
                SmtpServer.Credentials = new System.Net.NetworkCredential(s_senderAccount, s_senderPassword);
                SmtpServer.EnableSsl = true;

                SmtpServer.Send(mail);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception: " + Environment.NewLine + ex.ToString());
                // Do not throw this exception because the backup process should continue
                // even if email is not working.
            }
        }


        static bool GetFilePath(out string filePath)
        {
            string reply = "";

            filePath = "";

            do
            {
                try
                {
                    Console.WriteLine("Enter file path without quotation marks:");
                    filePath = Console.ReadLine();

                    Console.WriteLine("Is this file path correct? \"{0}\"", filePath);
                    Console.Write("Type 'y' for yes, 'n' for no, or 'q' or quit >");

                    reply = Console.ReadLine();

                    // if (!File.Exists("\"" + filePath + "\""))
                    if (!File.Exists(filePath))
                    {
                        Console.WriteLine(string.Format("File \"{0}\" does not exist.", filePath));
                        reply = "n";
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine("Exception: " + e.Message);
                }
            }
            while (reply != "y" && reply != "q");

            return reply != "q";
        }


        static bool GetDataCenter(out Amazon.RegionEndpoint dc)
        {
            string reply = "";

            // the compiler requires that dc always be assigned a value
            dc = Amazon.RegionEndpoint.USEast1;

            do
            {
                try
                {
                    Console.WriteLine("Enter 1 for Virginia, 2 for Oregon:");
                    reply = Console.ReadLine();

                    if (reply == "1" || reply == "2")
                    {
                        dc = reply == "1" ? Amazon.RegionEndpoint.USEast1 : Amazon.RegionEndpoint.USWest2;

                        Console.WriteLine("Your choice is \"{0}\"", dc.ToString());
                        Console.Write("Type 'y' for yes, 'n' for no, or 'q' or quit >");

                        reply = Console.ReadLine();
                    }
                    else
                        reply = "x"; // ensure reply isn't "y" or "q"
                }
                catch (Exception e)
                {
                    Console.WriteLine("Exception: " + e.Message);
                }
            }
            while (reply != "y" && reply != "q");

            return reply != "q";
        }


        static void WriteLogConsole(string s)
        {
            s_log.Append(s + Environment.NewLine);
            Console.WriteLine(s);
        }
    }
}
