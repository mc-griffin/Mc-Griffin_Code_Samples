// NightlyBackup

using System;
using System.Collections.Generic; // List
using System.Text; // StringBuilder
using System.IO; // Path
using System.Threading; // Sleep
using System.Data.SqlClient;
using System.Net.Mail;
using System.Diagnostics;
using System.Reflection; // MethodBase.GetCurrentMethod().DeclaringType.Namespace
// requires references AWSSDK.Core and AWSSDK.Glacier
using Amazon.Glacier;
using Amazon.Glacier.Transfer;
using Amazon.Runtime;


// Drive mappings don't work when a program is run as a service so to access the servers
// you must use UNC naming.


namespace NightlyBackup
{
    class Program
    {
        static bool s_use_Napa_7z = true;

        static string s_7zPassword = Credentials.Get7zPassword();

        static string s_destDrive = @"D:\";
        static string s_destFolder = "Company_backup_";
        static string s_destFolderPath = "";

        static string s_7z_flags = string.Format("a -t7z -p{0} -r -mx=9 -myx=9 -m0=LZMA2", s_7zPassword);
        static string s_7zFilePath = "\"" + @"C:\Program Files\7-Zip\7z.exe" + "\"";


        // An ArchivePart is a set of files in a folder
        class ArchivePart
        {
            public List<string> _files = new List<string>();
            public string SourceFolder { get; set; }
            public string UncPath { get; set; }
            public decimal SizeGBs { get; set; } // in GBs

            public ArchivePart(string uncPath, string sourceFolder)
                { UncPath = uncPath; SourceFolder = sourceFolder; SizeGBs = 0; }
        }

        class Archive
        {
            public List<ArchivePart> _parts = new List<ArchivePart>();
            // The applications and development archives have already been compressed and encrypted with 7z
            // so there is no need to do it again.
            // The archives from the applications servers for apx and eze have already had 7z applied but
            // 7z is need to combine the application server backup with the corresponding database backup.
            public bool Use7z { get; set; }
            public decimal SizeGBs  // in GBs
            {
                get
                {
                    decimal sizeGBs = 0;
                    foreach (ArchivePart part in _parts)
                        sizeGBs += part.SizeGBs;
                    return sizeGBs;
                }

            }
            public int FileCount
            {
                get
                {
                    int fileCount = 0;
                    foreach (ArchivePart part in _parts)
                        fileCount += part._files.Count;
                    return fileCount;
                }
            }
            public List<string> Files
            {
                get
                {
                    List<string> files = new List<string>();
                    foreach (ArchivePart part in _parts)
                        files.AddRange(part._files);
                    return files;
                }
            }

            public Archive(List<ArchivePart> parts, bool use7z) { _parts = parts; Use7z = use7z; }
        }

        static Dictionary<string, Archive> s_archives = new Dictionary<string, Archive>()
            {
                /* applications is so big that is causes network timeouts when I try to upload it.
                 * Therefore it must be split into smaller pieces.  Thus it must be handled separately.
                 * See GetApplicationsSources().
                 */
                { "Company_development_",
                    new Archive(new List<ArchivePart> {
                        new ArchivePart(@"\\novato\development\Backup", "development") }, false ) },
                { "Company_apx_", 
                    new Archive(
                        new List<ArchivePart> {
                            new ArchivePart(@"\\apx-db\Backup", "apx-db"),
                            new ArchivePart(@"\\apx-app\Backup", "apx-app") }, true ) },
                { "Company_eze_",
                    new Archive(
                        new List<ArchivePart> {
                            new ArchivePart(@"\\eze-db\Backup", "eze-db"),
                            new ArchivePart(@"\\eze-app\Backup", "eze-app") }, true ) },
                { "Company_tiburon_", 
                    new Archive(new List<ArchivePart> {
                        new ArchivePart(@"\\tiburon\DB", "tiburon") }, true ) },
            };

        static Dictionary<string, Archive> s_applicationsArchives = new Dictionary<string, Archive>();

        // Amazon.RegionEndpoint.USEast1 is Virginia
        // Amazon.RegionEndpoint.USWest2 is Oregon
        static List<Amazon.RegionEndpoint> s_datacenters = new List<Amazon.RegionEndpoint>()
            { Amazon.RegionEndpoint.USEast1, Amazon.RegionEndpoint.USWest2 };

        static string s_vaultName = "Company_Backup";

        static string s_senderAccount = "auto@company.com";
        static string s_senderPassword = Credentials.GetPassword();
        static string s_recipientAccount = "auto@company.com";

        static string s_programName = MethodBase.GetCurrentMethod().DeclaringType.Namespace;

        static int s_processesAlive = 0;

        static bool s_userInput = false;

        static StringBuilder s_log = new StringBuilder();


        static void Main(string[] args)
        {
            DateTime startDateTime = DateTime.MinValue;

            try
            {
                DateTime yearMonthDate = DateTime.MinValue;

                // Any command line parameters will initiate user interface
                String[] arguments = Environment.GetCommandLineArgs();
                if (arguments.Length <= 1)
                {
                    DateTime previousWorkDayDateTime = GetPreviousWorkDay(DateTime.Now);
                    yearMonthDate = new DateTime(previousWorkDayDateTime.Year, previousWorkDayDateTime.Month, previousWorkDayDateTime.Day);
                }
                else
                {
                    s_userInput = true;

                    string year, month, date;
                    if (!GetYearMonthDate(out year, out month, out date))
                        return;

                    yearMonthDate = new DateTime(Int32.Parse(year), Int32.Parse(month), Int32.Parse(date));
                }

                SetStatics(yearMonthDate);
                CreateFolder(yearMonthDate);

                if (!GetSources(yearMonthDate))
                    return;

                startDateTime = DateTime.Now;
                WriteLogConsole("Start time: " + startDateTime.ToString("yyyy-MM-dd hh:mm:ss"));

                // If more about 6 processes run then some of them fail silently.
                // So break up the process into multiple batches.
                CreateArchives(s_archives, yearMonthDate);
                CheckArchives(s_archives);

                CreateArchives(s_applicationsArchives, yearMonthDate);
                CheckArchives(s_applicationsArchives);

                UploadArchives();
            }
            catch (Exception e)
            {
                WriteLogConsole("Exception: " + e.Message);
            }

            DateTime endDateTime = DateTime.Now;
            WriteLogConsole("End time: " + endDateTime.ToString("yyyy-MM-dd hh:mm:ss"));

            TimeSpan timeTaken = endDateTime - startDateTime;
            WriteLogConsole("Time taken: " + timeTaken.ToString());

            Email(string.Format("Amazon {0} Completion", s_programName), s_log.ToString());

            if (s_userInput)
            {
                Console.WriteLine("Press a key to exit");
                Console.ReadKey();
            }
        }


        static void UploadArchives()
        {
            string[] files = Directory.GetFiles(s_destFolderPath, "*.7z");

            foreach (Amazon.RegionEndpoint datacenter in s_datacenters)
            {
                foreach (string filePath in files)
                {
                    // These all must be tasks
                    // UploadArchive(filePath, datacenter);

                    bool queued = ThreadPool.QueueUserWorkItem((o) => UploadArchive(filePath, datacenter));
                    if (queued)
                    {
                        s_processesAlive = Interlocked.Add(ref s_processesAlive, 1);
                        WriteLogConsole(string.Format("UploadArchive({0}, {1}) successfully queued ({2} processes)", filePath, datacenter, s_processesAlive));
                     }
                    else
                        WriteLogConsole(string.Format("UploadArchive({0}, {1}) FAILED to queue", filePath, datacenter));

                    // Wait 5 seconds to decrease the chance that log messages collide
                    Thread.Sleep(5000);
                }
            }

            WriteLogConsole("Finished with queueing");
            WriteLogConsole("Wait for all processes to complete");

            while (s_processesAlive != 0)
            {
                WriteLogConsole(string.Format("{0} processes alive (1)", s_processesAlive));
                Thread.Sleep(90000); // sleep 15 minutes
            }

            WriteLogConsole("All processes completed");
        }

        static void UploadArchive(string filePath, Amazon.RegionEndpoint region)
        {
            bool error = false;
            int attempt = 1;
            do
            {
                error = false;

                int indexLastBackslash = filePath.LastIndexOf(@"\");
                if (indexLastBackslash < 0)
                {
                    Console.WriteLine("Can't find the file name in \"{0}\"", filePath);
                    Console.WriteLine("Press a key to EXIT");
                    Console.ReadKey();
                    return;
                }
                string description = filePath.Substring(indexLastBackslash + 1);

                DateTime startDateTime = DateTime.Now;
                try
                {
                    double archiveSizeInGigabytes = new System.IO.FileInfo(filePath).Length / 1024.0 / 1024.0 / 1024.0;
                    WriteLogConsole(string.Format(
                        "Upload \"{0}\" with size {1:N2} GB to {2}", filePath, archiveSizeInGigabytes, region.ToString()));
                    WriteLogConsole("Start time: " + startDateTime.ToString("yyyy-MM-dd hh:mm:ss"));

                    var manager = new ArchiveTransferManager(region);
                    string archiveId = manager.Upload(s_vaultName, description, filePath).ArchiveId;
                    WriteLogConsole("Save the following Archive ID for " + filePath + " to " + region.ToString() + ":");
                    WriteLogConsole(archiveId);
                }
                catch (AmazonGlacierException e)
                {
                    WriteLogConsole("AmazonGlacierException for " + filePath + " to " + region.ToString() + ":" + e.Message);
                    error = true;
                }
                catch (AmazonServiceException e)
                {
                    WriteLogConsole("AmazonServiceException for " + filePath + " to " + region.ToString() + ":" + e.Message);
                    error = true;
                }
                catch (Exception e)
                {
                    WriteLogConsole("Exception for " + filePath + " to " + region.ToString() + ":" + e.Message);
                    error = true;
                }

                DateTime endDateTime = DateTime.Now;
                WriteLogConsole("End time: " + endDateTime.ToString("yyyy-MM-dd hh:mm:ss"));

                TimeSpan timeTaken = endDateTime - startDateTime;
                WriteLogConsole("Time taken: " + timeTaken.ToString());

                Email((error ? "Error: " : "")
                    + string.Format("Amazon {0}: ", s_programName)
                    + description + " (" + region.ToString() + ")", s_log.ToString());

                if (error)
                {
                    WriteLogConsole(string.Format("Try again.  Attempt number {0}. {1}, {2}", ++attempt, filePath, region.ToString()));
                    WriteLogConsole("");
                }
            }
            while (error);

            s_processesAlive = Interlocked.Add(ref s_processesAlive, -1);
            WriteLogConsole(string.Format("Completed {0} to {1}. {2} processes alive (2)", filePath, region.ToString(), s_processesAlive));
        }


        static void CreateArchives(Dictionary<string, Archive> archives, DateTime yearMonthDay)
        {
            List<Process> processes = new List<Process>();
            Dictionary<Process, string> processNames = new Dictionary<Process, string>();

            WriteLogConsole("");
            WriteLogConsole("CreateArchives");
            WriteLogConsole("");

            foreach (string archive in archives.Keys)
            {
                if (archives[archive].FileCount < 1)
                    continue;

                WriteLogConsole(string.Format("Archive: {0}", archive));

                if (!archives[archive].Use7z)
                {
                    Debug.Assert(archives[archive].FileCount == 1);
                    System.IO.File.Copy(archives[archive].Files[0], Path.Combine(s_destFolderPath, archive));

                    WriteLogConsole("");
                    continue;
                }

                Process process = CreateArchive(archive, archives[archive].Files);
                if (process != null)
                {
                    processes.Add(process);
                    processNames[process] = archive;
                }

                WriteLogConsole("");
            }

            foreach (Process process in processes)
            {
                WriteLogConsole(string.Format("Start() process {0}", processNames[process]));
                // throws
                // Console.WriteLine("Start() on {0}", process.ProcessName);
                process.Start();

                // This program fails to create archives even though the console output show their creation.
                // This is code is a guess as to a solution.  Wait after each processes starts, to 
                // decrease the chances that they stop on eachother.
                Thread.Sleep(5000);
            }

            // The following code has the intended effect of waiting until all 7z processes have completed
            // so that this program can accurately time how long the entire procedure took.
            //
            // Without a WaitForExit(), execution continues while the other processes runs asynchronously,
            // which prevents timing the work.
            //
            // Each call to WaitForExit() blocks, which is what we want.  However, I was concerned that
            // this program may block waiting on a process A before this program can call WaitForExit()
            // for process B.  But it doesn't matter if a process completes before WaitForExit() is called.

            foreach (Process process in processes)
            {
                WriteLogConsole(string.Format("WaitForExit() on process {0}", processNames[process]));
                // throws
                // Console.WriteLine("WaitForExit() on {0}", process.ProcessName);
                process.WaitForExit();
            }

            WriteLogConsole("");
        }

        static Process CreateArchive(string archive, List<string> files)
        {
            foreach (string file in files)
                WriteLogConsole(file);

            Process process = new Process();
            ProcessStartInfo startInfo = new ProcessStartInfo();
            // startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;

            startInfo.FileName = s_7zFilePath;

            StringBuilder arguments = new StringBuilder(s_7z_flags + " " + "\"" + Path.Combine(s_destFolderPath, archive) + "\"");
            // StringBuilder arguments = new StringBuilder("/C a -t7z -pm1adisoN! -r -mx=9 -myx=9 -m0=LZMA2 " + "\"" + Path.Combine(s_destFolderPath, s_7zFileName) + "\"");
            foreach (string sourcePath in files)
                arguments.Append(" \"" + sourcePath + "\"");

            startInfo.Arguments = arguments.ToString();

            WriteLogConsole("command: " + startInfo.FileName);
            WriteLogConsole("arguments: " + startInfo.Arguments);

            process.StartInfo = startInfo;

            return process;
        }

        static void CheckArchives(Dictionary<string, Archive> archives)
        {
            foreach (string archive in archives.Keys)
            {
                if (archives[archive].FileCount < 1)
                    continue;

                foreach (string filePath in archives[archive].Files)
                    if (!File.Exists(filePath))
                        Console.WriteLine("{0} does NOT exist", filePath);
            }
        }


        static void GetApplicationsSources(DateTime yearMonthDay)
        {
            const string applicationsFolder = "applications";
            const string uncPath = @"\\novato\applications\Backup";

            // assume applications sources are at drive F:
            foreach (string filePath in GetFiles(uncPath, yearMonthDay))
            {
                if (!filePath.EndsWith("7z"))
                    continue;

                ArchivePart part = new ArchivePart(uncPath, applicationsFolder);
                part._files = new List<string> { filePath };

                Archive archive = new Archive(new List<ArchivePart> { part }, false);

                string archiveName = "Company_" + filePath.Substring(filePath.LastIndexOf(@"\") + 1);

                s_applicationsArchives[archiveName] = archive;
            }
        }


        static bool GetSources(DateTime yearMonthDay)
        {
            // count all the files in all the archives and the total size
            int fileCount = 0;
            decimal sizeGBs = 0;

            foreach (string archiveName in s_archives.Keys)
            {
                Archive archive = s_archives[archiveName]; // throws is index is not found
                foreach (ArchivePart part in archive._parts)
                {
                    part._files.AddRange(GetFiles(part.UncPath, yearMonthDay));
                    // Compute archive size
                    foreach (string filePath in part._files)
                    {
                        double size = new System.IO.FileInfo(filePath).Length / 1024.0 / 1024.0 / 1024.0;
                        // review what is the more responsible conversions
                        part.SizeGBs += (decimal)size;
                        sizeGBs += (decimal)size;
                    }
                    fileCount += part._files.Count;
                }
            }

            // handle the applications case
            GetApplicationsSources(yearMonthDay);
            foreach (string archiveName in s_applicationsArchives.Keys)
            {
                Archive archive = s_applicationsArchives[archiveName]; // throws is index is not found
                foreach (ArchivePart part in archive._parts)
                {
                    foreach (string filePath in part._files)
                    {
                        double size = new System.IO.FileInfo(filePath).Length / 1024.0 / 1024.0 / 1024.0;
                        // review what is the more responsible conversions
                        part.SizeGBs += (decimal)size;
                        sizeGBs += (decimal)size;
                    }
                    fileCount += part._files.Count;
                }
            }

            WriteLogConsole("");
            WriteLogConsole(string.Format(
                "{0} archives containing a total of {1} files with a total size of {2:N2} GB",
                s_archives.Keys.Count + s_applicationsArchives.Count, fileCount, sizeGBs));
            WriteLogConsole("");

            foreach (string archiveName in s_archives.Keys)
            {
                Archive archive = s_archives[archiveName]; // throws is index is not found
                WriteLogConsole(string.Format(
                    "\"{0}\": {1} files, size {2:N2} GB", archiveName, archive.FileCount, archive.SizeGBs));

                foreach (ArchivePart part in archive._parts)
                {
                    foreach (string filePath in part._files)
                    {
                        double fileSizeInGigabytes = new System.IO.FileInfo(filePath).Length / 1024.0 / 1024.0 / 1024.0;
                        WriteLogConsole(string.Format(
                            "\"{0}\": size {1:N2} GB", filePath, fileSizeInGigabytes));
                    }
                }
                WriteLogConsole("");
            }

            foreach (string archiveName in s_applicationsArchives.Keys)
            {
                Archive archive = s_applicationsArchives[archiveName]; // throws is index is not found
                WriteLogConsole(string.Format(
                    "\"{0}\": {1} files, size {2:N2} GB", archiveName, archive.FileCount, archive.SizeGBs));

                foreach (ArchivePart part in archive._parts)
                {
                    foreach (string filePath in part._files)
                    {
                        double fileSizeInGigabytes = new System.IO.FileInfo(filePath).Length / 1024.0 / 1024.0 / 1024.0;
                        WriteLogConsole(string.Format(
                            "\"{0}\": size {1:N2} GB", filePath, fileSizeInGigabytes));
                    }
                }
                WriteLogConsole("");
            }

            return true;
        }


        static List<string> GetFiles(string path, DateTime yearMonthDay)
        {
            string dash = yearMonthDay.Year.ToString("D4") + "-" + yearMonthDay.Month.ToString("D2") + "-" + yearMonthDay.Day.ToString("D2");
            string underline = yearMonthDay.Year.ToString("D4") + "_" + yearMonthDay.Month.ToString("D2") + "_" + yearMonthDay.Day.ToString("D2");

            short count = 0;

            List<string> files = new List<String>();

            string[] fileEntries = null;
            try
            {
                fileEntries = Directory.GetFiles(path);
            }
            catch (Exception e)
            {
                WriteLogConsole(string.Format("Exception in GetFiles(\"{0}\"): {1}", path, e));
                Thread.Sleep(5000);

                try
                {
                    fileEntries = Directory.GetFiles(path);
                }
                catch (Exception f)
                {
                    WriteLogConsole(string.Format("Exception in GetFiles(\"{0}\"): {1}", path, f));
                }
            }

            foreach (string fileName in fileEntries)
                if (fileName.Contains(dash) || fileName.Contains(underline))
                {
                    // 2016-08-26 Tiburon is now generating Napa...7z file to be uploaded to manhattan and oldlarkspur.
                    // Use this instead of Napa...bak
                    if (s_use_Napa_7z)
                    {
                        if (0 <= fileName.IndexOf("Napa_backup_") && fileName.IndexOf(".bak") == fileName.Length - 4)
                            continue;
                    }
                    else if (fileName.IndexOf(".7z") == fileName.Length - 3)
                        continue;

                    files.Add(Path.Combine(path, fileName));
                    ++count;
                }

            if (count == 0)
                WriteLogConsole("ERROR:  No matching files found in " + path);

            return files;
        }


        static void CreateFolder(DateTime yearMonthDay)
        {
            if (Directory.Exists(s_destFolderPath))
                throw new Exception(string.Format("Folder \"{0}\" already exists", s_destFolderPath));

            DirectoryInfo di = Directory.CreateDirectory(s_destFolderPath);
            WriteLogConsole("");
            WriteLogConsole(string.Format("Created folder \"{0}\"", s_destFolderPath));
        }


        static bool GetYearMonthDate(out string year, out string month, out string date)
        {
            string reply = "";

            year = "";
            month = "";
            date = "";

            do
            {
                try
                {
                    Console.Write("Enter year: ");
                    year = Console.ReadLine();

                    Console.Write("Enter month (1-12) with leading zero if necessary: ");
                    month = Console.ReadLine();

                    Console.Write("Enter date (1-31) with leading zero if necessary: ");
                    date = string.Format("{0:00}", Console.ReadLine());

                    Console.WriteLine("Is this date correct? {0}-{1}-{2}", year, month, date);
                    Console.Write("Type 'y' for yes, 'n' for no, or 'q' or quit >");

                    reply = Console.ReadLine();
                }
                catch (Exception e)
                {
                    Console.WriteLine("Exception: " + e.Message);
                }
            }
            while (reply != "y" && reply != "q");

            return reply != "q";
        }


        static DateTime GetPreviousWorkDay(DateTime dateTime)
        {
            DateTime previousWorkDay = DateTime.MinValue;

            string connectionString = "Data Source=tiburon;Initial Catalog=Napa";
            SqlConnection cnn = new SqlConnection(connectionString);
            cnn.Open();

            string sql = string.Format("select max(substring(CONVERT(varchar,CalendarDate,111),1,12)) as CalendarDate from CalendarDays where CalendarDate < '{0}'",
                dateTime.ToString("yyyy-MM-dd"));
            SqlCommand command = new SqlCommand(sql, cnn);

            SqlDataReader dataReader = command.ExecuteReader();
            if (dataReader.Read())
            {
                string stringPreviousWorkDay = dataReader.GetValue(0).ToString();
                int year = 0;
                int month = 0;
                int date = 0;
                GetYearMonthDate(stringPreviousWorkDay, ref year, ref month, ref date);
                previousWorkDay = new DateTime(year, month, date);
            }

            dataReader.Close();
            command.Dispose();
            cnn.Close();

            if (previousWorkDay == DateTime.MinValue)
                throw new Exception(string.Format("No return value for previous work day of '{0}'", dateTime.ToString("yyyy-MM-dd")));

            return previousWorkDay;
        }

        // YYYY-MM-DD or YYYY/MM/DD
        static void GetYearMonthDate(string yearMonthDate, ref int year, ref int month, ref int date)
        {
            year = Int32.Parse(yearMonthDate.Substring(0, 4));
            month = Int32.Parse(yearMonthDate.Substring(5, 2));
            date = Int32.Parse(yearMonthDate.Substring(8, 2));
        }

        static void SetStatics(DateTime yearMonthDate)
        {
            // append the date to s_archives keys
            Dictionary<string, Archive> newArchives = new Dictionary<string, Archive>();
            foreach (string archiveName in s_archives.Keys)
            {
                Archive archive = s_archives[archiveName];
                string newArchiveName = archiveName + yearMonthDate.ToString("yyyy-MM-dd") + ".7z";
                newArchives[newArchiveName] = archive;
            }
            s_archives = newArchives;

            s_destFolder = s_destFolder + yearMonthDate.ToString("yyyy-MM-dd");
            s_destFolderPath = Path.Combine(s_destDrive, s_destFolder);

            WriteLogConsole("Destination directory path: " + s_destFolderPath);
            WriteLogConsole("");
        }


        static bool SetCurrentPath(string path)
        {
            const int maxAttempts = 5;
            const int waitTime = 5000; // in milliseconds
            int attempt = 0;
            for (; attempt <= maxAttempts; ++attempt)
            {
                try
                {
                    WriteLogConsole(string.Format("SetCurrentPath(\"{0}\")", path));
                    Directory.SetCurrentDirectory(path);
                    string newPath = Directory.GetCurrentDirectory();
                    if (newPath != path)
                        throw new Exception(string.Format("SetCurrentPath({0}) failed", path));

                    DriveInfo driveInfo = new DriveInfo(path);

                    // GRIFF fix this logic
                    if (driveInfo.IsReady)
                        break;
                    else
                    {
                        WriteLogConsole(string.Format("SetCurrentPath(): The specified directory \"{0}\" is not ready", path));
                        Thread.Sleep(waitTime);
                    }
                }
                catch (DirectoryNotFoundException e)
                {
                    WriteLogConsole(string.Format("SetCurrentPath(): The specified directory \"{0}\" does not exist or is inaccessible. {1}", path, e));
                    Thread.Sleep(waitTime);
                }
                catch (Exception e)
                {
                    WriteLogConsole(string.Format("Exception in SetCurrentPath(\"{0}\"): {1}", path, e));
                    Thread.Sleep(waitTime);
                }
            }

            if (attempt <= maxAttempts)
            {
                if (1 < attempt)
                    WriteLogConsole(string.Format("SetCurrentPath(\"{0}\") succeeded after {1} attempts, waiting {2} milliseconds after each failure", path, attempt, waitTime));
            }
            else
                WriteLogConsole(string.Format("SetCurrentPath(\"{0}\") failed after {1} attempts, waiting {2} milliseconds after each failure", path, attempt, waitTime));

            return attempt <= maxAttempts;
        }


        static void WriteLogConsole(string s)
        {
            s_log.Append(s + Environment.NewLine);
            Console.WriteLine(s);
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
    }
}
